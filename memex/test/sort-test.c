#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include <testex.h>
#include "memex.h"

#define LOGEX_TAG "LIST-TEST"
#define LOGEX_MAIN
#include <logex.h>

typedef int (*sort_test)(MLIST *);

struct sort_thread_t {
    MLIST *m;
    sort_test fn;
};

struct test_t {
    int ind;
    float f;
    double d;
    uint32_t u32;
    int32_t i32;
};

static int
rand_uint(int max)
{
    int x = rand() % max;
    return x;
}

static int
rand_int(int max)
{
    int x = rand() % max;
    return x - (max / 2);
}

static float
rand_float()
{
    int x = rand();
    float xf = (float)x / (float)(RAND_MAX / 2);
    return xf;
}

static MLIST *
init_list(POOL *pool)
{
    MLIST *m = memex_list_create(pool, sizeof(struct test_t));
    for (int i = 0; i < 10; i++) {
        struct test_t *t = memex_list_new_entry(m);
        t->ind = i;
        t->d = (double)rand_float();
        t->f = rand_float();
        t->u32 = (uint32_t)rand_uint(128);
        t->i32 = rand_int(128);
    }

    return m;
}

static void
print_test_list(MLIST *m)
{
    uint32_t N;
    struct test_t *entries = memex_list_get_entries(m, &N);
    for (int n = 0; n < N; n++) {
        struct test_t *e = entries + n;
        trace("[%d]: %d %d %f %f\n", e->ind, e->i32, e->u32, e->f, e->d);
    }
}

static int
sort_test_float(MLIST *m)
{
    int ret = 1;
    memex_list_sort_type(m, struct test_t, f, MEMEX_SORT_TYPE_FLOAT);

    uint32_t N;
    struct test_t *entries = memex_list_get_entries(m, &N);
    for (int n = 0; n < N; n++) {
        struct test_t *e = entries + n;
        trace("[%d]: %f\n", e->ind, e->f);
        if (n == 0) {
            continue;
        }

        if (entries[n - 1].f > entries[n].f) {
            goto do_return;
        }

    }
    ret = 0;

do_return:
    return ret;
}

static int 
sort_test_double(MLIST *m)
{
    int ret = 1;
    memex_list_sort_type(m, struct test_t, d, MEMEX_SORT_TYPE_DOUBLE);

    uint32_t N;
    struct test_t *entries = memex_list_get_entries(m, &N);
    for (int n = 0; n < N; n++) {
        struct test_t *e = entries + n;
        trace("[%d]: %f\n", e->ind, e->d);
        if (n == 0) {
            continue;
        }

        if (entries[n - 1].d > entries[n].d) {
            goto do_return;
        }

    }
    ret = 0;

do_return:
    return ret;
}

static int
sort_test_int(MLIST *m)
{
    int ret = 1;
    memex_list_sort_type(m, struct test_t, i32, MEMEX_SORT_TYPE_INT32);

    uint32_t N;
    struct test_t *entries = memex_list_get_entries(m, &N);
    for (int n = 0; n < N; n++) {
        struct test_t *e = entries + n;
        trace("[%d]: %d\n", e->ind, e->i32);
        if (n == 0) {
            continue;
        }

        if (entries[n - 1].i32 > entries[n].i32) {
            goto do_return;
        }
    }
    ret = 0;

do_return:
    return ret;
}

static int
sort_test_uint(MLIST *m)
{
    int ret = 1;
    memex_list_sort_type(m, struct test_t, u32, MEMEX_SORT_TYPE_UINT32);

    uint32_t N;
    struct test_t *entries = memex_list_get_entries(m, &N);
    for (int n = 0; n < N; n++) {
        struct test_t *e = entries + n;
        trace("[%d]: %d\n", e->ind, e->u32);

        if (n == 0) {
            continue;
        }

        if (entries[n - 1].u32 > entries[n].u32) {
            goto do_return;
        }
    }
    ret = 0;

do_return:
    return ret;
}

static int
basic_test()
{
    srand(time(0));

    POOL *pool = create_pool();

    MLIST *m = init_list(pool);
    print_test_list(m);

    if (sort_test_int(m) != 0) {
        verbose("int sort failed");
        return TESTEX_FAILURE;
    }

    if (sort_test_uint(m) != 0) {
        verbose("uint sort failed");
        return TESTEX_FAILURE;
    }

    if (sort_test_float(m) != 0) {
        verbose("float sort failed");
        return TESTEX_FAILURE;
    }

    if (sort_test_double(m) != 0) {
        verbose("double sort failed");
        return TESTEX_FAILURE;
    }

    free_pool(pool);
}

static void *
do_thread_fn(void *args)
{
    struct sort_thread_t *info = (struct sort_thread_t *)args;

    MLIST *m = info->m;
    memex_list_acquire(m);
    POOL *p = memex_list_get_pool(m);
    int *ret = palloc(p, sizeof(int));
    *ret = info->fn(info->m);
    memex_list_release(m);

    pthread_exit((void *)ret);
}

static int
thread_test()
{
    srand(time(0));

    POOL *pool = create_pool();
    MLIST *m = init_list(pool);

    struct sort_thread_t *info = palloc(pool, 4 * sizeof(struct sort_thread_t));
    for (int n = 0; n < 4; n++) {
        info[n].m = m;
    }
    info[0].fn = sort_test_int;
    info[1].fn = sort_test_uint;
    info[2].fn = sort_test_double;
    info[3].fn = sort_test_float;

    pthread_t id[4];
    for (int n = 0; n < 4; n++) {
        pthread_create(id + n, NULL, do_thread_fn, &info[n]);
    }

    for (int n = 0; n < 4; n++) {
        int *ret = palloc(pool, sizeof(int));
        pthread_join(id[n], (void *)&ret);

        if (*ret != 0) {
            verbose("thread safety failure");
            return TESTEX_FAILURE;
        }
    }

    memex_list_destroy(m);
    free_pool(pool);
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
