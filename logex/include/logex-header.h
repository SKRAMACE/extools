#ifndef __LOGEX_HDR_H__
#define __LOGEX_HDR_H__

#include <ctype.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#define LOG_LEVEL ((_logex_logger.level == DEFAULT) ? _logex_global_default : _logex_logger.level)
#define LOG_LEVEL_STR LEVEL_STRING(LOG_LEVEL)

#define LOGEX_MODULE(mod) struct logger_t *_logex_##mod##_logger = &_logex_logger

// Generic log call macros
#define _LOG(lvl, s, ...) _logex_log(lvl, #lvl, s, ##__VA_ARGS__)

// Log level call macros
#define logex(s, ...) _LOG(LOGEX, s, ##__VA_ARGS__)
#define fatal(s, ...) _LOG(FATAL, s, ##__VA_ARGS__)
#define critical(s, ...) _LOG(CRITICAL, s, ##__VA_ARGS__)
#define error(s, ...) _LOG(ERROR, s, ##__VA_ARGS__)
#define warn(s, ...) _LOG(WARNING, s, ##__VA_ARGS__)
#define warning warn
#define info(s, ...) _LOG(INFO, s, ##__VA_ARGS__)
#define verbose(s, ...) _logex_log(VERBOSE, "INFO", s, ##__VA_ARGS__)
#define command(s, ...) _LOG(CMD, s, ##__VA_ARGS__)
#define cmd command
#define debug(s, ...) _LOG(DEBUG, s, ##__VA_ARGS__)
#define trace(s, ...) _LOG(TRACE, s, ##__VA_ARGS__)

// Print the log level outside of logging mechanism
#define show_log_level() printf("---Log Level set to \"%s\"\n", LOG_LEVEL_STR)

// Log Settings
#define console_off() _logex_logger.console = 0;
#define console_on() _logex_logger.console = 1;
#define log_file_on(x) snprintf(_logex_logger.fname, LOGEX_FNAME_LEN, "%s", x); _logex_logger.fname[LOGEX_FNAME_LEN-1] = 0
#define log_file_off() _logex_logger.fname[0] = 0
#define log_file_clear() if (_logex_logger.fname[0]) { FILE *f = fopen(_logex_logger.fname, "w"); fclose(f); }

#define LOGEX_FNAME_LEN 1024
#define LOGEX_TAG_LEN 64
#define LOGEX_TIMESTAMP_LEN 64
#define LOGEX_TIMESTAMP_DEFAULT_FMT "%Y-%m-%d %H:%M:%S"

struct logger_t {
    int level;
    char fname[LOGEX_FNAME_LEN];
    int console;
    int milliseconds;
    char *_tag;
    char *file;
    char tag[LOGEX_TAG_LEN];
    const char *timestamp_fmt;
};

// Each file including this header transparently receives its own logger struct
static struct logger_t _logex_logger = {
    .level=DEFAULT,
    .fname={0},
    .console=1,
    .milliseconds=1,

#ifdef LOGEX_TAG
    ._tag=LOGEX_TAG,
#else
    ._tag=NULL,
#endif
    .file=NULL,
    .tag={0},
#ifdef LOGEX_TIMESTAMP_FMT
    .timestamp_fmt=LOGEX_TIMESTAMP_FMT,
#else
    .timestamp_fmt=LOGEX_TIMESTAMP_DEFAULT_FMT,
#endif
};

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
    if (_logex_logger.milliseconds) {
        snprintf(ms, 8, ".%06lu", tv.tv_usec);
    }
    snprintf(timestamp, bytes, "%s%s", timestr, ms);
}


static void
_logex_log(int level, const char *lvl_str, const char *fmt, ...)
{
    int cur_level = LOG_LEVEL;
    if (cur_level > level) {
        return;
    }

    char _logex_timestamp[LOGEX_TIMESTAMP_LEN] = {0};
    _logex_gen_timestamp(_logex_timestamp, LOGEX_TIMESTAMP_LEN);

    va_list args;
    va_start(args, fmt);

    if (!_logex_logger.tag[0]) {
        if (_logex_logger._tag) {
            char *x = _logex_logger._tag;
            char *y = _logex_logger.tag;
            for (int i = 0; i < LOGEX_TAG_LEN; i++) {
                if (x[i] == 0) {
                    break;
                }
                y[i] = (char)toupper((int)x[i]);
            }
        } else if (_logex_logger.file) {
            char *x = _logex_logger.file;
            char *y = _logex_logger.tag;
            for (int i = 0; i < LOGEX_TAG_LEN; i++) {
                if (x[i] == 0) {
                    *y++ = x[i];
                    break;
                } else if (x[i] == '/') {
                    y = _logex_logger.tag;
                    continue;
                }
                *y++ = x[i];
            }
        } else {
            snprintf(_logex_logger.tag, 2, " ");
        }
        printf("*tag = %s\n", _logex_logger.tag);
    }

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
_logex_log_print(int level, const char *lvl_str, const char *fmt, ...)
{
    char _logex_timestamp[LOGEX_TIMESTAMP_LEN] = {0};
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
    _logex_log_print(LOGEX, "LOGEX", "%s=%s%s%s", str, LEVEL_COLOR(LOG_LEVEL), LOG_LEVEL_STR, LEVEL_COLOR(RESET));
}

#endif
