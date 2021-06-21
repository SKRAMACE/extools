#include <stdlib.h>
#include <stdint.h>
#include <envex.h>

#include "memex.h"

#define LOGEX_TAG "MEMEX-LIST"
#include "memex-log.h"

#define DEFAULT_STEP_SIZE 0x10

static size_t memex_list_step_size = DEFAULT_STEP_SIZE;

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

    POOL *pool;
};

void *
memex_list_new_entry(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("Invalid MLIST");
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

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
    return (void *)entry;
}

void *
memex_list_get_entries(MLIST *list, uint32_t *n_entries)
{
    // Dereference input pointer
    if (!list) {
        error("Invalid MLIST");
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

    *n_entries = m->n_entry;
    return m->entries;
}

void *
memex_list_get_entries_copy(MLIST *list, uint32_t *n_entries)
{
    // Dereference input pointer
    if (!list) {
        error("Invalid MLIST");
        return NULL;
    }
    struct memex_list_t *m = (struct memex_list_t *)list;

    size_t bytes = m->entry_size * m->n_entry;
    void *copy = pcalloc(m->pool, bytes);
    memcpy(copy, m->entries, bytes);

    *n_entries = m->n_entry;
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

    trace("%p: created", m);

    return (MLIST *)m;
}

POOL *
memex_list_get_pool(MLIST *list) {
    // Dereference input pointer
    if (!list) {
        error("Invalid MLIST");
        return NULL;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;
    return m->pool;
}

void
memex_list_destroy(MLIST *list)
{
    // Dereference input pointer
    if (!list) {
        error("Invalid MLIST");
        return;
    }

    struct memex_list_t *m = (struct memex_list_t *)list;
    POOL *free_me = m->pool;
    free_pool(free_me);

    trace("%p: destroyed", list);
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
    memex_list_step_size = size;
    info("default step_size=%zd", size);
}

void
memex_list_set_log_level(char *level)
{
    memex_set_log_level_str(level);
}
