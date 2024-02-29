// LOGEX_TAG prints before every log message in this file
#define LOGEX_TAG "MAIN"

// logex-main.h must be included in the entrypoint source file
#include "logex-main.h"

#include "examples.h"

// Optional: Associate modules with environment variables
// Even if no modules are given, there is always a "LOGEX_DEFAULT_LOG_LEVEL" env var
//      Note: Only available if LOGEX_MODULES_START and LOGEX_MODULES_END are used
LOGEX_MODULES_START

// First value is module name, and must match up with the LOGEX_MODULE() macro in other source files
LOGEX_ADD_MODULE(sub, SUB_LOG_LEVEL)

// The source including "logex-main.h" is automatically the "main" module, but the env var is not set
LOGEX_ADD_MODULE(main, MAIN_LOG_LEVEL)

//... Add as many of these as you want
// LOGEX_MODULES_END is mandatory
LOGEX_MODULES_END

void
main_hello()
{
    // Highest Priority
    fatal("Hello");
    critical("Hello");
    error("Hello");
    warning("Hello");
    info("Hello");
    verbose("Hello");
    command("Hello");
    debug("Hello");
    trace("Hello");
    // Lowest Priority
}

int
main(int argc, char *argv[])
{
    // This macro executes the LOGEX_ADD_MODULE macros
    LOGEX_INIT_MODULES();

    main_hello();
    sub_hello();
    simple_hello();
}
