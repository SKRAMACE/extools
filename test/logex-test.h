#ifndef __LOGEX_TEST_H__
#define __LOGEX_TEST_H__

#define TEST_DEFAULT(expect) \
    printf("%s default value: %s (expecting %s)\n", __FILE__, LEVEL_STRING(LOG_LEVEL), LEVEL_STRING(expect))

void other_file_default_test(int expect);

#endif
