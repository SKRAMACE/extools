#ifndef LOGEX_LIB_BUILD
#define LOGEX_LIB_BUILD

#ifdef LOGEX_LIB_BUILD
#error "logex-lib.h included after logex-main.h"
#endif

#include <logex.h>

#define set_log_level_str(x) _logex_logger.level = level_string_to_enum(x)
#define set_log_level(x) _logex_logger.level = x

#endif
