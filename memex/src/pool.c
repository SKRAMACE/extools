/*
 *   memex pool is a simple memory pool hierarchy designed to simplify memory
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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <envex.h>

#include "memex.h"

#define LOGEX_TAG "MEMEX-POOL"
#include "memex-log.h"

#define RADPOOL_ALLOC_INCREMENT 0x80
#define MEMEX_STATE_VALID 0x10001000
#define MEMEX_STATE_FREED 0x10101010

static pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;

// Struct for keeping track of memory allocations
struct alloc_info {
    void *addr;
    uint64_t len;
};

// Implementation struct
static struct memex_pool_t {
    struct alloc_info *allocs;
    uint32_t alloc_space;
    uint32_t alloc_count;
    struct memex_pool_t **pools;
    uint32_t pool_space;
    uint32_t pool_count;
    struct memex_pool_t *super_pool;
    pthread_mutex_t lock;
    int state;
} *master_pool = NULL;

static void
init_pool(POOL *pool)
{
    struct memex_pool_t *p = (struct memex_pool_t *)pool;
    p->super_pool = NULL;
    p->alloc_space = RADPOOL_ALLOC_INCREMENT;
    p->alloc_count = 0;
    p->pool_space = RADPOOL_ALLOC_INCREMENT;
    p->pool_count = 0;
    p->state = MEMEX_STATE_VALID;

    p->allocs = malloc(RADPOOL_ALLOC_INCREMENT * sizeof(struct alloc_info));
    trace("%p:  Buf alloc (%p)", pool, p->allocs);

    p->pools = malloc(RADPOOL_ALLOC_INCREMENT * sizeof(struct memex_pool_t*));
    trace("%p:  Buf alloc (%p)", pool, p->pools);

    pthread_mutex_init(&p->lock, NULL);
}

static void
init_master_pool()
{
    pthread_mutex_lock(&master_lock);
    if (master_pool) {
        pthread_mutex_unlock(&master_lock);
        return;
    }

    info("Allocating master pool");
    master_pool = malloc(sizeof(struct memex_pool_t));
    init_pool(master_pool);
    pthread_mutex_unlock(&master_lock);
}

/*
 *  Allocate memory in pool
 */
void *
palloc(POOL *pool, size_t bytes)
{
    struct memex_pool_t *p = (struct memex_pool_t*)pool;

    if (!p) {
        error("Null pool pointer");
        return NULL;
    }

    if (p->state != MEMEX_STATE_VALID) {
        if (p->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, p->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&p->lock);

    // Resize the allocs array, if necessary
    if (p->alloc_space == p->alloc_count) {
        uint32_t new_space = p->alloc_space + RADPOOL_ALLOC_INCREMENT;

        void *a = realloc(p->allocs, new_space * sizeof(struct alloc_info));
        trace("%p:  Buf realloc (%p -> %p)", p, p->allocs, a);
        p->allocs = a;

        p->alloc_space = new_space;
    }

    // Call malloc and add pointer to allocs array
    void *addr = malloc(bytes);
    trace("%p: Data alloc (%p)", pool, addr);

    struct alloc_info *info = p->allocs + p->alloc_count++;
    info->addr = addr;
    info->len = bytes;
    pthread_mutex_unlock(&p->lock);

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
    struct memex_pool_t *p = (struct memex_pool_t *)pool;
    void *ret = NULL;
    
    if (!p) {
        error("Null pool pointer");
        return NULL;
    }

    if (!addr) {
        return palloc(pool, bytes);
    }

    if (p->state != MEMEX_STATE_VALID) {
        if (p->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, p->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&p->lock);

    uint32_t i;
    // Search pool for alloc addr
    for (i = 0; i < p->alloc_count; i++) {
        if (p->allocs[i].addr == addr) {
            void *re = p->allocs[i].addr;
            trace("Reallocating from %zd to %zd bytes", p->allocs[i].len, bytes);
            re = realloc(re, bytes);
            p->allocs[i].addr = re;
            p->allocs[i].len = bytes;
            ret = re;
            goto do_return;
        }
    }

    // Search each sub-pool recursively
    for (i = 0; i < p->pool_count; i++) {
        ret = repalloc(addr, bytes, p->pools[i]);
        if (ret) {
            goto do_return;
        }
    }

do_return:
    pthread_mutex_unlock(&p->lock);
    return ret;
}

static void
add_subpool(POOL *pool, POOL *sub)
{
    struct memex_pool_t *p = (struct memex_pool_t*)pool;
    pthread_mutex_lock(&p->lock);

    // Resize the pools array, if necessary
    if (p->pool_space == p->pool_count) {
        trace("Expanding pool tracking buffer");
        uint32_t new_space = p->pool_space + RADPOOL_ALLOC_INCREMENT;

        void *a = realloc(p->pools, new_space * sizeof(struct memex_pool_t*));
        trace("%p:  Buf realloc (%p -> %p)", p, p->pools, a);
        p->pools = a;

        p->pool_space = new_space;
    }

    // Create a new subpool and add to the pools array
    p->pools[p->pool_count++] = (struct memex_pool_t*)sub;
    ((struct memex_pool_t*)sub)->super_pool = p;

    pthread_mutex_unlock(&p->lock);
}

POOL *
create_pool()
{
    // Logging init
    if (memex_logging_init == 0 && ENVEX_EXISTS("MEMEX_POOL_LOG_LEVEL")) {
        char lvl[32];
        ENVEX_COPY(lvl, 32, "MEMEX_POOL_LOG_LEVEL", "");
        memex_pool_set_log_level(lvl);
    }

    if (!master_pool) {
        init_master_pool();
    }

    struct memex_pool_t *p = malloc(sizeof(struct memex_pool_t));
    trace("%p:  Buf alloc (%p)", p, p);
    init_pool(p);

    // Every pool is a sub of the master pool
    add_subpool(master_pool, p);

    // Cast to generic struct
    return (POOL*)p;
}

POOL *
create_subpool(POOL *pool)
{
    // Cast to implementation struct
    struct memex_pool_t *p = (struct memex_pool_t*)pool;

    // Create a new subpool and add to the pools array
    struct memex_pool_t *sub = malloc(sizeof(struct memex_pool_t));
    info("%p: Creating sub pool %p", pool, sub);
    trace("%p:  Buf alloc (%p)", sub, sub);
    init_pool(sub);

    add_subpool(p, sub);

    return (POOL *)sub;
}

POOL *
copy_pool(POOL *pool)
{
    info("%p: Copying", pool);

    int i;
    struct memex_pool_t *p = (struct memex_pool_t*)pool;

    POOL *new = create_pool();

    pthread_mutex_lock(&p->lock);
    for (i = 0; i < p->alloc_count; i++) {
        struct alloc_info *src = p->allocs + i;
        char *dst = palloc(new, src->len);
        memcpy(dst, src->addr, src->len);
    }

    for (i = 0; i < p->pool_count; i++) {
        POOL *sub = copy_pool((POOL*)p->pools[i]);
        add_subpool(new, sub);
    }
    pthread_mutex_unlock(&p->lock);

    return new;
}

static POOL *
unlink_pool(POOL *pool) {
    struct memex_pool_t *p = (struct memex_pool_t*)pool;
    if (p->state != MEMEX_STATE_VALID) {
        if (p->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, p->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&p->lock);

    // If no parent, there's no need to unlink
    if (!p->super_pool) {
        goto no_parent;
    }

    // Get parent
    struct memex_pool_t *parent = p->super_pool;
    if (parent->state != MEMEX_STATE_VALID) {
        if (parent->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, parent->state);
        }
        goto no_parent;
    }
    pthread_mutex_lock(&parent->lock);

    // Find this pool in it's parent's pool list
    uint32_t i, j;
    for (i = 0; i < parent->pool_count; i++) {
        if (parent->pools[i] == p) {
            break;
        }
    }

    // Pool not found in parent's pool list
    if (i == parent->pool_count) {
        goto do_return;
    }

    // Remove pool from parent pool list
    parent->pool_count--;
    for (j = i; j < parent->pool_count; j++) {
        parent->pools[j] = parent->pools[j + 1];
    }

do_return:
    pthread_mutex_unlock(&parent->lock);

no_parent:
    pthread_mutex_unlock(&p->lock);
}

// Free allocations in pool
static inline void
pfree_allocs(struct memex_pool_t *p)
{
    uint32_t i;
    for (i = 0; i < p->alloc_count; i++) {
        struct alloc_info *info = p->allocs + i;
        if (info->addr) {
            trace("%p: Data free (%p)", p, info->addr);
            free(info->addr);
        }
    }

    trace("%p:  Buf free (%p)", p, p->allocs);
    free(p->allocs);
}

// Recursively free pool and sub-pools without unlinking the parent
static void
pfree_sub(POOL *pool)
{
    struct memex_pool_t *p = (struct memex_pool_t*)pool;
    if (p->state != MEMEX_STATE_VALID) {
        if (p->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, p->state);
        }
        return;
    }

    pthread_mutex_lock(&p->lock);
    p->state = MEMEX_STATE_FREED;
    pthread_mutex_unlock(&p->lock);

    pthread_mutex_lock(&p->lock);
    info("%p: Free", pool);
    uint32_t i;
    for (i = 0; i < p->pool_count; i++) {
        pfree_sub((POOL *)p->pools[i]);
    }
    pfree_allocs(p);

    trace("%p:  Buf free (%p)", p, p->pools);
    free(p->pools);

    pthread_mutex_unlock(&p->lock);
    pthread_mutex_destroy(&p->lock);

    trace("%p:  Buf free (%p)", p, p);
    free(p);
}

void
free_pool(POOL *pool)
{
    if (!pool) {
        return;
    }
    unlink_pool(pool);
    pfree_sub(pool);
}

void
pool_cleanup()
{
    pthread_mutex_lock(&master_lock);
    if (master_pool) {
        info("Freeing master pool");
        free_pool(master_pool);
        master_pool = NULL;
    }
    pthread_mutex_unlock(&master_lock);
}

void
pfree(POOL *pool, void *addr)
{
    struct memex_pool_t *p = (struct memex_pool_t*)pool;
    if (p->state != MEMEX_STATE_VALID) {
        if (p->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid Pool: (state = %d)", __FUNCTION__, __LINE__, p->state);
        }
        return;
    }

    pthread_mutex_lock(&p->lock);
    uint32_t i;
    for (i = 0; i < p->alloc_count; i++) {
        struct alloc_info *info = p->allocs + i;
        if (info->addr != addr) {
            continue;
        }

        trace("%p: Data free (%p)", p, info->addr);
        free(info->addr);
        info->addr = NULL;
        info->len = 0;
    }
    pthread_mutex_unlock(&p->lock);
}

void
memex_pool_set_log_level(char *level)
{
    memex_set_log_level_str(level);
}
