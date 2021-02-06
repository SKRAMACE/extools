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
static int n_cleanup_fn = 0;

static struct memex_t {
    void *fn;
    void *args;
} stack[MAX_CLEANUP_FN];

static void
memex_cleanup_push_internal(void *fn, void *args)
{
    if (memex_logging_init == 0 && ENVEX_EXISTS("MEMEX_CLEANUP_LOG_LEVEL")) {
        char lvl[32];
        ENVEX_COPY(lvl, 32, "MEMEX_CLEANUP_LOG_LEVEL", "");
        memex_cleanup_set_log_level(lvl);
    }

    if (n_cleanup_fn >= MAX_CLEANUP_FN) {
        error("Max cleanup functions reached");
        return;
    }

    int n = n_cleanup_fn++;
    stack[n].fn = fn;
    stack[n].args = args;
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
    if (cleanup_called) {
        return;
    }
    cleanup_called = 1;

    int i = n_cleanup_fn - 1;
    for (; i >= 0; i--) {
        if (stack[i].args) {
            memex_cleanup_args_fn fn = (memex_cleanup_args_fn)stack[i].fn;
            fn(stack[i].args);
        } else {
            memex_cleanup_fn fn = (memex_cleanup_fn)stack[i].fn;
            fn();
        }
    }
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
