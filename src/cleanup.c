#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "memex.h"

#define MAX_CLEANUP_FN 10
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
    int i = n_cleanup_fn - 1;
    for (; i >= 0; i--) {
        fn[i]();
    }
}

void
memex_cleanup_init()
{
    // Register cleanup function
    signal(SIGINT, memex_cleanup);
}
