#ifndef _MACROS_H_
#define _MACROS_H_

#include <stdbool.h>
#include <stddef.h>

/* Array macros. */
#define array_size(a) (sizeof(a) / sizeof(*(a)))

/* Explicit type conversions macros. */
// LD, LU, LLD, LLU for explicit conversion of integer
#define LD(v) ((long)(v))
#define LU(v) ((unsigned long)(v))
#define LLD(v) ((long long)(v))
#define LLU(v) ((unsigned long long)(v))

/* Flag manipulation macros. */
#define CHECK_FLAG(V, F) ((V) & (F))
#define SET_FLAG(V, F) (V) |= (F)
#define UNSET_FLAG(V, F) (V) &= ~(F)
#define RESET_FLAG(V) (V) = 0
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
#define ABS(n) ((n) > 0 ? (n) : -(n))
#define NABS(n) ((n) < 0 ? (n) : -(n))

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
#define IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c) (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_CNTRL(c) ((c) >= 0 && (c) < 0x20)
#define IS_GRAPH(c) ((c) >= 0x20 && (c) < 0x7F)
#define IS_HEX(c) (IS_DIGIT(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define IS_LOWER(c) (((c) >= 'a' && (c) <= 'z'))
#define IS_UPPER(c) (((c) >= 'A' && (c) <= 'Z'))
#define LOWER(c) ((c) | 0x20)
#define UPPER(c) ((c) & ~0x20)

/* Print color macros */
#define GRAY "\x1b[30m"
#define L_GRAY "\x1b[1;30m"
#define NOCOLOR "\x1b[0m"

/* Debug macros */
#ifdef PRINT_DEBUG
// #define printd(...) printf(__VA_ARGS__)
// fprintf(stdout, GRAY "%s %d] " NOCOLOR fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#define printd(fmt, ...)                                  \
    do {                                                  \
        fprintf(stdout, GRAY fmt NOCOLOR, ##__VA_ARGS__); \
        fflush(stdout);                                   \
    } while (0)
#else
#define printd(...)
#endif

#ifdef PRINT_ERROR
#define printe(...) fprintf(stderr, __VA_ARGS__)
#else
#define printe(...)
#endif

/* CPP features related macros */
/* */

#endif