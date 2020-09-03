#ifndef __MEMEX_H__
#define __MEMEX_H__

#include <stdint.h>
#include <stddef.h>

// Memory Pools
typedef void POOL;

POOL *create_pool();
POOL *create_subpool(POOL *pool);
POOL *copy_pool(POOL *pool);
void *palloc(POOL *pool, size_t bytes);
void *pcalloc(POOL *pool, size_t bytes);
void *repalloc(void *addr, size_t bytes, POOL *pool);
void pfree(POOL *pool);
void pool_cleanup();

// Auto Cleanup
typedef void (*memex_cleanup_fn)(void);

void memex_cleanup_init();
void memex_cleanup();
void memex_cleanup_push(memex_cleanup_fn);

#endif
