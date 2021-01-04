#ifndef __MEMEX_LOG_H__
#define __MEMEX_LOG_H__

#define LOGEX_TAG "MEMEX"
#include <logex-lib.h>

#define memex_set_log_level_str(x) {\
    char _memex_set_log_level_lvl[32]; \
    strncpy_upper(_memex_set_log_level_lvl, 32, x); \
    set_log_level_str(_memex_set_log_level_lvl);}

void strncpy_upper(char *dst, int n, char *src);

#endif
