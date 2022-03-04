#include <stdint.h>
#include <stdlib.h>

#include "memex.h"

#define LOGEX_TAG "MEMEX-LIST"
#include "memex-log.h"

void
memex_merge_sort(struct memex_sort_t *list, int len)
{
    if (len < 1) {
        verbose("%s: Invalid list length (%d)", __FUNCTION__, len);
        return;
    }

    if (len == 1) {
        return;
    }

    if (list->type == MEMEX_SORT_TYPE_NOINIT) {
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

        // Handle unsigned 64-bit case
        if (list->type == MEMEX_SORT_TYPE_UINT64) {
            if ((uint64_t)b->val < (uint64_t)a->val) {
                *m++ = *b++;
                blen--;
            } else {
                *m++ = *a++;
                alen--;
            }

        // Handle all other integers
        } else if (list->type >= MEMEX_SORT_TYPE_INT64) {
            if ((int64_t)b->val < (int64_t)a->val) {
                *m++ = *b++;
                blen--;
            } else {
                *m++ = *a++;
                alen--;
            }

        } else if (list->type >= MEMEX_SORT_TYPE_DOUBLE) {
            if ((int64_t)b->val < (int64_t)a->val) {
                *m++ = *b++;
                blen--;
            } else {
                *m++ = *a++;
                alen--;
            }
        }
    }

    free(copy);
}
