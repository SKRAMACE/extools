/*
 *   memex cleanup is a stack-based cleanup routine manager
 *
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
#include <signal.h>
#include <envex.h>

#include "memex.h"

#define LOGEX_TAG "MEMEX-CLEAN"
#include "memex-log.h"

#define MAX_CLEANUP_FN 10

static int cleanup_called = 0;

struct memex_t {
    void *fn;
    void *args;
};
static MLIST *stack = NULL;

static void
memex_cleanup_push_internal(void *fn, void *args)
{
    if (memex_logging_init == 0 && ENVEX_EXISTS("MEMEX_CLEANUP_LOG_LEVEL")) {
        char lvl[32];
        ENVEX_COPY(lvl, 32, "MEMEX_CLEANUP_LOG_LEVEL", "");
        memex_cleanup_set_log_level(lvl);
    }

    if (!stack) {
        POOL *pool = create_pool_unmanaged();
        stack = memex_list_create(pool, sizeof(struct memex_t));
    }

    struct memex_t *e = memex_list_new_entry(stack);
    e->fn = fn;
    e->args = args;
}

void
memex_cleanup_push(memex_cleanup_fn fn)
{
    memex_cleanup_push_internal(fn, NULL);
}

void
memex_cleanup_push_args(memex_cleanup_args_fn fn, void *args)
{
    memex_cleanup_push_internal(fn, args);
}

void
memex_cleanup()
{
    // Quick return
    if (cleanup_called) {
        return;
    }

    // Handle race condition
    memex_list_acquire(stack);
    if (cleanup_called) {
        memex_list_release(stack);
        return;
    }
    cleanup_called = 1;
    memex_list_release(stack);

    // Do actual cleanup
    memex_list_acquire(stack);
    uint32_t N;
    struct memex_t *entries = memex_list_get_entries(stack, &N);
    for (int i = N - 1; i >= 0; i--) {
        struct memex_t *e = entries + i;
        if (e->args) {
            memex_cleanup_args_fn fn = (memex_cleanup_args_fn)e->fn;
            fn(e->args);
        } else {
            memex_cleanup_fn fn = (memex_cleanup_fn)e->fn;
            fn();
        }
    }
    POOL *free_me = memex_list_get_pool(stack);
    memex_list_release(stack);
    memex_list_destroy(stack);

    free_pool(free_me);
}

static void
memex_cleanup_and_exit()
{
    memex_cleanup();
    exit(0);
}

void
memex_cleanup_init()
{
    // Register cleanup function
    signal(SIGINT, memex_cleanup_and_exit);
}

void
memex_cleanup_set_log_level(char *level)
{
    memex_set_log_level_str(level);
}
