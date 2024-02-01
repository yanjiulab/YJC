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

#define STR_MAX_PREALLOC (1024 * 1024)
extern const char* STR_NOINIT;

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// #include <assert.h>
// #include <ctype.h>
// #include <errno.h>
// #include <limits.h>
// #include <stddef.h>
// // #include <stdio.h>
// // #include <stdlib.h>
// // #include <string.h>
// #include <sys/time.h>
// #include <time.h>

typedef char* string_t;

/* STR allocator selection.
 *
 * This file is used in order to change the STR allocator at compile time.
 * Just define the following defines to what you want to use. Also add
 * the include of your alternate allocator if needed (not needed in order
 * to use the default libc allocator). */

#define s_malloc  malloc
#define s_realloc realloc
#define s_free    free

/* Note: strhdr5 is never used, we just access the flags byte directly.
 * However is here to document the layout of type 5 string_t strings. */
struct __attribute__((__packed__)) strhdr5 {
    unsigned char flags; /* 3 lsb of type, and 5 msb of string length */
    char buf[];
};
struct __attribute__((__packed__)) strhdr8 {
    uint8_t len;         /* used */
    uint8_t alloc;       /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__((__packed__)) strhdr16 {
    uint16_t len;        /* used */
    uint16_t alloc;      /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__((__packed__)) strhdr32 {
    uint32_t len;        /* used */
    uint32_t alloc;      /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};
struct __attribute__((__packed__)) strhdr64 {
    uint64_t len;        /* used */
    uint64_t alloc;      /* excluding the header and null terminator */
    unsigned char flags; /* 3 lsb of type, 5 unused bits */
    char buf[];
};

#define STR_TYPE_5        0
#define STR_TYPE_8        1
#define STR_TYPE_16       2
#define STR_TYPE_32       3
#define STR_TYPE_64       4
#define STR_TYPE_MASK     7
#define STR_TYPE_BITS     3
#define STR_HDR_VAR(T, s) struct strhdr##T* sh = (void*)((s) - (sizeof(struct strhdr##T)));
#define STR_HDR(T, s)     ((struct strhdr##T*)((s) - (sizeof(struct strhdr##T))))
#define STR_TYPE_5_LEN(f) ((f) >> STR_TYPE_BITS)

static inline size_t str_len(const string_t s) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5:
        return STR_TYPE_5_LEN(flags);
    case STR_TYPE_8:
        return STR_HDR(8, s)->len;
    case STR_TYPE_16:
        return STR_HDR(16, s)->len;
    case STR_TYPE_32:
        return STR_HDR(32, s)->len;
    case STR_TYPE_64:
        return STR_HDR(64, s)->len;
    }
    return 0;
}

static inline size_t str_avail(const string_t s) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5: {
        return 0;
    }
    case STR_TYPE_8: {
        STR_HDR_VAR(8, s);
        return sh->alloc - sh->len;
    }
    case STR_TYPE_16: {
        STR_HDR_VAR(16, s);
        return sh->alloc - sh->len;
    }
    case STR_TYPE_32: {
        STR_HDR_VAR(32, s);
        return sh->alloc - sh->len;
    }
    case STR_TYPE_64: {
        STR_HDR_VAR(64, s);
        return sh->alloc - sh->len;
    }
    }
    return 0;
}

static inline void str_setlen(string_t s, size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5: {
        unsigned char* fp = ((unsigned char*)s) - 1;
        *fp = STR_TYPE_5 | (newlen << STR_TYPE_BITS);
    } break;
    case STR_TYPE_8:
        STR_HDR(8, s)->len = newlen;
        break;
    case STR_TYPE_16:
        STR_HDR(16, s)->len = newlen;
        break;
    case STR_TYPE_32:
        STR_HDR(32, s)->len = newlen;
        break;
    case STR_TYPE_64:
        STR_HDR(64, s)->len = newlen;
        break;
    }
}

static inline void str_inclen(string_t s, size_t inc) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5: {
        unsigned char* fp = ((unsigned char*)s) - 1;
        unsigned char newlen = STR_TYPE_5_LEN(flags) + inc;
        *fp = STR_TYPE_5 | (newlen << STR_TYPE_BITS);
    } break;
    case STR_TYPE_8:
        STR_HDR(8, s)->len += inc;
        break;
    case STR_TYPE_16:
        STR_HDR(16, s)->len += inc;
        break;
    case STR_TYPE_32:
        STR_HDR(32, s)->len += inc;
        break;
    case STR_TYPE_64:
        STR_HDR(64, s)->len += inc;
        break;
    }
}

/* str_alloc() = str_avail() + str_len() */
static inline size_t str_alloc(const string_t s) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5:
        return STR_TYPE_5_LEN(flags);
    case STR_TYPE_8:
        return STR_HDR(8, s)->alloc;
    case STR_TYPE_16:
        return STR_HDR(16, s)->alloc;
    case STR_TYPE_32:
        return STR_HDR(32, s)->alloc;
    case STR_TYPE_64:
        return STR_HDR(64, s)->alloc;
    }
    return 0;
}

static inline void str_setalloc(string_t s, size_t newlen) {
    unsigned char flags = s[-1];
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5:
        /* Nothing to do, this type has no total allocation info. */
        break;
    case STR_TYPE_8:
        STR_HDR(8, s)->alloc = newlen;
        break;
    case STR_TYPE_16:
        STR_HDR(16, s)->alloc = newlen;
        break;
    case STR_TYPE_32:
        STR_HDR(32, s)->alloc = newlen;
        break;
    case STR_TYPE_64:
        STR_HDR(64, s)->alloc = newlen;
        break;
    }
}

string_t str_newlen(const void* init, size_t initlen);
string_t str_new(const char* init);
string_t str_empty(void);
string_t str_fmt(const char* fmt, ...);
string_t str_dup(const string_t s);
void str_free(string_t s);
string_t str_growzero(string_t s, size_t len);
string_t str_catlen(string_t s, const void* t, size_t len);
string_t str_cat(string_t s, const char* t);
string_t str_catstr(string_t s, const string_t t);
string_t str_cpylen(string_t s, const char* t, size_t len);
string_t str_cpy(string_t s, const char* t);

string_t str_catvprintf(string_t s, const char* fmt, va_list ap);
#ifdef __GNUC__
string_t str_catprintf(string_t s, const char* fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
string_t str_catprintf(string_t s, const char* fmt, ...);
#endif

string_t str_catfmt(string_t s, char const* fmt, ...);
string_t str_trim(string_t s, const char* cset);
void str_range(string_t s, ssize_t start, ssize_t end);
void str_updatelen(string_t s);
void str_clear(string_t s);
int str_cmp(const string_t s1, const string_t s2);
string_t* str_splitlen(const char* s, ssize_t len, const char* sep, int seplen, int* count);
void str_freesplitres(string_t* tokens, int count);
void str_tolower(string_t s);
void str_toupper(string_t s);
string_t str_fromlonglong(long long value);
string_t str_catrepr(string_t s, const char* p, size_t len);
string_t* str_splitargs(const char* line, int* argc);
string_t str_mapchars(string_t s, const char* from, const char* to, size_t setlen);
string_t str_join(char** argv, int argc, char* sep);
string_t str_joinstr(string_t* argv, int argc, const char* sep, size_t seplen);

/* Low level functions exposed to the user API */
string_t strMakeRoomFor(string_t s, size_t addlen);
void strIncrLen(string_t s, ssize_t incr);
string_t strRemoveFreeSpace(string_t s);
size_t strAllocSize(string_t s);
void* strAllocPtr(string_t s);

/* Original char * API */
#define strmatch(s1, s2) (strncmp((s1), (s2), strlen((s2))) == 0)
#define strlcpy          strncpy
#define strlcat          strncat

bool str_startswith(const char* str, const char* prefix);
bool str_endswith(const char* str, const char* suffix);
bool str_all_digit(const char* str);
bool str_contains(const char* str, const char* sub);

// Prints to an automatically-allocated string. Returns NULL if an encoding error occurs or if sufficient memory cannot
// be allocated.

// Attempts to parse a string as an integer value, exiting on failure.
int str2int(const char* string);
// Attempts to parse a string as a double value, exiting on failure.
double str2double(const char* string);
// 1 y on yes true enable => true
bool str2bool(const char* str);
// 1T2G3M4K5B => ?B
size_t str2size(const char* str);
// 1w2d3h4m5s => ?s
time_t str2time(const char* str);
// Hashes a string using the FNV-1a algorithm.
uint32_t str_hash(const char* string);

/*-------------------------- new string operations ---------------------------------*/
// Use REPLACE string to replace the FIND string in STR.
char* str_replace(const char* str, const char* find, const char* replace);
/*-------------------------- in place modified operations ---------------------------------*/
// Converts a string to uppercase in place.
char* str_upper(char* str);
// Converts a string to lowercase in place.
char* str_lower(char* str);
// Reverses the string in place.
char* str_reverse(char* str);
char* str_hex(char* buff, size_t bufsiz, const uint8_t* str, size_t num);
// Generates a random string of len in heap if buf is NULL, otherwise in buf.
char* str_random(char* buf, int len);

#ifdef REDIS_TEST
int strTest(int argc, char* argv[]);
#endif

#endif