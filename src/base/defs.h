#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <stddef.h>

#include "macros.h"
#include "math.h"

#define FLOAT_PRECISION 1e-6
#define FLOAT_EQUAL_ZERO(f) (ABS(f) < FLOAT_PRECISION)

#ifndef INFINITE
#define INFINITE (uint32_t) - 1
#endif

#ifndef _WIN32

// MAKEWORD, HIBYTE, LOBYTE
#ifndef MAKEWORD
#define MAKEWORD(h, l) ((((WORD)h) << 8) | (l & 0xff))
#endif

#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((WORD)w) >> 8))
#endif

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)(w & 0xff))
#endif

// MAKELONG, HIWORD, LOWORD
#ifndef MAKELONG
#define MAKELONG(h, l) (((int32_t)h) << 16 | (l & 0xffff))
#endif

#ifndef HIWORD
#define HIWORD(n) ((WORD)(((int32_t)n) >> 16))
#endif

#ifndef LOWORD
#define LOWORD(n) ((WORD)(n & 0xffff))
#endif

#endif // _WIN32

// MAKEINT64, HIINT, LOINT
#ifndef MAKEINT64
#define MAKEINT64(h, l) (((int64_t)h) << 32 | (l & 0xffffffff))
#endif

#ifndef HIINT
#define HIINT(n) ((int32_t)(((int64_t)n) >> 32))
#endif

#ifndef LOINT
#define LOINT(n) ((int32_t)(n & 0xffffffff))
#endif

#ifndef MAKE_FOURCC
#define MAKE_FOURCC(a, b, c, d)                               \
    (((uint32)d) | (((uint32)c) << 8) | (((uint32)b) << 16) | \
     (((uint32)a) << 24))
#endif

#ifndef LIMIT
#define LIMIT(lower, v, upper)                         \
    ((v) < (lower) ? (lower) : (v) > (upper) ? (upper) \
                                             : (v))
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef SAFE_ALLOC
#define SAFE_ALLOC(p, size)                      \
    do {                                         \
        void* ptr = malloc(size);                \
        if (!ptr) {                              \
            fprintf(stderr, "malloc failed!\n"); \
            exit(-1);                            \
        }                                        \
        memset(ptr, 0, size);                    \
        *(void**)&(p) = ptr;                     \
    } while (0)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)    \
    do {                \
        if (p) {        \
            free(p);    \
            (p) = NULL; \
        }               \
    } while (0)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  \
    do {                \
        if (p) {        \
            delete (p); \
            (p) = NULL; \
        }               \
    } while (0)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) \
    do {                     \
        if (p) {             \
            delete[] (p);    \
            (p) = NULL;      \
        }                    \
    } while (0)
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     \
    do {                    \
        if (p) {            \
            (p)->release(); \
            (p) = NULL;     \
        }                   \
    } while (0)
#endif

#ifndef SAFE_CLOSE
#define SAFE_CLOSE(fd)   \
    do {                 \
        if ((fd) >= 0) { \
            close(fd);   \
            (fd) = -1;   \
        }                \
    } while (0)
#endif

#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x

#define STRINGCAT(x, y) STRINGCAT_HELPER(x, y)
#define STRINGCAT_HELPER(x, y) x##y

#ifndef offsetof
#define offsetof(type, member) ((size_t)(&((type*)0)->member))
#endif

#ifndef offsetofend
#define offsetofend(type, member) \
    (offsetof(type, member) + sizeof(((type*)0)->member))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
    ((type*)((char*)(ptr)-offsetof(type, member)))
#endif

/* helper to get type safety/avoid casts on calls
 * (w/o this, functions accepting all prefix types need casts on the caller
 * side, which strips type safety since the cast will accept any pointer
 * type.)
 */
#ifndef __cplusplus
#define prefixtype(uname, typename, fieldname) typename* fieldname;
#define TRANSPARENT_UNION __attribute__((transparent_union))
#else
#define prefixtype(uname, typename, fieldname) \
    typename* fieldname;                       \
    uname(typename* x) {                       \
        this->fieldname = x;                   \
    }
#define TRANSPARENT_UNION
#endif

/*
 * Add explicit static cast only when using a C++ compiler.
 */
#ifdef __cplusplus
#define static_cast(l, r) static_cast<decltype(l)>((r))
#else
#define static_cast(l, r) (r)
#endif

#endif // !DEFS_H