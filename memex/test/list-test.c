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

static int
copy_test()
{
    POOL *pool = create_pool();
    MLIST *m0 = memex_list_create(pool, sizeof(int));

    int N = 10;
    for (int n = 0; n < N; n++) {
        int *entry = memex_list_new_entry(m0);
        *entry = n;
    }

    MLIST *m1 = memex_list_copy(pool, m0);

    int N0, N1;
    int *e0 = memex_list_get_entries(m0, &N0);
    int *e1 = memex_list_get_entries(m1, &N1);

    if (N != N1) {
        verbose("Invalid number of entries (expected %d, got %d)", N, N1);
        return TESTEX_FAILURE;
    }
        
    if (N0 != N1) {
        verbose("Copy error: Size mismatch");
        return TESTEX_FAILURE;
    }

    if (e0 == e1) {
        verbose("Copy error: buffers are the same");
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < N; n++) {
        if (e0[n] != e1[n]) {
            verbose("Copy error: value mismatch");
            return TESTEX_FAILURE;
        }
    }

    int *x = memex_list_new_entry(m1);
    *x = 100;

    e1 = memex_list_get_entries(m1, &N1);
    if ((N + 1) != N1) {
        verbose("Invalid number of entries (expected %d, got %d)", N + 1, N1);
        return TESTEX_FAILURE;
    }

    if (e1[N1 - 1] != 100) {
        verbose("Copy failed to function independently");
        return TESTEX_FAILURE;
    }

    return TESTEX_SUCCESS;
}

static int
remove_test()
{
    POOL *pool = create_pool();
    MLIST *m = memex_list_create(pool, sizeof(int));

    int N = 10;
    for (int n = 0; n < N; n++) {
        int *entry = memex_list_new_entry(m);
        *entry = n;
    }

    memex_list_remove_index(m, 10);
    int M;
    int *entries = memex_list_get_entries(m, &M);
    if (M != N) {
        verbose("Invalid number of entries (expected %d, got %d)", N, M);
        return TESTEX_FAILURE;
    }

    memex_list_remove_index(m, 9);
    entries = memex_list_get_entries(m, &M);
    if (M != N-1) {
        verbose("Invalid number of entries (expected %d, got %d)", N-1, M);
        return TESTEX_FAILURE;
    }

    memex_list_remove_index(m, 2);
    entries = memex_list_get_entries(m, &M);
    if (M != N-2) {
        verbose("Invalid number of entries (expected %d, got %d)", N-2, M);
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < M; n++) {
        if (entries[n] == 2) {
            verbose("Failed to remove index 2");
            return TESTEX_FAILURE;
        }

        if (entries[n] == 9) {
            verbose("Failed to remove index 9");
            return TESTEX_FAILURE;
        }

        entries[n] = n;
    }

    int A = 4;
    memex_list_remove_after_index(m, A);
    entries = memex_list_get_entries(m, &M);
    if (M != A+1) {
        verbose("Invalid number of entries (expected %d, got %d)", A+1, M);
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < M; n++) {
        if (entries[n] > A) {
            verbose("\"Remove after\" failure");
            return TESTEX_FAILURE;
        }
    }

    N = M;
    A = 3;
    memex_list_remove_before_index(m, A);
    entries = memex_list_get_entries(m, &M);
    if (M != N-A) {
        verbose("Invalid number of entries (expected %d, got %d)", N-A, M);
        return TESTEX_FAILURE;
    }

    for (int n = 0; n < M; n++) {
        if (entries[n] < A) {
            verbose("\"Remove after\" failure");
            return TESTEX_FAILURE;
        }
    }

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
    testex_add(copy_test);
    testex_add(remove_test);
    testex_add(thread_test);

    testex_run();
    testex_cleanup();
}
