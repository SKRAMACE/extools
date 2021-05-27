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
void free_pool(POOL *pool);
void pool_cleanup();
void pfree(POOL *pool, void *addr);

// Auto Cleanup
typedef void (*memex_cleanup_fn)(void);
typedef void (*memex_cleanup_args_fn)(void*);

void memex_cleanup_init();
void memex_cleanup();
void memex_cleanup_push(memex_cleanup_fn fn);
void memex_cleanup_push_args(memex_cleanup_args_fn fn, void *args);

// Lists
typedef void MLIST;

MLIST *memex_list_create(POOL *pool, const size_t entry_size);
void *memex_list_new_entry(MLIST *list);
void *memex_list_get_entries(MLIST *list, uint32_t *n_entries);
void memex_list_set_step_size(MLIST *list, size_t size);
void memex_list_destroy(MLIST *list);

void memex_list_set_default_step_size(size_t size);

// Logging
void memex_pool_set_log_level(char *level);
void memex_cleanup_set_log_level(char *level);
void memex_list_set_log_level(char *level);

#endif
