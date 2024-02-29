#ifndef __LOGEX_LEVELS_H__
#define __LOGEX_LEVELS_H__

#define LOGEX_LVL_STR_MAX 16
enum logging_level_e {
    DEFAULT=101,
    RESET=100,
    LOGEX=99,
    FATAL=90,
    CRITICAL=80,
    ERROR=70,
    WARNING=60,
    INFO=50,
    VERBOSE=40,
    CMD=30,
    DEBUG=20,
    TRACE=10
};

#define LEVEL_COLOR(x) \
    (x == LOGEX) ? "\e[94m" : \
    (x == FATAL) ? "\e[41;97m" : \
    (x == CRITICAL) ? "\e[41;97m" : \
    (x == ERROR) ? "\e[1;31m" : \
    (x == WARNING) ? "\e[1;33m" : \
    (x == INFO) ? "\e[97m" : \
    (x == VERBOSE) ? "\e[97m" : \
    (x == CMD) ? "\e[92m" : \
    (x == DEBUG) ? "\e[1;34m" : \
    (x == TRACE) ? "\e[94m" : \
    "\e[0m"

#define LEVEL_STRING(x) \
    (x == LOGEX) ? "LOGEX" : \
    (x == FATAL) ? "FATAL" : \
    (x == CRITICAL) ? "CRITICAL" : \
    (x == ERROR) ? "ERROR" : \
    (x == WARNING) ? "WARNING" : \
    (x == INFO) ? "INFO" : \
    (x == VERBOSE) ? "INFO" : \
    (x == CMD) ? "CMD" : \
    (x == DEBUG) ? "DEBUG" : \
    (x == TRACE) ? "TRACE" : \
    "UNKNOWN"

#define LEVEL_STRING_TO_ENUM(x) { \
    char s[9]; \
    snprintf(s, 9, "%s", x); s[8] = 0; \
    for (int i = 0; i < 9; i++) { \
        int c = (int)s[i]; \
        if (!c) { \
            break; \
        } \
        s[i] = toupper(c); \
    } \
    _logex_logger.level = \
    (0 == strncmp(s, "LOGEX", 5)) ? LOGEX : \
    (0 == strncmp(s, "FATAL", 5)) ? FATAL : \
    (0 == strncmp(s, "CRITICAL", 8)) ? CRITICAL : \
    (0 == strncmp(s, "ERROR", 5)) ? ERROR : \
    (0 == strncmp(s, "WARN", 4)) ? WARNING : \
    (0 == strncmp(s, "INFO", 4)) ? INFO : \
    (0 == strncmp(s, "VERBOSE", 7)) ? VERBOSE : \
    (0 == strncmp(s, "CMD", 3)) ? CMD : \
    (0 == strncmp(s, "DEBUG", 5)) ? DEBUG : \
    (0 == strncmp(s, "TRACE", 5)) ? TRACE : \
    DEFAULT; \
}
#endif
