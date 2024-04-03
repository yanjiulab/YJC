#ifndef _MACROS_H_
#define _MACROS_H_

#include "color.h"
#include <stdbool.h>
#include <stddef.h>

/* Array macros. */
#define array_size(a)      (sizeof(a) / sizeof(*(a)))

/* Explicit type conversions macros. */
// LD, LU, LLD, LLU for explicit conversion of integer
#define LD(v)              ((long)(v))
#define LU(v)              ((unsigned long)(v))
#define LLD(v)             ((long long)(v))
#define LLU(v)             ((unsigned long long)(v))

/* Flag manipulation macros. */
#define CHECK_FLAG(V, F)   ((V) & (F))
#define SET_FLAG(V, F)     (V) |= (F)
#define UNSET_FLAG(V, F)   (V) &= ~(F)
#define RESET_FLAG(V)      (V) = 0
#define COND_FLAG(V, F, C) ((C) ? (SET_FLAG(V, F)) : (UNSET_FLAG(V, F)))

/* Math related macros. */
#define MAX(a, b)                          \
    ({                                     \
        typeof(a) _max_a = (a);            \
        typeof(b) _max_b = (b);            \
        _max_a > _max_b ? _max_a : _max_b; \
    })
#define MIN(a, b)                          \
    ({                                     \
        typeof(a) _min_a = (a);            \
        typeof(b) _min_b = (b);            \
        _min_a < _min_b ? _min_a : _min_b; \
    })
#define numcmp(a, b)                                          \
    ({                                                        \
        typeof(a) _cmp_a = (a);                               \
        typeof(b) _cmp_b = (b);                               \
        (_cmp_a < _cmp_b) ? -1 : ((_cmp_a > _cmp_b) ? 1 : 0); \
    })
#define ABS(n)         ((n) > 0 ? (n) : -(n))
#define NABS(n)        ((n) < 0 ? (n) : -(n))

/* ASCII charaters related macros.

    [0, 0x20)    control-charaters
    [0x20, 0x7F) printable-charaters

    0x0A => LF
    0x0D => CR
    0x20 => SPACE
    0x7F => DEL

    [0x09, 0x0D] => \t\n\v\f\r
    [0x30, 0x39] => 0~9
    [0x41, 0x5A] => A~Z
    [0x61, 0x7A] => a~z
*/
#define IS_ALPHA(c)    (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c)    ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_CNTRL(c)    ((c) >= 0 && (c) < 0x20)
#define IS_GRAPH(c)    ((c) >= 0x20 && (c) < 0x7F)
#define IS_HEX(c)      (IS_DIGIT(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define IS_LOWER(c)    (((c) >= 'a' && (c) <= 'z'))
#define IS_UPPER(c)    (((c) >= 'A' && (c) <= 'Z'))
#define LOWER(c)       ((c) | 0x20)
#define UPPER(c)       ((c) & ~0x20)

#define CR             '\r'
#define LF             '\n'
#define CRLF           "\r\n"

/* Debug macros */
#ifdef PRINT_DEBUG
// #define printd(...) printf(__VA_ARGS__)
// fprintf(stdout, GRAY "%s %d] " NOCOLOR fmt, __FILE__, __LINE__, ##__VA_ARGS__);
// fprintf(stdout, L_DKGREY "[%s:%d:%s] ", __FILE__, __LINE__, __FUNCTION__);
#define printd(fmt, ...)                                          \
    do {                                                          \
        fprintf(stdout, L_DKGREY fmt NORMAL CRLF, ##__VA_ARGS__); \
        fflush(stdout);                                           \
    } while (0)

#define printe(fmt, ...)                                     \
    do {                                                     \
        fprintf(stderr, RED fmt NORMAL CRLF, ##__VA_ARGS__); \
        fflush(stderr);                                      \
    } while (0)
#else
#define printd(...)
#define printe(...)
#endif

/* Stream macros
 * PUT_NET puts "network ordered" data to the datastream.
 * PUT_HOST puts "host ordered" data to the datastream.
 * GET_NET gets the data and keeps it in "network order" in the memory
 * GET_HOST gets the data, but in the memory it is in "host order"
 * The same for all {PUT,GET}_{NET,HOST}{16,32,64}
 */

#define GET_BYTE(val, cp) ((val) = *(cp)++)
#define PUT_BYTE(val, cp) (*(cp)++ = (uint8_t)(val))

#define GET_HOST16(val, cp)   \
    do {                      \
        register uint16_t Xv; \
        Xv = (*(cp)++) << 8;  \
        Xv |= *(cp)++;        \
        (val) = Xv;           \
    } while (0)

#define PUT_HOST16(val, cp)           \
    do {                              \
        register uint16_t Xv;         \
        Xv = (uint16_t)(val);         \
        *(cp)++ = (uint8_t)(Xv >> 8); \
        *(cp)++ = (uint8_t)Xv;        \
    } while (0)

#define GET_NET16(val, cp)    \
    do {                      \
        register uint16_t Xv; \
        Xv = *(cp)++;         \
        Xv |= (*(cp)++) << 8; \
        (val) = Xv;           \
    } while (0)

#define PUT_NET16(val, cp)            \
    do {                              \
        register uint16_t Xv;         \
        Xv = (uint16_t)(val);         \
        *(cp)++ = (uint8_t)Xv;        \
        *(cp)++ = (uint8_t)(Xv >> 8); \
    } while (0)

#define GET_HOST32(val, cp)    \
    do {                       \
        register uint32_t Xv;  \
        Xv = (*(cp)++) << 24;  \
        Xv |= (*(cp)++) << 16; \
        Xv |= (*(cp)++) << 8;  \
        Xv |= *(cp)++;         \
        (val) = Xv;            \
    } while (0)

#define PUT_HOST32(val, cp)            \
    do {                               \
        register uint32_t Xv;          \
        Xv = (uint32_t)(val);          \
        *(cp)++ = (uint8_t)(Xv >> 24); \
        *(cp)++ = (uint8_t)(Xv >> 16); \
        *(cp)++ = (uint8_t)(Xv >> 8);  \
        *(cp)++ = (uint8_t)Xv;         \
    } while (0)

#define GET_NET32(val, cp)     \
    do {                       \
        register uint32_t Xv;  \
        Xv = *(cp)++;          \
        Xv |= (*(cp)++) << 8;  \
        Xv |= (*(cp)++) << 16; \
        Xv |= (*(cp)++) << 24; \
        (val) = Xv;            \
    } while (0)

#define PUT_NET32(val, cp)             \
    do {                               \
        register uint32_t Xv;          \
        Xv = (uint32_t)(val);          \
        *(cp)++ = (uint8_t)Xv;         \
        *(cp)++ = (uint8_t)(Xv >> 8);  \
        *(cp)++ = (uint8_t)(Xv >> 16); \
        *(cp)++ = (uint8_t)(Xv >> 24); \
    } while (0)

#define GET_HOST64(val, cp)    \
    do {                       \
        register uint64_t Xv;  \
        Xv = (*(cp)++) << 56;  \
        Xv |= (*(cp)++) << 48; \
        Xv |= (*(cp)++) << 40; \
        Xv |= (*(cp)++) << 32; \
        Xv |= (*(cp)++) << 24; \
        Xv |= (*(cp)++) << 16; \
        Xv |= (*(cp)++) << 8;  \
        Xv |= *(cp)++;         \
        (val) = Xv;            \
    } while (0)

#define PUT_HOST64(val, cp)            \
    do {                               \
        register uint64_t Xv;          \
        Xv = (uint64_t)(val);          \
        *(cp)++ = (uint8_t)(Xv >> 56); \
        *(cp)++ = (uint8_t)(Xv >> 48); \
        *(cp)++ = (uint8_t)(Xv >> 40); \
        *(cp)++ = (uint8_t)(Xv >> 32); \
        *(cp)++ = (uint8_t)(Xv >> 24); \
        *(cp)++ = (uint8_t)(Xv >> 16); \
        *(cp)++ = (uint8_t)(Xv >> 8);  \
        *(cp)++ = (uint8_t)Xv;         \
    } while (0)

/* CPP features related macros */
/* */

#endif