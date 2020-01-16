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

#define ENVEX_INT8(v,x,d) \
    v = (getenv(x)) ? (int8_t)atoi(getenv(x)) : (int8_t)d

#define ENVEX_UINT8(v,x,d) \
    v = (getenv(x)) ? (uint8_t)atoi(getenv(x)) : (uint8_t)d

#define ENVEX_INT16(v,x,d) \
    v = (getenv(x)) ? (int16_t)atoi(getenv(x)) : (int16_t)d

#define ENVEX_UINT16(v,x,d) \
    v = (getenv(x)) ? (uint16_t)atoi(getenv(x)) : (uint16_t)d

#define ENVEX_INT32(v,x,d) \
    v = (getenv(x)) ? (int32_t)atoi(getenv(x)) : (int32_t)d

#define ENVEX_UINT32(v,x,d) \
    v = (getenv(x)) ? (uint32_t)atoi(getenv(x)) : (uint32_t)d

#define ENVEX_FLOAT(v,x,d) \
    v = (getenv(x)) ? (float)atof(getenv(x)) : (float)d

#define ENVEX_DOUBLE(v,x,d) \
    v = (getenv(x)) ? atof(getenv(x)) : (double)d

#define ENVEX_HEX32(v,x,d) \
    v = (getenv(x)) ? (uint32_t)strtol(getenv(x), NULL, 16) : (uint32_t)d

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
