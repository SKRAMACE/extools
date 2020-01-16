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

#define ENVEX_TOINT(v,x,d) \
    v = (getenv(x)) ? atoi(getenv(x)) : d

#define ENVEX_TOFLOAT(v,x,d) \
    v = (getenv(x)) ? atof(getenv(x)) : (float)d

#define ENVEX_HEXINT(v,x,d) \
    v = (getenv(x)) ? strtol(getenv(x), NULL, 16) : d

char *
envex_str(char *dst, char *name, char *def)
{
    char *s = calloc(1024, 1);

    char *x = getenv(name);
    if (x) {
        strncpy(s, x, 1023);
    } else if (def) {
        strncpy(s, def, 1023);
    } else {
        *s = 0;
    }

    return s;
}

#define ENVEX_TOSTR(v,x,d) \
    envex_str(v,x,d)

#define ENVEX_EXISTS(x) \
    (getenv(x) ? 1 : 0)

#define ENVEX_ASSERT(x,msg) \
    if (x) { \
        printf("ENV ERROR: %s\n", msg);\
        return ENVEX_ERROR; \
    }

#endif
