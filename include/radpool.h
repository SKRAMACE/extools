#ifndef __POOL_H__
#define __POOL_H__

#include <stdint.h>

typedef void POOL;

POOL *create_pool();
POOL *create_subpool(POOL *pool);
POOL *copy_pool(POOL *pool);
void *palloc(POOL *pool, size_t bytes);
void *pcalloc(POOL *pool, size_t bytes);
void *repalloc(void *addr, size_t bytes, POOL *pool);
void pfree(POOL *pool);
void pool_cleanup();

#endif
