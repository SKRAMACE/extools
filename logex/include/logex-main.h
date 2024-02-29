#ifndef LOGEX_MAIN_SOURCE
#define LOGEX_MAIN_SOURCE

#ifdef LOGEX_LIB_BUILD
#error "logex-main.h included after logex-lib.h"
#endif

#include <string.h>
//#include <stdarg.h>
#include <stdlib.h>
//#include <ctype.h>
#include "logex.h"
#include "logex-levels.h"

int
logex_level_string_to_enum(const char *lvl_str)
{
    if (!lvl_str || !lvl_str[0]) {
        return DEFAULT;
    }

    // Copy level string to local buffer
    char s[LOGEX_LVL_STR_MAX];
    int c = snprintf(s, LOGEX_LVL_STR_MAX, "%s", lvl_str);
    s[LOGEX_LVL_STR_MAX - 1] = 0;

    // Convert each char to uppercase
    // Break on NULL
    for (int i = 0; i < LOGEX_LVL_STR_MAX; i++) {
        if (s[i] == 0) {
            break;
        }

        s[i] = (char)toupper((int)s[i]);
    }

    return (0 == strncmp(s, "LOGEX", 5)) ? LOGEX :
        (0 == strncmp(s, "FATAL", 5)) ? FATAL :
        (0 == strncmp(s, "CRITICAL", 8)) ? CRITICAL :
        (0 == strncmp(s, "ERROR", 5)) ? ERROR :
        (0 == strncmp(s, "WARN", 4)) ? WARNING :
        (0 == strncmp(s, "INFO", 4)) ? INFO :
        (0 == strncmp(s, "VERBOSE", 7)) ? VERBOSE :
        (0 == strncmp(s, "CMD", 3)) ? CMD :
        (0 == strncmp(s, "DEBUG", 5)) ? DEBUG :
        (0 == strncmp(s, "TRACE", 5)) ? TRACE :
        DEFAULT;
}

#define set_log_level_default(x) _logex_global_default = (x >= RESET) ? ERROR : x
#define set_log_level_default_str(x) _logex_global_default = logex_level_string_to_enum(x)

struct log_modules_t {
    const char *envvar;
    void (*fn)(char *);
};

#define LOGEX_MODULES_START static void _logex_init_modules() { \
    char *envdefault = getenv("LOGEX_DEFAULT_LOG_LEVEL"); \
    if (envdefault) { _logex_global_default = logex_level_string_to_enum(envdefault);} \
    if (_logex_global_default == DEFAULT) { _logex_global_default = ERROR; }

#define LOGEX_ADD_MODULE(mod,e) \
    extern struct logger_t *_logex_##mod##_logger; \
    char *env##mod = getenv(#e); \
    _logex_##mod##_logger->level = logex_level_string_to_enum(env##mod);

#define LOGEX_MODULES_END }
#define LOGEX_INIT_MODULES() _logex_init_modules()

LOGEX_MODULE(main);

#endif
