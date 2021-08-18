#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <testex.h>
#include "memex.h"

#define LOGEX_TAG "LIST-TEST"
#define LOGEX_MAIN
#include <logex.h>

static int
basic_test()
{
    POOL *pool = create_pool();
    MLIST *m = memex_list_create(pool, sizeof(int));

    int N = 10;
    for (int n = 0; n < N; n++) {
        int *entry = memex_list_new_entry(m);
        *entry = n;
    }

    int M;
    int *entries = memex_list_get_entries(m, &M);
    int *copy = memex_list_get_entries_copy(m, &M);

    if (M != N) {
        verbose("Invalid number of entries (expected %d, got %d)", N, M);
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < M; n++) {
        if (entries[n] != n) {
            verbose("sequence error: (expected %d, got %d)", n, entries[n]);
            return TESTEX_FAILURE;
        }
    }

    if (copy == entries) {
        verbose("copy error");
        return TESTEX_FAILURE;
    }

    memex_list_clear(m);
    entries = memex_list_get_entries(m, &M);
    if (M > 0) {
        verbose("clear error");
        return TESTEX_FAILURE;
    }

    POOL *p = memex_list_get_pool(m);
    char *x = palloc(p, 100);
    if (!x) {
        verbose("Pool error");
        return TESTEX_FAILURE;
    }

    memex_list_destroy(m);
    return TESTEX_SUCCESS;
}

static void *
clear_list(void *args)
{
    MLIST *m = (MLIST *)args;
    memex_list_acquire(m);
    memex_list_clear(m);
    memex_list_release(m);
    pthread_exit(NULL);
}

static int
thread_test()
{
    POOL *pool = create_pool();
    MLIST *m = memex_list_create(pool, sizeof(int));

    int N = 10;
    for (int n = 0; n < N; n++) {
        int *entry = memex_list_new_entry(m);
        *entry = n;
    }

    // Acquire list before kicking off thread
    memex_list_acquire(m);

    // Kick off thread
    pthread_t id;
    pthread_create(&id, NULL, clear_list, (void *)m);

    // Get entries, and verify that the thread hasn't been cleared
    int M;
    int *entries = memex_list_get_entries(m, &M);

    if (M != N) {
        verbose("Invalid number of entries (expected %d, got %d)", N, M);
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < M; n++) {
        if (entries[n] != n) {
            verbose("sequence error: (expected %d, got %d)", n, entries[n]);
            return TESTEX_FAILURE;
        }
    }

    // Release list: Thread should now complete it's actions
    memex_list_release(m);
    pthread_join(id, NULL);

    memex_list_acquire(m);
    entries = memex_list_get_entries(m, &M);
    if (M > 0) {
        verbose("clear error");
        return TESTEX_FAILURE;
    }
    memex_list_release(m);

    memex_list_destroy(m);
    return TESTEX_SUCCESS;
}

int
main(int nargs, char *argv[])
{
    TESTEX_LOG_INIT("info");
    testex_setup();

    testex_add(basic_test);
    testex_add(thread_test);

    testex_run();
    testex_cleanup();
}
