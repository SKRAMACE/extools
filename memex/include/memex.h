#ifndef __MEMEX_H__
#define __MEMEX_H__

#include <stdint.h>
#include <stddef.h>

// Memory Pools
typedef void POOL;

#define MEMEX_STATE_VALID 0x10001000
#define MEMEX_STATE_FREED 0x10101010

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
MLIST *memex_list_copy(POOL *pool, MLIST *list);
void memex_list_clear(MLIST *list);
void *memex_list_new_entry(MLIST *list);
void *memex_list_get_entries(MLIST *list, uint32_t *n_entries);
void *memex_list_get_entries_copy(MLIST *list, uint32_t *n_entries);
void memex_list_set_step_size(MLIST *list, size_t size);
void memex_list_destroy(MLIST *list);
POOL *memex_list_get_pool(MLIST *list);
void memex_list_acquire(MLIST *list);
void memex_list_release(MLIST *list);

void memex_list_set_default_step_size(size_t size);

// Logging
void memex_pool_set_log_level(char *level);
void memex_cleanup_set_log_level(char *level);
void memex_list_set_log_level(char *level);

// Sort
enum memex_sort_type_e {
    MEMEX_SORT_TYPE_NOINIT=0,
    MEMEX_SORT_TYPE_DOUBLE,
    MEMEX_SORT_TYPE_FLOAT,
    MEMEX_SORT_TYPE_UINT64,
    MEMEX_SORT_TYPE_INT64,
    MEMEX_SORT_TYPE_UINT32,
    MEMEX_SORT_TYPE_INT32,
    MEMEX_SORT_TYPE_UINT16,
    MEMEX_SORT_TYPE_INT16,
    MEMEX_SORT_TYPE_UINT8,
    MEMEX_SORT_TYPE_INT8,
};

struct memex_sort_t {
    void *ptr;
    double val;
};

#define memex_list_sort_set(list, _STRUCT_, _MEMBER_, type) \
{\
    _STRUCT_ _x; \
    void *_x0 = &_x; \
    void *_x1 = &_x._MEMBER_; \
    int off = (int)(_x1 - _x0); \
    _memex_list_sort_set(list, off, type); \
}

#define memex_list_sort_type(list, _STRUCT_, _MEMBER_, type)\
{\
    memex_list_sort_set(list, _STRUCT_, _MEMBER_, type); \
    memex_list_sort(list); \
}

#define memex_list_sort_double(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_DOUBLE)
#define memex_list_sort_float(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_FLOAT)
#define memex_list_sort_int64(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_INT64)
#define memex_list_sort_uint64(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_UINT64)
#define memex_list_sort_int32(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_INT32)
#define memex_list_sort_uint32(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_UINT32)
#define memex_list_sort_int16(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_INT16)
#define memex_list_sort_uint16(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_UINT16)
#define memex_list_sort_int8(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_INT8)
#define memex_list_sort_uint8(list, _STRUCT_, _MEMBER_) \
    memex_list_sort_type(list, _STRUCT_, _MEMBER_, MEMEX_SORT_TYPE_UINT8)

void _memex_list_sort_set(MLIST *list, int offset, enum memex_sort_type_e type);
void memex_list_sort(MLIST *list);

void memex_merge_sort(struct memex_sort_t *list, int len);

#endif
