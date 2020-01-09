#ifndef __ENVEX_H__
#define __ENVEX_H__

#include <stdlib.h>
#include <stdio.h>

#define ENVEX_SUCCESS   0
#define ENVEX_ERROR     1

#define ENVEX_REQUIRED(x) \
    if (!getenv(x)) { \
        printf("ENV ERROR: Missing \"%s\"\n", x);\
        return ENVEX_ERROR; \
    }

#define ENVEX_TOINT(x) \
    (getenv(x)) ? atoi(getenv(x)) : 0

#define ENVEX_HEXINT(x) \
    (getenv(x)) ? strtol(getenv(x), NULL, 16) : 0

char *
envex_str(char *name)
{
    char *x = getenv(name);

    char *s = calloc(1024, 1);
    strncpy(s, x, 1023);

    if (!x) {
        *s = 0;
    }

    return s;
}

#define ENVEX_TOSTR(x) \
    envex_str(x)

#define ENVEX_EXISTS(x) \
    (getenv(x) ? 1 : 0)

#define ENVEX_ASSERT(x,msg) \
    if (x) { \
        printf("ENV ERROR: %s\n", msg);\
        return ENVEX_ERROR; \
    }

 #endif
