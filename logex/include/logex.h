#ifdef __LOGEX_H__
#error "logex.h included twice.  This will cause errors."
#endif

//#include <stdio.h>
//#include <stdarg.h>
//#include <sys/time.h>
//#include <string.h>
//#include <time.h>


#include "logex-levels.h"
#ifdef LOGEX_BUILD_LIBRARY
#include "liblogex.h"
#else //ifdef LOGEX_BUILD_LIBRARY
#if defined(LOGEX_MAIN_SOURCE)
    #ifdef LOGEX_DEFAULT_LOG_LEVEL
    int _logex_global_default = LOGEX_DEFAULT_LOG_LEVEL;
    #else
    int _logex_global_default = ERROR;
    #endif
#elif defined(LOGEX_LIB_BUILD)
static int _logex_global_default = ERROR;
#else
extern int _logex_global_default;
#define set_log_level_str(x) _logex_logger.level = level_string_to_enum(x)
#define set_log_level(x) _logex_logger.level = x;
#endif // !defined() && !defined()
#include "logex-header.h"
#endif //ifdef LOGEX_BUILD_LIBRARY
