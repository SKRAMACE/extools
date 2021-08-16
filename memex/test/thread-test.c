#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "memex.h"

static void *
worker(void *args)
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

int
main(int argc, char *argv[])
{
    POOL *p = create_pool();

    srand(time(0));

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
        pthread_create(id, NULL, worker, pt[i]);
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
}
