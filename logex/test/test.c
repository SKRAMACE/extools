#include <stdio.h>

#define LOGEX_MAIN
#define LOGEX_TAG "MAIN"
#include "logex.h"
#include "logex-test.h"

void
run_all_levels()
{
    show_log_level();
    fatal("test");
    critical("test");
    error("test");
    warning("test");
    info("test");
    verbose("test");
    cmd("test");
    debug("test");
    trace("test");
    printf("\n");
}

int
main(int argc, char *argv[])
{
    error("<< this should say \"%s\"", LOGEX_TAG);

    TEST_DEFAULT(ERROR);
    set_log_level_str("CRITICAL");
    TEST_DEFAULT(CRITICAL);
    other_file_default_test(CRITICAL);
    TEST_DEFAULT(CRITICAL);
    run_all_levels();

    set_log_level(FATAL);
    run_all_levels();

    set_log_level(TRACE);
    run_all_levels();

    set_log_level_str("info");
    run_all_levels();

    set_log_level_str("DEBUG");
    run_all_levels();

    char *fname = "/tmp/hello.log";
    printf("---Switching output to \"%s\"\n", fname);
    log_file_on(fname);
    log_file_clear();
    console_off();
    run_all_levels();
}
