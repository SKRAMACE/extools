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

#define ENVEX_COPY(v,b,x,d) \
    if (getenv(x)) { \
        strncpy(v, getenv(x), b); \
    } else if (d) { \
        strncpy(v, d, b); \
    } else { \
        *v = 0; \
    }

#define ENVEX_TOSTR(v,x,d) \
    {*(char **)&v = calloc(1024, 1); \
    if (getenv(x)) { \
        strncpy(v, getenv(x), 1023); \
    } else if (d) { \
        strncpy(v, d, 1023); \
    } else { \
        *v = 0; \
    }}

#define ENVEX_TOSTR_LOWER(v,x,d) \
    ENVEX_TOSTR(v,x,d) \
    {unsigned char *p = (unsigned char *)v; \
    while (*p) { \
        if (*p > 0x40 && *p < 0x5B) { *p = *p + 0x20; } \
        p++; \
    }}

#define ENVEX_TOSTR_UPPER(v,x,d) \
    ENVEX_TOSTR(v,x,d) \
    {unsigned char *p = (unsigned char *)v; \
    while (*p) { \
        if (*p > 0x60 && *p < 0x7B) { *p = *p - 0x20; } \
        p++; \
    }}

#define ENVEX_EXISTS(x) \
    (getenv(x) ? 1 : 0)

#define ENVEX_ASSERT(x,msg) \
    if (x) { \
        printf("ENV ERROR: %s\n", msg);\
        return ENVEX_ERROR; \
    }

#endif
