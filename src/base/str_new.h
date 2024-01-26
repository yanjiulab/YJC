/* A C dynamic strings library
 *
 * Copyright (c) 2006-2015, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2015, Oran Agra
 * Copyright (c) 2015, Redis Labs, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STR_H_
#define _STR_H_

#define STR_MAX_PREALLOC (1024*1024)
extern const char *STR_NOINIT;

#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>

typedef char *string_t;

/* STR allocator selection.
 *
 * This file is used in order to change the STR allocator at compile time.
 * Just define the following defines to what you want to use. Also add
 * the include of your alternate allocator if needed (not needed in order
 * to use the default libc allocator). */

#define s_malloc malloc
#define s_realloc realloc
#define s_free free

/* Note: strhdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 STR strings. */
struct __attribute__ ((__packed__)) strhdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__ ((__packed__)) strhdr8 {
    uint8_t len; /* used */
    uint8_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) strhdr16 {
    uint16_t len; /* used */
    uint16_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) strhdr32 {
    uint32_t len; /* used */
    uint32_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__ ((__packed__)) strhdr64 {
    uint64_t len; /* used */
    uint64_t alloc; /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};

#define STR_TYPE_5  0
#define STR_TYPE_8  1
#define STR_TYPE_16 2
#define STR_TYPE_32 3
#define STR_TYPE_64 4
#define STR_TYPE_MASK 7
#define STR_TYPE_BITS 3
#define STR_HDR_VAR(T,s) struct strhdr##T *sh = (void*)((s)-(sizeof(struct strhdr##T)));
#define STR_HDR(T,s) ((struct strhdr##T *)((s)-(sizeof(struct strhdr##T))))
#define STR_TYPE_5_LEN(f) ((f)>>STR_TYPE_BITS)

static inline size_t str_len(const str s) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5:
            return STR_TYPE_5_LEN(flags);
        case STR_TYPE_8:
            return STR_HDR(8,s)->len;
        case STR_TYPE_16:
            return STR_HDR(16,s)->len;
        case STR_TYPE_32:
            return STR_HDR(32,s)->len;
        case STR_TYPE_64:
            return STR_HDR(64,s)->len;
    }
    return 0;
}

static inline size_t str_avail(const str s) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5: {
            return 0;
        }
        case STR_TYPE_8: {
            STR_HDR_VAR(8,s);
            return sh->alloc - sh->len;
        }
        case STR_TYPE_16: {
            STR_HDR_VAR(16,s);
            return sh->alloc - sh->len;
        }
        case STR_TYPE_32: {
            STR_HDR_VAR(32,s);
            return sh->alloc - sh->len;
        }
        case STR_TYPE_64: {
            STR_HDR_VAR(64,s);
            return sh->alloc - sh->len;
        }
    }
    return 0;
}

static inline void str_setlen(str s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5:
            {
                unsigned char *fp = ((unsigned char*)s)-1;
                *fp = STR_TYPE_5 | (newlen << STR_TYPE_BITS);
            }
            break;
        case STR_TYPE_8:
            STR_HDR(8,s)->len = newlen;
            break;
        case STR_TYPE_16:
            STR_HDR(16,s)->len = newlen;
            break;
        case STR_TYPE_32:
            STR_HDR(32,s)->len = newlen;
            break;
        case STR_TYPE_64:
            STR_HDR(64,s)->len = newlen;
            break;
    }
}

static inline void str_inclen(str s, size_t inc) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5:
            {
                unsigned char *fp = ((unsigned char*)s)-1;
                unsigned char newlen = STR_TYPE_5_LEN(flags)+inc;
                *fp = STR_TYPE_5 | (newlen << STR_TYPE_BITS);
            }
            break;
        case STR_TYPE_8:
            STR_HDR(8,s)->len += inc;
            break;
        case STR_TYPE_16:
            STR_HDR(16,s)->len += inc;
            break;
        case STR_TYPE_32:
            STR_HDR(32,s)->len += inc;
            break;
        case STR_TYPE_64:
            STR_HDR(64,s)->len += inc;
            break;
    }
}

/* str_alloc() = str_avail() + str_len() */
static inline size_t str_alloc(const str s) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5:
            return STR_TYPE_5_LEN(flags);
        case STR_TYPE_8:
            return STR_HDR(8,s)->alloc;
        case STR_TYPE_16:
            return STR_HDR(16,s)->alloc;
        case STR_TYPE_32:
            return STR_HDR(32,s)->alloc;
        case STR_TYPE_64:
            return STR_HDR(64,s)->alloc;
    }
    return 0;
}

static inline void str_setalloc(str s, size_t newlen) {
    unsigned char flags = s[-1];
    switch(flags&STR_TYPE_MASK) {
        case STR_TYPE_5:
            /* Nothing to do, this type has no total allocation info. */
            break;
        case STR_TYPE_8:
            STR_HDR(8,s)->alloc = newlen;
            break;
        case STR_TYPE_16:
            STR_HDR(16,s)->alloc = newlen;
            break;
        case STR_TYPE_32:
            STR_HDR(32,s)->alloc = newlen;
            break;
        case STR_TYPE_64:
            STR_HDR(64,s)->alloc = newlen;
            break;
    }
}

str str_newlen(const void *init, size_t initlen);
str str_new(const char *init);
str str_empty(void);
str str_dup(const str s);
void str_free(str s);
str str_growzero(str s, size_t len);
str str_catlen(str s, const void *t, size_t len);
str str_cat(str s, const char *t);
str str_catstr(str s, const str t);
str str_cpylen(str s, const char *t, size_t len);
str str_cpy(str s, const char *t);

str str_catvprintf(str s, const char *fmt, va_list ap);
#ifdef __GNUC__
str str_catprintf(str s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
str strcatprintf(str s, const char *fmt, ...);
#endif

str str_catfmt(str s, char const *fmt, ...);
str str_trim(str s, const char *cset);
void str_range(str s, ssize_t start, ssize_t end);
void str_updatelen(str s);
void str_clear(str s);
int str_cmp(const str s1, const str s2);
str *str_splitlen(const char *s, ssize_t len, const char *sep, int seplen, int *count);
void str_freesplitres(str *tokens, int count);
void str_tolower(str s);
void str_toupper(str s);
str str_fromlonglong(long long value);
str str_catrepr(str s, const char *p, size_t len);
str *str_splitargs(const char *line, int *argc);
str str_mapchars(str s, const char *from, const char *to, size_t setlen);
str str_join(char **argv, int argc, char *sep);
str str_joinstr(str *argv, int argc, const char *sep, size_t seplen);

/* Low level functions exposed to the user API */
str strMakeRoomFor(str s, size_t addlen);
void strIncrLen(str s, ssize_t incr);
str strRemoveFreeSpace(str s);
size_t strAllocSize(str s);
void *strAllocPtr(str s);


#ifdef REDIS_TEST
int strTest(int argc, char *argv[]);
#endif

#endif