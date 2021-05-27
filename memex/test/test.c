#include <stdio.h>
#include <string.h>

#include "memex.h"

struct data_t {
    char oe[10];
    uint8_t  u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
};

static void
validate(MLIST *list)
{
    uint32_t n_entries;
    struct data_t *entries = (struct data_t *)memex_list_get_entries(list, &n_entries);

    uint32_t i = 0;
    for (i = 0; i < n_entries; i++) {
        struct data_t *d = entries + i;

        if ((uint16_t)d->u32 != d->u16) {
            printf("ERROR!!!\n");
        } else if ((uint32_t)d->u64 != d->u32) {
            printf("ERROR!!!\n");
        }
    }
}

static void
print_list(MLIST *list)
{
    uint32_t n_entries;
    struct data_t *entries = (struct data_t *)memex_list_get_entries(list, &n_entries);

    uint32_t i = 0;
    for (i = 0; i < n_entries; i++) {
        struct data_t *d = entries + i;
        printf("%u %u %u %lu %s\n", d->u8, d->u16, d->u32, d->u64, d->oe);
    }
}

static MLIST *
make_list(POOL *p)
{
    printf("%s\n", __FUNCTION__);

    MLIST *list = memex_list_create(p, sizeof(struct data_t));

    uint32_t i = 0;
    for (; i < 100; i++) {
        struct data_t *d = (struct data_t *)memex_list_new_entry(list);

        d->u8 = (uint8_t)i;
        d->u16 = (uint16_t)i;
        d->u32 = (uint32_t)i;
        d->u64 = (uint64_t)i;
        if (i % 2 == 0) {
            snprintf(d->oe, 10, "%s", "even");
        } else {
            snprintf(d->oe, 10, "%s", "odd");
        }
    }

    return list;
}

static void
subpools(POOL *p, MLIST *list)
{
    printf("%s\n", __FUNCTION__);
    uint32_t n_entries;
    struct data_t *entries = (struct data_t *)memex_list_get_entries(list, &n_entries);

    char *data = (char *)entries;
    uint32_t bytes = n_entries * sizeof(struct data_t);

    POOL *s = create_subpool(p);

    uint32_t i = 0;
    for (; i < 1000; i++) {
        POOL *sp = create_subpool(s);
        char *x = palloc(sp, bytes);
        memcpy(x, data, bytes);
    }
}

void
main(int argc, char *argv[])
{
    POOL *p = create_pool();
    MLIST *list = make_list(p);
    list = make_list(p);
    list = make_list(p);

    validate(list);

    subpools(p, list);
    subpools(p, list);

    //print_list(list);

    pool_cleanup();
}
