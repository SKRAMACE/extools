#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <testex.h>
#include <memex.h>

#define LOGEX_TAG "POOL-TEST"
#define LOGEX_MAIN
#include <logex.h>

static int
basic_test()
{
    POOL *pool = create_pool();
    if (!pool) {
        verbose("pool failure");
        return TESTEX_FAILURE;
    }

    POOL *sub = create_subpool(pool);
    if (!sub) {
        verbose("subpool failure");
        return TESTEX_FAILURE;
    }

    int N = 10;
    char *x = palloc(pool, N);
    char *y = pcalloc(sub, N);
    for (int n = 0; n < N; n++) {
        x[n] = n;
        if (y[n] != 0) {
            verbose("pcalloc error");
            return TESTEX_FAILURE;
        }
        y[n] = n;
    }

    for (int n = 0; n < N; n++) {
        if (x[n] != y[n]) {
            verbose("memory corruption");
            return TESTEX_FAILURE;
        }
    }

    N *= 2;
    x = repalloc(x, N, pool);
    for (int n = 0; n < N; n++) {
        x[n] = n;
    }

    pfree(pool, x);
    free_pool(pool);

    return TESTEX_SUCCESS;
}

static int
free_test()
{
    POOL *pool0 = create_pool();
    POOL *pool1 = create_pool();
    POOL *sub = create_subpool(pool0);

    int N = 10;
    char *x = palloc(pool0, N);
    char *y = palloc(pool1, N);
    if (!x || !y) {
        verbose("alloc failure");
        return TESTEX_FAILURE;
    }

    free_pool(pool0);
    x = palloc(pool0, N);
    if (x) {
        verbose("alloc after free failure");
        return TESTEX_FAILURE;
    }

    x = palloc(sub, N);
    if (x) {
        verbose("subpool alloc after parent freed failure");
        return TESTEX_FAILURE;
    }

    y = palloc(pool1, N);
    if (!y) {
        verbose("pool exclusivity failure");
        return TESTEX_FAILURE;
    }

    POOL *upool = create_pool_unmanaged();
    x = palloc(upool, N);
    if (!x) {
        verbose("unmanaged pool alloc failure");
        return TESTEX_FAILURE;
    }

    pool_cleanup();
    x = palloc(pool1, 10);
    y = palloc(upool, 10);
    if (x) {
        verbose("pool alloc after cleanup failure");
        return TESTEX_FAILURE;
    }

    if (!y) {
        verbose("unmanaged pool failure");
        return TESTEX_FAILURE;
    }

    free_pool(upool);
    y = palloc(upool, 10);
    if (y) {
        verbose("unmanaged pool alloc after free failure");
        return TESTEX_FAILURE;
    }

    return TESTEX_SUCCESS;
}

static void *
pool_worker(void *args)
{
    POOL *pool = (POOL *)args;
    char *buf = NULL;
    size_t len = 0;

    do {
        if (len == 0) {
            len = 1024;
            buf = palloc(pool, len);

        } else if (len == 1024) {
            len *= 1024;
            buf = repalloc(buf, len, pool);

        } else {
            len = 0;
            pfree(pool, buf);
        }

        if (!buf) {
            break;
        }

        usleep(1000);

    } while (buf);

    pthread_exit(NULL);
}

static int 
thread_test()
{
    POOL *p = create_pool();

    pthread_t t[20];
    POOL *pt[20];
    for (int i = 0; i < 20; i++) {
        int x = rand();
        double xd = (double)x / (double)RAND_MAX;
        if (xd > 0.5) {
            pt[i] = create_subpool(p);
        } else {
            pt[i] = p;
        }

        pthread_t *id = t + i;
        pthread_create(id, NULL, pool_worker, pt[i]);
    }

    while (1) {
        int x = rand();
        double xd = (double)x / (double)RAND_MAX;
        if (xd > 0.999) {
            break;
        }

        usleep(1000);
    }

    pool_cleanup();

    for (int i = 0; i < 20; i++) {
        pthread_join(t[i], NULL);
    }

    return TESTEX_SUCCESS;
}

int
main(int nargs, char *argv[])
{
    TESTEX_LOG_INIT("info");
    testex_setup();

    testex_add(basic_test);
    testex_add(free_test);
    testex_add(thread_test);

    testex_run();
    testex_cleanup();
}
