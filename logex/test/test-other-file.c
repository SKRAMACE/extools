#include <stdio.h>

#define LOGEX_TAG "OTHER"
#include "logex-std.h"
#include "logex-test.h"


void
other_file_default_test(int expect)
{
    TEST_DEFAULT(expect);
    set_log_level(INFO);
    TEST_DEFAULT(INFO);
}
