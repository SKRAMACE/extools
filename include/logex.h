/************
LOGEX is a logging infrastructure made up of only include files

This header must be preceeded by one of the following DEFINES:

If the source contains the main() function, use this:
#define LOGEX_MAIN

Otherwise, use this:
#define LOGEX_STD

For a library build, use this:
#define LOGEX_LIB

The following include directives are equivalent to the above defines:
#include <logex-main.h>
#include <logex-std.h>
#include <logex-lib.h>

The following OPTIONAL defines add extra functionality
#define LOGEX_TAG ""
#define LOGEX_TIMESTAMP_FMT ""
#define LOGEX_
*/

#ifndef __LOGEX_H__
#define __LOGEX_H__

#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define LOGEX_FNAME_LEN 1024
#define LOGEX_TAG_LEN 64
#define LOGEX_TIMESTAMP_LEN 64
#define LOGEX_TIMESTAMP_FMT "%Y-%m-%d %H:%M:%S"

enum logging_level_e {
    DEFAULT=101,
    RESET=100,
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

#define LOG_LEVEL ((_logex_logger.level == DEFAULT) ? _logex_global_default : _logex_logger.level)
#define LOG_LEVEL_STR LEVEL_STRING(LOG_LEVEL)
#define _LOG(lvl, s, ...) _logex_log(lvl, #lvl, s, ##__VA_ARGS__)

// LOGGING CALLS
#define fatal(s, ...) _LOG(FATAL, s, ##__VA_ARGS__)
#define critical(s, ...) _LOG(CRITICAL, s, ##__VA_ARGS__)
#define error(s, ...) _LOG(ERROR, s, ##__VA_ARGS__)
#define warn(s, ...) _LOG(WARNING, s, ##__VA_ARGS__)
#define warning warn
#define info(s, ...) _LOG(INFO, s, ##__VA_ARGS__)
#define verbose(s, ...) _logex_log(VERBOSE, "INFO", s, ##__VA_ARGS__)
#define cmd(s, ...) _LOG(CMD, s, ##__VA_ARGS__)
#define debug(s, ...) _LOG(DEBUG, s, ##__VA_ARGS__)
#define trace(s, ...) _LOG(TRACE, s, ##__VA_ARGS__)

// LOGGING SETTINGS
#define show_log_level() printf("---Log Level set to \"%s\"\n", LOG_LEVEL_STR)
#define console_off() _logex_logger.console = 0;
#define console_on() _logex_logger.console = 1;
#define log_file_on(x) snprintf(_logex_logger.fname, LOGEX_FNAME_LEN, "%s", x); _logex_logger.fname[LOGEX_FNAME_LEN-1] = 0
#define log_file_off() _logex_logger.fname[0] = 0
#define log_file_clear() if (_logex_logger.fname[0]) { FILE *f = fopen(_logex_logger.fname, "w"); fclose(f); }

#if defined(LOGEX_MAIN)
    int _logex_global_default = ERROR;
    #define _set_global_default(x) _logex_global_default = (x >= RESET) ? ERROR : x
    #define set_log_level_str(x) LEVEL_STRING_TO_ENUM(x); _set_global_default(_logex_logger.level)
    #define set_log_level(x) _logex_logger.level = x; _set_global_default(x)
#elif   defined(LOGEX_STD)
    extern int _logex_global_default;
    #define set_log_level_str(x) LEVEL_STRING_TO_ENUM(x)
    #define set_log_level(x) _logex_logger.level = x;
#elif   defined(LOGEX_LIB)
    static int _logex_global_default = ERROR;
    #define set_log_level_str(x) LEVEL_STRING_TO_ENUM(x)
    #define set_log_level(x) _logex_logger.level = x;
#else
    int _logex_global_default = ERROR;
    #define set_log_level_str(x)
    #define set_log_level(x)
    #error Logex missing required defines (see logex.h for details)
#endif

struct logger_t {
    int level;
    char fname[LOGEX_FNAME_LEN];
    int console;
    int miliseconds;
    const char *tag;
    const char *timestamp_fmt;
};

// Each file including this header transparently receives its own logger struct
static struct logger_t _logex_logger = {
    .level=DEFAULT,
    .fname={0},
    .console=1,
    .miliseconds=1,

#ifdef LOGEX_TAG
    .tag=LOGEX_TAG,
#else
    .tag=NULL,
#endif

#ifdef LOGEX_TIMESTAMP_FMT
    .timestamp_fmt=LOGEX_TIMESTAMP_FMT,
#else
    .timestamp_fmt="%Y-%m-%d %H:%M:%S",
#endif
};

static char _logex_timestamp[LOGEX_TIMESTAMP_LEN] = {0};

static void
_logex_gen_timestamp(char *timestamp, size_t bytes)
{
    // Get time with microsecond precision
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert seconds to "struct tm" (compatible with strftime)
    time_t nowtime = tv.tv_sec;
    struct tm *nowtm = gmtime(&nowtime);

    char timestr[LOGEX_TIMESTAMP_LEN];
    strftime(timestr, LOGEX_TIMESTAMP_LEN, _logex_logger.timestamp_fmt, nowtm);

    char ms[8] = {0};
    if (_logex_logger.miliseconds) {
        snprintf(ms, 8, ".%06lu", tv.tv_usec);
    }
    snprintf(timestamp, bytes, "%s%s", timestr, ms);
}

static void
_logex_log_print(int level, const char *lvl_str, const char *fmt, ...)
{
    _logex_gen_timestamp(_logex_timestamp, LOGEX_TIMESTAMP_LEN);

    va_list args;
    va_start(args, fmt);

    if (_logex_logger.fname[0]) {
        FILE *f = fopen(_logex_logger.fname, "a+");
        fprintf(f, "%s [%s] ", _logex_timestamp, lvl_str);
        if (_logex_logger.tag) {
            fprintf(f, "%s: ", _logex_logger.tag);
        }
        vfprintf(f, fmt, args);
        fprintf(f, "\n");
        fclose(f);
    }

    if (_logex_logger.console) {
        fprintf(stdout, "%s [%s%s%s] ", _logex_timestamp,
            LEVEL_COLOR(level), lvl_str, LEVEL_COLOR(RESET));
        if (_logex_logger.tag) {
            fprintf(stdout, "%s: ", _logex_logger.tag);
        }
        vfprintf(stdout, fmt, args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(args);
}

static void
_logex_log_print_lvl(const char *str)
{
    _logex_log_print(INFO, "LOGEX", "%s=%s%s%s", str, LEVEL_COLOR(LOG_LEVEL), LOG_LEVEL_STR, LEVEL_COLOR(RESET));
}

static void
_logex_log(int level, const char *lvl_str, const char *fmt, ...)
{
    int cur_level = LOG_LEVEL;
    if (cur_level > level) {
        return;
    }

    _logex_gen_timestamp(_logex_timestamp, LOGEX_TIMESTAMP_LEN);

    va_list args;
    va_start(args, fmt);

    if (_logex_logger.fname[0]) {
        FILE *f = fopen(_logex_logger.fname, "a+");
        fprintf(f, "%s [%s] ", _logex_timestamp, lvl_str);
        if (_logex_logger.tag) {
            fprintf(f, "%s: ", _logex_logger.tag);
        }
        vfprintf(f, fmt, args);
        fprintf(f, "\n");
        fclose(f);
    }

    if (_logex_logger.console) {
        fprintf(stdout, "%s [%s%s%s] ", _logex_timestamp,
            LEVEL_COLOR(level), lvl_str, LEVEL_COLOR(RESET));
        if (_logex_logger.tag) {
            fprintf(stdout, "%s: ", _logex_logger.tag);
        }
        vfprintf(stdout, fmt, args);
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(args);
}

#endif      // ifndef (everything above this is only included once
