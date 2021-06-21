#include <stdint.h>
#include <stdlib.h>

#include "memex.h"

void
memex_merge_sort(struct memex_sort_t *list, int len)
{
    if (len == 1) {
        return;
    }

    size_t bytes = len * sizeof(struct memex_sort_t);
    struct memex_sort_t *copy = malloc(bytes);
    for (int i = 0; i < len; i++) {
        copy[i] = list[i];
    }

    struct memex_sort_t *a = copy;
    int alen = (len + 1) / 2;

    struct memex_sort_t *b = a + alen;
    int blen = len - alen;

    memex_merge_sort(a, alen);
    memex_merge_sort(b, blen);
    struct memex_sort_t *m = list;
    while (alen + blen) {
        if (alen == 0) {
            *m++ = *b++;
            blen--;
            continue;
        }

        if (blen == 0) {
            *m++ = *a++;
            alen--;
            continue;
        }

        if (b->val < a->val) {
            *m++ = *b++;
            blen--;
        } else {
            *m++ = *a++;
            alen--;
        }
    }
    free(copy);
}
