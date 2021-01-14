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

#include "memex.h"

#define MAX_CLEANUP_FN 10
static int cleanup_called = 0;
static int n_cleanup_fn = 0;
static memex_cleanup_fn fn[MAX_CLEANUP_FN];

void
memex_cleanup_push(memex_cleanup_fn new_fn)
{
    if (n_cleanup_fn >= MAX_CLEANUP_FN) {
        printf("ERROR: Max cleanup functions reached\n");
        return;
    }
    fn[n_cleanup_fn++] = new_fn;
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
        fn[i]();
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
