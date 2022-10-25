#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <envex.h>

#include "memex.h"

#define LOGEX_TAG "MEMEX-LIST"
#include "memex-log.h"

#define DEFAULT_STEP_SIZE 0x10

static pthread_mutex_t master_lock = PTHREAD_MUTEX_INITIALIZER;

static size_t memex_list_step_size = DEFAULT_STEP_SIZE;

enum memex_type_e {
    MEMEX_TYPE_LIST=0,
    MEMEX_TYPE_FIFO,
    MEMEX_TYPE_STACK,
};

struct memex_list_t {
    // Number of new entries to allocate at a time
    uint32_t step;

    // Number of total entries
    uint32_t size;

    // Number of filled entries
    uint32_t n_entry;

    // Entry size in bytes
    uint32_t entry_size;

    // Data buffer
    void *entries;

    // Sort
    enum memex_sort_type_e sort_type;
    int sort_off;

    pthread_mutex_t lock;
    pthread_mutexattr_t attr;

    int state;
    int type;
    POOL *pool;
};

void *
memex_list_new_entry(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&m->lock);

    // Increment counter, manage list size, get last empty buffer
    size_t i = m->n_entry++;

    if (i >= m->size) {
        trace("Expanding list size from %d to %d", m->size, m->size + m->step);
        m->size += m->step;
        size_t bytes = m->size * m->entry_size;
        m->entries = repalloc(m->entries, bytes, m->pool);
    }

    char *addr = (char *)m->entries;
    char *entry = addr + (i * m->entry_size);
    memset(entry, 0, m->entry_size);
    pthread_mutex_unlock(&m->lock);

    return (void *)entry;
}

void *
memex_list_get_entries(MLIST *list, uint32_t *n_entries)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        *n_entries = 0;
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&m->lock);
    *n_entries = m->n_entry;
    pthread_mutex_unlock(&m->lock);

    return m->entries;
}

void *
memex_list_get_entries_copy(MLIST *list, uint32_t *n_entries)
{
    // Dereference input pointer
    if (!list) {
        *n_entries = 0;
        error("%s: Invalid MLIST", __FUNCTION__);
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return NULL;
    }
    pthread_mutex_lock(&m->lock);
    size_t bytes = m->entry_size * m->n_entry;
    void *copy = pcalloc(m->pool, bytes);
    memcpy(copy, m->entries, bytes);

    *n_entries = m->n_entry;
    pthread_mutex_unlock(&m->lock);

    return copy;
}

MLIST *
memex_list_create(POOL *pool, const size_t entry_size)
{
    if (memex_logging_init == 0 && ENVEX_EXISTS("MEMEX_LIST_LOG_LEVEL")) {
        char lvl[32];
        ENVEX_COPY(lvl, 32, "MEMEX_LIST_LOG_LEVEL", "");
        memex_list_set_log_level(lvl);
    }

    POOL *p = create_subpool(pool);
    struct memex_list_t *m = (struct memex_list_t *)pcalloc(p, sizeof(struct memex_list_t));
    m->pool = p;
    m->step = memex_list_step_size;
    m->entry_size = entry_size;

    pthread_mutexattr_init(&m->attr);
    pthread_mutexattr_settype(&m->attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&m->lock, &m->attr);
    m->state = MEMEX_STATE_VALID;

    trace("%p: created", m);

    return (MLIST *)m;
}

MLIST *
memex_fifo_create(POOL *pool, const size_t entry_size)
{
    struct memex_list_t *m = (struct memex_list_t *)memex_list_create(pool, entry_size);
    m->type = MEMEX_TYPE_FIFO;
    return (MLIST *)m;
}

MLIST *
memex_stack_create(POOL *pool, const size_t entry_size)
{
    struct memex_list_t *m = (struct memex_list_t *)memex_list_create(pool, entry_size);
    m->type = MEMEX_TYPE_STACK;
    return (MLIST *)m;
}

MLIST *
memex_list_copy(POOL *pool, MLIST *list)
{
    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return NULL;
    }

    pthread_mutex_lock(&m->lock);
    struct memex_list_t *new = (struct memex_list_t *)memex_list_create(pool, (const size_t)m->entry_size);

    new->size = m->size;

    size_t bytes = m->size * m->entry_size;
    new->entries = palloc(pool, bytes);
    memcpy(new->entries, m->entries, bytes);
    new->n_entry = m->n_entry;
    new->sort_type = m->sort_type;
    new->sort_off = m->sort_off;
    new->state = m->state;
    pthread_mutex_unlock(&m->lock);

    return new;
}

POOL *
memex_list_get_pool(MLIST *list) {
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return NULL;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;
    return m->pool;
}

void
memex_list_clear(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }
    pthread_mutex_lock(&m->lock);
    m->n_entry = 0;
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_remove_index(MLIST *list, uint32_t index)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }

    pthread_mutex_lock(&m->lock);
    if (index >= m->n_entry) {
        goto do_return;
    }

    if (index == (m->n_entry - 1)) {
        goto dec_return;
    }

    char *dst = m->entries + (index * m->entry_size);
    char *src = dst + m->entry_size;
    size_t bytes = (m->n_entry - index + 1) * m->entry_size;
    memcpy(dst, src, bytes);

dec_return:
    m->n_entry--;

do_return:
    pthread_mutex_unlock(&m->lock);
}

int
memex_list_push(MLIST *list, void *entry)
{
    int ret = 1;

    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        goto do_return;
    }

    // Dereference input pointer
    struct memex_list_t *m = (struct memex_list_t *)list;
    if (m->type != MEMEX_TYPE_FIFO && m->type != MEMEX_TYPE_STACK) {
        error("%s: Can only push to type FIFO or STACK", __FUNCTION__);
        goto do_return;
    }

    // Create new entry
    void *new = memex_list_new_entry(list);
    if (!new) {
        goto do_return;
    }

    pthread_mutex_lock(&m->lock);
    memcpy(new, entry, m->entry_size);
    pthread_mutex_unlock(&m->lock);
    ret = 0;

do_return:
    return ret;
}

int
memex_list_pop(MLIST *list, void *entry, uint32_t *n_entries)
{
    int ret = 1;
    void *val = NULL;
    if (n_entries) {
        *n_entries = 0;
    }

    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        goto do_return;
    }

    // Dereference input pointer
    struct memex_list_t *m = (struct memex_list_t *)list;
    if (m->type != MEMEX_TYPE_FIFO && m->type != MEMEX_TYPE_STACK) {
        error("%s: Can only pop from type FIFO or STACK", __FUNCTION__);
        goto do_return;
    }

    // Hnadle race condition
    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        goto do_return;
    }

    pthread_mutex_lock(&m->lock);
    ret = 0;

    if (m->n_entry == 0) {
        goto do_return;
    }

    if (n_entries) {
        *n_entries = 1;
    }
    if (m->type == MEMEX_TYPE_FIFO) {
        // Copy first entry
        memcpy(entry, m->entries, m->entry_size);

        // Overwrite first entry
        size_t bytes = (m->n_entry - 1) * m->entry_size;
        memcpy(m->entries, m->entries + m->entry_size, bytes);

    } else if (m->type == MEMEX_TYPE_STACK) {
        // Copy last entry
        char *src = m->entries + ((m->n_entry - 1) * m->entry_size);
        memcpy(entry, src, m->entry_size);
    }

dec_return:
    m->n_entry--;

do_return:
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_remove_after_index(MLIST *list, uint32_t index)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }

    pthread_mutex_lock(&m->lock);
    m->n_entry = (index < (m->n_entry - 1)) ? index + 1 : m->n_entry;
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_remove_before_index(MLIST *list, uint32_t index)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }

    if (index == 0) {
        return;
    }

    pthread_mutex_lock(&m->lock);
    char *src = m->entries + (index * m->entry_size);
    char *dst = m->entries;
    size_t bytes = (m->n_entry - index) * m->entry_size;
    memcpy(dst, src, bytes);
    m->n_entry -= index;
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_destroy(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }

    pthread_mutex_lock(&m->lock);
    m->state = MEMEX_STATE_FREED;
    pthread_mutex_unlock(&m->lock);

    pthread_mutex_lock(&m->lock);
    POOL *free_me = m->pool;
    pthread_mutex_unlock(&m->lock);

    free_pool(free_me);

    trace("%p: destroyed", list);
}

void
_memex_list_sort_set(MLIST *list, int offset, enum memex_sort_type_e type)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;
    pthread_mutex_lock(&m->lock);
    m->sort_off = offset;
    m->sort_type = type;
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_sort(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }
    pthread_mutex_lock(&m->lock);

    // Copy entries to char buffer
    size_t bytes = m->entry_size * m->n_entry;
    uint8_t *copy = pcalloc(m->pool, bytes);
    memcpy(copy, m->entries, bytes);

    bytes = m->size * sizeof(struct memex_sort_t);
    struct memex_sort_t *sort = pcalloc(m->pool, bytes);

    uint32_t N = m->n_entry;
    for (int n = 0; n < N; n++) {
        uint8_t *e = copy + (n * m->entry_size);
        uint8_t *ptr = e + m->sort_off;
        int64_t val = 0;
        double *dval = (double *)&val;
        switch (m->sort_type) {
        case MEMEX_SORT_TYPE_DOUBLE:
            *dval = *(double *)(ptr);
            break;
        case MEMEX_SORT_TYPE_FLOAT:
            *dval = (double)(*(float *)(ptr));
            break;
        case MEMEX_SORT_TYPE_UINT64:
            val = *(uint64_t *)(ptr);
            break;
        case MEMEX_SORT_TYPE_INT64:
            val = *(int64_t *)(ptr);
            break;
        case MEMEX_SORT_TYPE_UINT32:
            val = (int64_t)(*(uint32_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_INT32:
            val = (int64_t)(*(int32_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_UINT16:
            val = (int64_t)(*(uint16_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_INT16:
            val = (int64_t)(*(int16_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_UINT8:
            val = (int64_t)(*(uint8_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_INT8:
            val = (int64_t)(*(int8_t *)(ptr));
            break;
        case MEMEX_SORT_TYPE_NOINIT:
            error("Uninitialized sort type");
            goto do_return;

        default:
            error("Invalid memex sort type: %d", m->sort_type);
            goto do_return;
        }

        sort[n].ptr = e;
        sort[n].type = m->sort_type;
        sort[n].val = val;
    }

    memex_merge_sort(sort, N);
    for (int n = 0; n < N; n++) {
        uint8_t *e = m->entries + (n * m->entry_size);
        memcpy(e, sort[n].ptr, m->entry_size);
    }

do_return:
    pfree(m->pool, copy);
    pthread_mutex_unlock(&m->lock);
    return;
}

void
memex_list_set_step_size(MLIST *list, size_t size)
{
    struct memex_list_t *m = (struct memex_list_t *)list;
    trace("%p: step_size=%zd", list, size);
}

void
memex_list_set_default_step_size(size_t size)
{
    pthread_mutex_lock(&master_lock);
    memex_list_step_size = size;
    pthread_mutex_unlock(&master_lock);

    info("default step_size=%zd", size);
}

void
memex_list_acquire(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }
    pthread_mutex_lock(&m->lock);
}

void
memex_list_release(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("%s: Invalid MLIST", __FUNCTION__);
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;

    if (m->state != MEMEX_STATE_VALID) {
        if (m->state != MEMEX_STATE_FREED) {
            error("%s:%d: Invalid MLIST: (state = %d)", __FUNCTION__, __LINE__, m->state);
        }
        return;
    }
    pthread_mutex_unlock(&m->lock);
}

void
memex_list_set_log_level(char *level)
{
    memex_set_log_level_str(level);
}
