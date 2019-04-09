/*
 *   radpool is a simple memory pool hierarchy designed to simplify memory
 *   management in buffer-heavy software
 *
 *   Copyright (C) 2017 SKRAMACE
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "radpool.h"

#define RADPOOL_ALLOC_INCREMENT 0x10

// Struct for keeping track of memory allocations
struct alloc_info {
    void *addr;
    uint64_t len;
};

// Implementation struct
static struct radpool_t {
    struct alloc_info *allocs;
    uint32_t alloc_space;
    uint32_t alloc_count;
    struct radpool_t **pools;
    uint32_t pool_space;
    uint32_t pool_count;
    struct radpool_t *super_pool;
} *master_pool = NULL;

static void
init_pool(POOL *pool)
{
    struct radpool_t *p = (struct radpool_t *)pool;
    p->allocs = malloc(RADPOOL_ALLOC_INCREMENT * sizeof(struct alloc_info));
    p->alloc_space = RADPOOL_ALLOC_INCREMENT;
    p->alloc_count = 0;
    p->pools = malloc(RADPOOL_ALLOC_INCREMENT * sizeof(struct radpool_t*));
    p->pool_space = RADPOOL_ALLOC_INCREMENT;
    p->pool_count = 0;
    p->super_pool = NULL;
}

/*
 *  Allocate memory in pool
 */
void *
palloc(POOL *pool, size_t bytes)
{
    struct radpool_t *p = (struct radpool_t*)pool;

    if (!p) {
        return NULL;
    }

    // Resize the allocs array, if necessary
    if (p->alloc_space == p->alloc_count) {
        uint32_t new_space = p->alloc_space + RADPOOL_ALLOC_INCREMENT;
        p->allocs = realloc(p->allocs, new_space * sizeof(struct alloc_info));
        p->alloc_space = new_space;
    }

    // Call malloc and add pointer to allocs array
    void *addr = malloc(bytes);
    struct alloc_info *info = p->allocs + p->alloc_count++;
    info->addr = addr;
    info->len = bytes;

    // Return the allocated memory addr
    return addr;
}

/*
 *  Allocate memory and zero buffer
 */
void *
pcalloc(POOL *pool, size_t bytes)
{
    void *addr = palloc(pool, bytes);
    if (!addr) {
        return NULL;
    }
    memset(addr, 0, bytes);
    return addr;
}

/*
 * Realloc memory tracked by pool
 */
void *
repalloc(void *addr, size_t bytes, POOL *pool)
{
    struct radpool_t *p = (struct radpool_t *)pool;
    void *ret = NULL;
    
    if (!p) {
        return NULL;
    }

    if (!addr) {
        return palloc(pool, bytes);
    }

    uint32_t i;
    // Search pool for alloc addr
    for (i = 0; i < p->alloc_count; i++) {
        if (p->allocs[i].addr == addr) {
            void *re = p->allocs[i].addr;
            re = realloc(re, bytes);
            p->allocs[i].addr = re;
            p->allocs[i].len = bytes;
            return re;
        }
    }

    // Search each sub-pool recursively
    for (i = 0; i < p->pool_count; i++) {
        ret = repalloc(addr, bytes, p->pools[i]);
        if (ret) {
            return ret;
        }
    }
    return NULL;
}

static void
add_subpool(POOL *pool, POOL *sub)
{
    struct radpool_t *p = (struct radpool_t*)pool;

    // Resize the pools array, if necessary
    if (p->pool_space == p->pool_count) {
        uint32_t new_space = p->pool_space + RADPOOL_ALLOC_INCREMENT;
        p->pools = realloc(p->pools, new_space * sizeof(struct radpool_t*));
        p->pool_space = new_space;
    }

    // Create a new subpool and add to the pools array
    p->pools[p->pool_count++] = (struct radpool_t*)sub;
    ((struct radpool_t*)sub)->super_pool = p;
}

POOL *
create_pool()
{
    if (!master_pool) {
        master_pool = malloc(sizeof(struct radpool_t));
        init_pool(master_pool);
    }

    // Every pool is a sub of the master pool
    struct radpool_t *p = malloc(sizeof(struct radpool_t));
    init_pool(p);

    add_subpool(master_pool, p);

    // Cast to generic struct
    return (POOL*)p;
}

POOL *
create_subpool(POOL *pool)
{
    // Cast to implementation struct
    struct radpool_t *p = (struct radpool_t*)pool;

    // Create a new subpool and add to the pools array
    struct radpool_t *sub = malloc(sizeof(struct radpool_t));
    init_pool(sub);

    add_subpool(p, sub);

    return (POOL *)sub;
}

POOL *
copy_pool(POOL *pool)
{
    int i;
    struct radpool_t *p = (struct radpool_t*)pool;

    POOL *new = create_pool();

    for (i = 0; i < p->alloc_count; i++) {
        struct alloc_info *src = p->allocs + i;
        char *dst = palloc(new, src->len);
        memcpy(dst, src->addr, src->len);
    }

    for (i = 0; i < p->pool_count; i++) {
        POOL *sub = copy_pool((POOL*)p->pools[i]);
        add_subpool(new, sub);
    }

    return new;
}

static void
unlink_pool(POOL *pool) {
    struct radpool_t *p = (struct radpool_t*)pool;

    // If no parent, there's no need to unlink
    if (!p->super_pool) {
        return;
    }

    // Get parent
    struct radpool_t *parent = p->super_pool;

    // Find this pool in it's parent's pool list
    uint32_t i, j;
    for (i = 0; i < parent->pool_count; i++) {
        if (parent->pools[i] == p) {
            break;
        }
    }

    // Pool not found in parent's pool list
    if (i == parent->pool_count) {
        return;
    }

    // Remove pool from parent pool list
    parent->pool_count--;
    for (j = i; j < parent->pool_count; j++) {
        parent->pools[j] = parent->pools[j + 1];
    }
}

// Free allocations in pool
static inline void
pfree_allocs(struct radpool_t *p)
{
    uint32_t i;
    for (i = 0; i < p->alloc_count; i++) {
        struct alloc_info *info = p->allocs + i;
        free(info->addr);
    }
    free(p->allocs);
}

// Recursively free pool and sub-pools without unlinking the parent
static void
pfree_sub(POOL *pool)
{
    struct radpool_t *p = (struct radpool_t*)pool;

    uint32_t i;
    for (i = 0; i < p->pool_count; i++) {
        pfree_sub((POOL *)p->pools[i]);
    }
    pfree_allocs(p);
    free(p->pools);
    free(p);
}

void
pfree(POOL *pool)
{
    unlink_pool(pool);
    pfree_sub(pool);
}

void
pool_cleanup()
{
    pfree(master_pool);
}
