/* STRLib 2.0 -- A C dynamic strings library
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

#include "str_new.h"
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* STR_NOINIT = "STR_NOINIT";

static inline int strHdrSize(char type) {
    switch (type & STR_TYPE_MASK) {
    case STR_TYPE_5:
        return sizeof(struct strhdr5);
    case STR_TYPE_8:
        return sizeof(struct strhdr8);
    case STR_TYPE_16:
        return sizeof(struct strhdr16);
    case STR_TYPE_32:
        return sizeof(struct strhdr32);
    case STR_TYPE_64:
        return sizeof(struct strhdr64);
    }
    return 0;
}

static inline char strReqType(size_t string_size) {
    if (string_size < 1 << 5)
        return STR_TYPE_5;
    if (string_size < 1 << 8)
        return STR_TYPE_8;
    if (string_size < 1 << 16)
        return STR_TYPE_16;
#if (LONG_MAX == LLONG_MAX)
    if (string_size < 1ll << 32)
        return STR_TYPE_32;
    return STR_TYPE_64;
#else
    return STR_TYPE_32;
#endif
}

/* Create a new str string with the content specified by the 'init' pointer
 * and 'initlen'.
 * If NULL is used for 'init' the string is initialized with zero bytes.
 * If STR_NOINIT is used, the buffer is left uninitialized;
 *
 * The string is always null-terminated (all the str strings are, always) so
 * even if you create an str string with:
 *
 * mystring = str_newlen("abc",3);
 *
 * You can print the string with printf() as there is an implicit \0 at the
 * end of the string. However the string is binary safe and can contain
 * \0 characters in the middle, as the length is stored in the str header. */
str str_newlen(const void* init, size_t initlen) {
    void* sh;
    str s;
    char type = strReqType(initlen);
    /* Empty strings are usually created in order to append. Use type 8
     * since type 5 is not good at this. */
    if (type == STR_TYPE_5 && initlen == 0)
        type = STR_TYPE_8;
    int hdrlen = strHdrSize(type);
    unsigned char* fp; /* flags pointer. */

    sh = s_malloc(hdrlen + initlen + 1);
    if (sh == NULL)
        return NULL;
    if (init == STR_NOINIT)
        init = NULL;
    else if (!init)
        memset(sh, 0, hdrlen + initlen + 1);
    s = (char*)sh + hdrlen;
    fp = ((unsigned char*)s) - 1;
    switch (type) {
    case STR_TYPE_5: {
        *fp = type | (initlen << STR_TYPE_BITS);
        break;
    }
    case STR_TYPE_8: {
        STR_HDR_VAR(8, s);
        sh->len = initlen;
        sh->alloc = initlen;
        *fp = type;
        break;
    }
    case STR_TYPE_16: {
        STR_HDR_VAR(16, s);
        sh->len = initlen;
        sh->alloc = initlen;
        *fp = type;
        break;
    }
    case STR_TYPE_32: {
        STR_HDR_VAR(32, s);
        sh->len = initlen;
        sh->alloc = initlen;
        *fp = type;
        break;
    }
    case STR_TYPE_64: {
        STR_HDR_VAR(64, s);
        sh->len = initlen;
        sh->alloc = initlen;
        *fp = type;
        break;
    }
    }
    if (initlen && init)
        memcpy(s, init, initlen);
    s[initlen] = '\0';
    return s;
}

/* Create an empty (zero length) str string. Even in this case the string
 * always has an implicit null term. */
str str_empty(void) {
    return str_newlen("", 0);
}

/* Create a new str string starting from a null terminated C string. */
str str_new(const char* init) {
    size_t initlen = (init == NULL) ? 0 : strlen(init);
    return str_newlen(init, initlen);
}

/* Duplicate an str string. */
str str_dup(const str s) {
    return str_newlen(s, str_len(s));
}

/* Free an str string. No operation is performed if 's' is NULL. */
void str_free(str s) {
    if (s == NULL)
        return;
    s_free((char*)s - strHdrSize(s[-1]));
}

/* Set the str string length to the length as obtained with strlen(), so
 * considering as content only up to the first null term character.
 *
 * This function is useful when the str string is hacked manually in some
 * way, like in the following example:
 *
 * s = str_new("foobar");
 * s[2] = '\0';
 * str_updatelen(s);
 * printf("%d\n", str_len(s));
 *
 * The output will be "2", but if we comment out the call to str_updatelen()
 * the output will be "6" as the string was modified but the logical length
 * remains 6 bytes. */
void str_updatelen(str s) {
    size_t reallen = strlen(s);
    str_setlen(s, reallen);
}

/* Modify an str string in-place to make it empty (zero length).
 * However all the existing buffer is not discarded but set as free space
 * so that next append operations will not require allocations up to the
 * number of bytes previously available. */
void str_clear(str s) {
    str_setlen(s, 0);
    s[0] = '\0';
}

/* Enlarge the free space at the end of the str string so that the caller
 * is sure that after calling this function can overwrite up to addlen
 * bytes after the end of the string, plus one more byte for nul term.
 *
 * Note: this does not change the *length* of the str string as returned
 * by str_len(), but only the free buffer space we have. */
str strMakeRoomFor(str s, size_t addlen) {
    void *sh, *newsh;
    size_t avail = str_avail(s);
    size_t len, newlen, reqlen;
    char type, oldtype = s[-1] & STR_TYPE_MASK;
    int hdrlen;

    /* Return ASAP if there is enough space left. */
    if (avail >= addlen)
        return s;

    len = str_len(s);
    sh = (char*)s - strHdrSize(oldtype);
    reqlen = newlen = (len + addlen);
    if (newlen < STR_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += STR_MAX_PREALLOC;

    type = strReqType(newlen);

    /* Don't use type 5: the user is appending to the string and type 5 is
     * not able to remember empty space, so strMakeRoomFor() must be called
     * at every appending operation. */
    if (type == STR_TYPE_5)
        type = STR_TYPE_8;

    hdrlen = strHdrSize(type);
    assert(hdrlen + newlen + 1 > reqlen); /* Catch size_t overflow */
    if (oldtype == type) {
        newsh = s_realloc(sh, hdrlen + newlen + 1);
        if (newsh == NULL)
            return NULL;
        s = (char*)newsh + hdrlen;
    } else {
        /* Since the header size changes, need to move the string forward,
         * and can't use realloc */
        newsh = s_malloc(hdrlen + newlen + 1);
        if (newsh == NULL)
            return NULL;
        memcpy((char*)newsh + hdrlen, s, len + 1);
        s_free(sh);
        s = (char*)newsh + hdrlen;
        s[-1] = type;
        str_setlen(s, len);
    }
    str_setalloc(s, newlen);
    return s;
}

/* Reallocate the str string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation.
 *
 * After the call, the passed str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
str strRemoveFreeSpace(str s) {
    void *sh, *newsh;
    char type, oldtype = s[-1] & STR_TYPE_MASK;
    int hdrlen, oldhdrlen = strHdrSize(oldtype);
    size_t len = str_len(s);
    size_t avail = str_avail(s);
    sh = (char*)s - oldhdrlen;

    /* Return ASAP if there is no space left. */
    if (avail == 0)
        return s;

    /* Check what would be the minimum STR header that is just good enough to
     * fit this string. */
    type = strReqType(len);
    hdrlen = strHdrSize(type);

    /* If the type is the same, or at least a large enough type is still
     * required, we just realloc(), letting the allocator to do the copy
     * only if really needed. Otherwise if the change is huge, we manually
     * reallocate the string to use the different header type. */
    if (oldtype == type || type > STR_TYPE_8) {
        newsh = s_realloc(sh, oldhdrlen + len + 1);
        if (newsh == NULL)
            return NULL;
        s = (char*)newsh + oldhdrlen;
    } else {
        newsh = s_malloc(hdrlen + len + 1);
        if (newsh == NULL)
            return NULL;
        memcpy((char*)newsh + hdrlen, s, len + 1);
        s_free(sh);
        s = (char*)newsh + hdrlen;
        s[-1] = type;
        str_setlen(s, len);
    }
    str_setalloc(s, len);
    return s;
}

/* Return the total size of the allocation of the specified str string,
 * including:
 * 1) The str header before the pointer.
 * 2) The string.
 * 3) The free buffer at the end if any.
 * 4) The implicit null term.
 */
size_t str_allocSize(str s) {
    size_t alloc = str_alloc(s);
    return strHdrSize(s[-1]) + alloc + 1;
}

/* Return the pointer of the actual STR allocation (normally STR strings
 * are referenced by the start of the string buffer). */
void* str_allocPtr(str s) {
    return (void*)(s - strHdrSize(s[-1]));
}

/* Increment the str length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls strMakeRoomFor(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Usage example:
 *
 * Using strIncrLen() and strMakeRoomFor() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * str string without copying into an intermediate buffer:
 *
 * oldlen = str_len(s);
 * s = strMakeRoomFor(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * strIncrLen(s, nread);
 */
void strIncrLen(str s, ssize_t incr) {
    unsigned char flags = s[-1];
    size_t len;
    switch (flags & STR_TYPE_MASK) {
    case STR_TYPE_5: {
        unsigned char* fp = ((unsigned char*)s) - 1;
        unsigned char oldlen = STR_TYPE_5_LEN(flags);
        assert((incr > 0 && oldlen + incr < 32) || (incr < 0 && oldlen >= (unsigned int)(-incr)));
        *fp = STR_TYPE_5 | ((oldlen + incr) << STR_TYPE_BITS);
        len = oldlen + incr;
        break;
    }
    case STR_TYPE_8: {
        STR_HDR_VAR(8, s);
        assert((incr >= 0 && sh->alloc - sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
        len = (sh->len += incr);
        break;
    }
    case STR_TYPE_16: {
        STR_HDR_VAR(16, s);
        assert((incr >= 0 && sh->alloc - sh->len >= incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
        len = (sh->len += incr);
        break;
    }
    case STR_TYPE_32: {
        STR_HDR_VAR(32, s);
        assert((incr >= 0 && sh->alloc - sh->len >= (unsigned int)incr) || (incr < 0 && sh->len >= (unsigned int)(-incr)));
        len = (sh->len += incr);
        break;
    }
    case STR_TYPE_64: {
        STR_HDR_VAR(64, s);
        assert((incr >= 0 && sh->alloc - sh->len >= (uint64_t)incr) || (incr < 0 && sh->len >= (uint64_t)(-incr)));
        len = (sh->len += incr);
        break;
    }
    default:
        len = 0; /* Just to avoid compilation warnings. */
    }
    s[len] = '\0';
}

/* Grow the str to have the specified length. Bytes that were not part of
 * the original length of the str will be set to zero.
 *
 * if the specified length is smaller than the current length, no operation
 * is performed. */
str strgrowzero(str s, size_t len) {
    size_t curlen = str_len(s);

    if (len <= curlen)
        return s;
    s = strMakeRoomFor(s, len - curlen);
    if (s == NULL)
        return NULL;

    /* Make sure added region doesn't contain garbage */
    memset(s + curlen, 0, (len - curlen + 1)); /* also set trailing \0 byte */
    str_setlen(s, len);
    return s;
}

/* Append the specified binary-safe string pointed by 't' of 'len' bytes to the
 * end of the specified str string 's'.
 *
 * After the call, the passed str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
str str_catlen(str s, const void* t, size_t len) {
    size_t curlen = str_len(s);

    s = strMakeRoomFor(s, len);
    if (s == NULL)
        return NULL;
    memcpy(s + curlen, t, len);
    str_setlen(s, curlen + len);
    s[curlen + len] = '\0';
    return s;
}

/* Append the specified null termianted C string to the str string 's'.
 *
 * After the call, the passed str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
str str_cat(str s, const char* t) {
    return str_catlen(s, t, strlen(t));
}

/* Append the specified str 't' to the existing str 's'.
 *
 * After the call, the modified str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
str str_catstr(str s, const str t) {
    return str_catlen(s, t, str_len(t));
}

/* Destructively modify the str string 's' to hold the specified binary
 * safe string pointed by 't' of length 'len' bytes. */
str str_cpylen(str s, const char* t, size_t len) {
    if (str_alloc(s) < len) {
        s = strMakeRoomFor(s, len - str_len(s));
        if (s == NULL)
            return NULL;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    str_setlen(s, len);
    return s;
}

/* Like str_cpylen() but 't' must be a null-terminated string so that the length
 * of the string is obtained with strlen(). */
str str_cpy(str s, const char* t) {
    return str_cpylen(s, t, strlen(t));
}

/* Helper for str_catlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * STR_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define STR_LLSTR_SIZE 21
int strll2str(char* s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    if (value < 0) {
        /* Since v is unsigned, if value==LLONG_MIN then
         * -LLONG_MIN will overflow. */
        if (value != LLONG_MIN) {
            v = -value;
        } else {
            v = ((unsigned long long)LLONG_MAX) + 1;
        }
    } else {
        v = value;
    }

    p = s;
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);
    if (value < 0)
        *p++ = '-';

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical strll2str(), but for unsigned long long type. */
int strull2str(char* s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0' + (v % 10);
        v /= 10;
    } while (v);

    /* Compute length and add null term. */
    l = p - s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while (s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Create an str string from a long long value. It is much faster than:
 *
 * str_catprintf(str_empty(),"%lld\n", value);
 */
str str_fromlonglong(long long value) {
    char buf[STR_LLSTR_SIZE];
    int len = strll2str(buf, value);

    return str_newlen(buf, len);
}

/* Like str_catprintf() but gets va_list instead of being variadic. */
str str_catvprintf(str s, const char* fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    size_t buflen = strlen(fmt) * 2;
    int bufstr_len;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = s_malloc(buflen);
        if (buf == NULL)
            return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Alloc enough space for buffer and \0 after failing to
     * fit the string in the current buffer size. */
    while (1) {
        va_copy(cpy, ap);
        bufstr_len = vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (bufstr_len < 0) {
            if (buf != staticbuf)
                s_free(buf);
            return NULL;
        }
        if (((size_t)bufstr_len) >= buflen) {
            if (buf != staticbuf)
                s_free(buf);
            buflen = ((size_t)bufstr_len) + 1;
            buf = s_malloc(buflen);
            if (buf == NULL)
                return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the STR string and return it. */
    t = str_catlen(s, buf, bufstr_len);
    if (buf != staticbuf)
        s_free(buf);
    return t;
}

/* Append to the str string 's' a string obtained using printf-alike format
 * specifier.
 *
 * After the call, the modified str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = str_new("Sum is: ");
 * s = str_catprintf(s,"%d+%d = %d",a,b,a+b).
 *
 * Often you need to create a string from scratch with the printf-alike
 * format. When this is the need, just use str_empty() as the target string:
 *
 * s = str_catprintf(str_empty(), "... your format ...", args);
 */
str str_catprintf(str s, const char* fmt, ...) {
    va_list ap;
    char* t;
    va_start(ap, fmt);
    t = str_catvprintf(s, fmt, ap);
    va_end(ap);
    return t;
}

/* This function is similar to str_catprintf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the str string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %S - STR string
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
str str_catfmt(str s, char const* fmt, ...) {
    size_t initlen = str_len(s);
    const char* f = fmt;
    long i;
    va_list ap;

    /* To avoid continuous reallocations, let's start with a buffer that
     * can hold at least two times the format string itself. It's not the
     * best heuristic but seems to work in practice. */
    s = strMakeRoomFor(s, initlen + strlen(fmt) * 2);
    va_start(ap, fmt);
    f = fmt;     /* Next format specifier byte to process. */
    i = initlen; /* Position of the next byte to write to dest str. */
    while (*f) {
        char next, *str;
        size_t l;
        long long num;
        unsigned long long unum;

        /* Make sure there is always space for at least 1 char. */
        if (str_avail(s) == 0) {
            s = strMakeRoomFor(s, 1);
        }

        switch (*f) {
        case '%':
            next = *(f + 1);
            if (next == '\0')
                break;
            f++;
            switch (next) {
            case 's':
            case 'S':
                str = va_arg(ap, char*);
                l = (next == 's') ? strlen(str) : str_len(str);
                if (str_avail(s) < l) {
                    s = strMakeRoomFor(s, l);
                }
                memcpy(s + i, str, l);
                str_inclen(s, l);
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap, int);
                else
                    num = va_arg(ap, long long);
                {
                    char buf[STR_LLSTR_SIZE];
                    l = strll2str(buf, num);
                    if (str_avail(s) < l) {
                        s = strMakeRoomFor(s, l);
                    }
                    memcpy(s + i, buf, l);
                    str_inclen(s, l);
                    i += l;
                }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap, unsigned int);
                else
                    unum = va_arg(ap, unsigned long long);
                {
                    char buf[STR_LLSTR_SIZE];
                    l = strull2str(buf, unum);
                    if (str_avail(s) < l) {
                        s = strMakeRoomFor(s, l);
                    }
                    memcpy(s + i, buf, l);
                    str_inclen(s, l);
                    i += l;
                }
                break;
            default: /* Handle %% and generally %<unknown>. */
                s[i++] = next;
                str_inclen(s, 1);
                break;
            }
            break;
        default:
            s[i++] = *f;
            str_inclen(s, 1);
            break;
        }
        f++;
    }
    va_end(ap);

    /* Add null-term */
    s[i] = '\0';
    return s;
}

/* Remove the part of the string from left and from right composed just of
 * contiguous characters found in 'cset', that is a null terminated C string.
 *
 * After the call, the modified str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = str_new("AA...AA.a.aa.aHelloWorld     :::");
 * s = str_trim(s,"Aa. :");
 * printf("%s\n", s);
 *
 * Output will be just "HelloWorld".
 */
str str_trim(str s, const char* cset) {
    char *end, *sp, *ep;
    size_t len;

    sp = s;
    ep = end = s + str_len(s) - 1;
    while (sp <= end && strchr(cset, *sp))
        sp++;
    while (ep > sp && strchr(cset, *ep))
        ep--;
    len = (ep - sp) + 1;
    if (s != sp)
        memmove(s, sp, len);
    s[len] = '\0';
    str_setlen(s, len);
    return s;
}

/* Turn the string into a smaller (or equal) string containing only the
 * substring specified by the 'start' and 'end' indexes.
 *
 * start and end can be negative, where -1 means the last character of the
 * string, -2 the penultimate character, and so forth.
 *
 * The interval is inclusive, so the start and end characters will be part
 * of the resulting string.
 *
 * The string is modified in-place.
 *
 * Example:
 *
 * s = str_new("Hello World");
 * str_range(s,1,-1); => "ello World"
 */
void str_range(str s, ssize_t start, ssize_t end) {
    size_t newlen, len = str_len(s);

    if (len == 0)
        return;
    if (start < 0) {
        start = len + start;
        if (start < 0)
            start = 0;
    }
    if (end < 0) {
        end = len + end;
        if (end < 0)
            end = 0;
    }
    newlen = (start > end) ? 0 : (end - start) + 1;
    if (newlen != 0) {
        if (start >= (ssize_t)len) {
            newlen = 0;
        } else if (end >= (ssize_t)len) {
            end = len - 1;
            newlen = (end - start) + 1;
        }
    }
    if (start && newlen)
        memmove(s, s + start, newlen);
    s[newlen] = 0;
    str_setlen(s, newlen);
}

/* Apply tolower() to every character of the str string 's'. */
void str_tolower(str s) {
    size_t len = str_len(s), j;

    for (j = 0; j < len; j++)
        s[j] = tolower(s[j]);
}

/* Apply toupper() to every character of the str string 's'. */
void str_toupper(str s) {
    size_t len = str_len(s), j;

    for (j = 0; j < len; j++)
        s[j] = toupper(s[j]);
}

/* Compare two str strings s1 and s2 with memcmp().
 *
 * Return value:
 *
 *     positive if s1 > s2.
 *     negative if s1 < s2.
 *     0 if s1 and s2 are exactly the same binary string.
 *
 * If two strings share exactly the same prefix, but one of the two has
 * additional characters, the longer string is considered to be greater than
 * the smaller one. */
int str_cmp(const str s1, const str s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = str_len(s1);
    l2 = str_len(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1, s2, minlen);
    if (cmp == 0)
        return l1 > l2 ? 1 : (l1 < l2 ? -1 : 0);
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of str strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * str_split("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. str_split() is just the
 * same function but for zero-terminated strings.
 */
str* str_splitlen(const char* s, ssize_t len, const char* sep, int seplen, int* count) {
    int elements = 0, slots = 5;
    long start = 0, j;
    str* tokens;

    if (seplen < 1 || len <= 0) {
        *count = 0;
        return NULL;
    }

    tokens = s_malloc(sizeof(str) * slots);
    if (tokens == NULL)
        return NULL;

    for (j = 0; j < (len - (seplen - 1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements + 2) {
            str* newtokens;

            slots *= 2;
            newtokens = s_realloc(tokens, sizeof(str) * slots);
            if (newtokens == NULL)
                goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s + j) == sep[0]) || (memcmp(s + j, sep, seplen) == 0)) {
            tokens[elements] = str_newlen(s + start, j - start);
            if (tokens[elements] == NULL)
                goto cleanup;
            elements++;
            start = j + seplen;
            j = j + seplen - 1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = str_newlen(s + start, len - start);
    if (tokens[elements] == NULL)
        goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup: {
    int i;
    for (i = 0; i < elements; i++)
        str_free(tokens[i]);
    s_free(tokens);
    *count = 0;
    return NULL;
}
}

/* Free the result returned by str_splitlen(), or do nothing if 'tokens' is NULL. */
void str_freesplitres(str* tokens, int count) {
    if (!tokens)
        return;
    while (count--)
        str_free(tokens[count]);
    s_free(tokens);
}

/* Append to the str string "s" an escaped string representation where
 * all the non-printable characters (tested with isprint()) are turned into
 * escapes in the form "\n\r\a...." or "\x<hex-number>".
 *
 * After the call, the modified str string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
str str_catrepr(str s, const char* p, size_t len) {
    s = str_catlen(s, "\"", 1);
    while (len--) {
        switch (*p) {
        case '\\':
        case '"':
            s = str_catprintf(s, "\\%c", *p);
            break;
        case '\n':
            s = str_catlen(s, "\\n", 2);
            break;
        case '\r':
            s = str_catlen(s, "\\r", 2);
            break;
        case '\t':
            s = str_catlen(s, "\\t", 2);
            break;
        case '\a':
            s = str_catlen(s, "\\a", 2);
            break;
        case '\b':
            s = str_catlen(s, "\\b", 2);
            break;
        default:
            if (isprint(*p))
                s = str_catprintf(s, "%c", *p);
            else
                s = str_catprintf(s, "\\x%02x", (unsigned char)*p);
            break;
        }
        p++;
    }
    return str_catlen(s, "\"", 1);
}

/* Helper function for str_splitargs() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for str_splitargs() that converts a hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

/* Split a line into arguments, where every argument can be in the
 * following programming-language REPL-alike form:
 *
 * foo bar "newline are supported\n" and "\xff\x00otherstuff"
 *
 * The number of arguments is stored into *argc, and an array
 * of str is returned.
 *
 * The caller should free the resulting array of str strings with
 * str_freesplitres().
 *
 * Note that str_catrepr() is able to convert back a string into
 * a quoted string in the same format str_splitargs() is able to parse.
 *
 * The function returns the allocated tokens on success, even when the
 * input string is empty, or NULL if the input contains unbalanced
 * quotes or closed quotes followed by non space characters
 * as in: "foo"bar or "foo'
 */
str* str_splitargs(const char* line, int* argc) {
    const char* p = line;
    char* current = NULL;
    char** vector = NULL;

    *argc = 0;
    while (1) {
        /* skip blanks */
        while (*p && isspace(*p))
            p++;
        if (*p) {
            /* get a token */
            int inq = 0;  /* set to 1 if we are in "quotes" */
            int insq = 0; /* set to 1 if we are in 'single quotes' */
            int done = 0;

            if (current == NULL)
                current = str_empty();
            while (!done) {
                if (inq) {
                    if (*p == '\\' && *(p + 1) == 'x' &&
                        is_hex_digit(*(p + 2)) &&
                        is_hex_digit(*(p + 3))) {
                        unsigned char byte;

                        byte = (hex_digit_to_int(*(p + 2)) * 16) +
                               hex_digit_to_int(*(p + 3));
                        current = str_catlen(current, (char*)&byte, 1);
                        p += 3;
                    } else if (*p == '\\' && *(p + 1)) {
                        char c;

                        p++;
                        switch (*p) {
                        case 'n':
                            c = '\n';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'a':
                            c = '\a';
                            break;
                        default:
                            c = *p;
                            break;
                        }
                        current = str_catlen(current, &c, 1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p + 1) && !isspace(*(p + 1)))
                            goto err;
                        done = 1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = str_catlen(current, p, 1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p + 1) == '\'') {
                        p++;
                        current = str_catlen(current, "'", 1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p + 1) && !isspace(*(p + 1)))
                            goto err;
                        done = 1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = str_catlen(current, p, 1);
                    }
                } else {
                    switch (*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done = 1;
                        break;
                    case '"':
                        inq = 1;
                        break;
                    case '\'':
                        insq = 1;
                        break;
                    default:
                        current = str_catlen(current, p, 1);
                        break;
                    }
                }
                if (*p)
                    p++;
            }
            /* add the token to the vector */
            vector = s_realloc(vector, ((*argc) + 1) * sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL)
                vector = s_malloc(sizeof(void*));
            return vector;
        }
    }

err:
    while ((*argc)--)
        str_free(vector[*argc]);
    s_free(vector);
    if (current)
        str_free(current);
    *argc = 0;
    return NULL;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: str_mapchars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the str string pointer, that is always the same
 * as the input pointer since no resize is needed. */
str str_mapchars(str s, const char* from, const char* to, size_t setlen) {
    size_t j, i, l = str_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an str string. */
str str_join(char** argv, int argc, char* sep) {
    str join = str_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = str_cat(join, argv[j]);
        if (j != argc - 1)
            join = str_cat(join, sep);
    }
    return join;
}

/* Like str_join, but joins an array of STR strings. */
str str_joinstr(str* argv, int argc, const char* sep, size_t seplen) {
    str join = str_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = str_catstr(join, argv[j]);
        if (j != argc - 1)
            join = str_catlen(join, sep, seplen);
    }
    return join;
}

#if defined(STR_TEST_MAIN)
#include "limits.h"
#include "testhelp.h"
#include <stdio.h>

#define UNUSED(x) (void)(x)
int strTest(void) {
    {
        str x = str_new("foo"), y;

        test_cond("Create a string and obtain the length",
                  str_len(x) == 3 && memcmp(x, "foo\0", 4) == 0)

            str_free(x);
        x = str_newlen("foo", 2);
        test_cond("Create a string with specified length",
                  str_len(x) == 2 && memcmp(x, "fo\0", 3) == 0)

            x = str_cat(x, "bar");
        test_cond("Strings concatenation",
                  str_len(x) == 5 && memcmp(x, "fobar\0", 6) == 0);

        x = str_cpy(x, "a");
        test_cond("str_cpy() against an originally longer string",
                  str_len(x) == 1 && memcmp(x, "a\0", 2) == 0)

            x = str_cpy(x, "xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
        test_cond("str_cpy() against an originally shorter string",
                  str_len(x) == 33 &&
                      memcmp(x, "xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0", 33) == 0)

            str_free(x);
        x = str_catprintf(str_empty(), "%d", 123);
        test_cond("str_catprintf() seems working in the base case",
                  str_len(x) == 3 && memcmp(x, "123\0", 4) == 0)

            str_free(x);
        x = str_catprintf(str_empty(), "a%cb", 0);
        test_cond("str_catprintf() seems working with \\0 inside of result",
                  str_len(x) == 3 && memcmp(x, "a\0"
                                               "b\0",
                                            4) == 0)

        {
            str_free(x);
            char etalon[1024 * 1024];
            for (size_t i = 0; i < sizeof(etalon); i++) {
                etalon[i] = '0';
            }
            x = str_catprintf(str_empty(), "%0*d", (int)sizeof(etalon), 0);
            test_cond("str_catprintf() can print 1MB",
                      str_len(x) == sizeof(etalon) && memcmp(x, etalon, sizeof(etalon)) == 0)
        }

        str_free(x);
        x = str_new("--");
        x = str_catfmt(x, "Hello %s World %I,%I--", "Hi!", LLONG_MIN, LLONG_MAX);
        test_cond("str_catfmt() seems working in the base case",
                  str_len(x) == 60 &&
                      memcmp(x, "--Hello Hi! World -9223372036854775808,"
                                "9223372036854775807--",
                             60) == 0)
            printf("[%s]\n", x);

        str_free(x);
        x = str_new("--");
        x = str_catfmt(x, "%u,%U--", UINT_MAX, ULLONG_MAX);
        test_cond("str_catfmt() seems working with unsigned numbers",
                  str_len(x) == 35 &&
                      memcmp(x, "--4294967295,18446744073709551615--", 35) == 0)

            str_free(x);
        x = str_new(" x ");
        str_trim(x, " x");
        test_cond("str_trim() works when all chars match",
                  str_len(x) == 0)

            str_free(x);
        x = str_new(" x ");
        str_trim(x, " ");
        test_cond("str_trim() works when a single char remains",
                  str_len(x) == 1 && x[0] == 'x')

            str_free(x);
        x = str_new("xxciaoyyy");
        str_trim(x, "xy");
        test_cond("str_trim() correctly trims characters",
                  str_len(x) == 4 && memcmp(x, "ciao\0", 5) == 0)

            y = str_dup(x);
        str_range(y, 1, 1);
        test_cond("str_range(...,1,1)",
                  str_len(y) == 1 && memcmp(y, "i\0", 2) == 0)

            str_free(y);
        y = str_dup(x);
        str_range(y, 1, -1);
        test_cond("str_range(...,1,-1)",
                  str_len(y) == 3 && memcmp(y, "iao\0", 4) == 0)

            str_free(y);
        y = str_dup(x);
        str_range(y, -2, -1);
        test_cond("str_range(...,-2,-1)",
                  str_len(y) == 2 && memcmp(y, "ao\0", 3) == 0)

            str_free(y);
        y = str_dup(x);
        str_range(y, 2, 1);
        test_cond("str_range(...,2,1)",
                  str_len(y) == 0 && memcmp(y, "\0", 1) == 0)

            str_free(y);
        y = str_dup(x);
        str_range(y, 1, 100);
        test_cond("str_range(...,1,100)",
                  str_len(y) == 3 && memcmp(y, "iao\0", 4) == 0)

            str_free(y);
        y = str_dup(x);
        str_range(y, 100, 100);
        test_cond("str_range(...,100,100)",
                  str_len(y) == 0 && memcmp(y, "\0", 1) == 0)

            str_free(y);
        str_free(x);
        x = str_new("foo");
        y = str_new("foa");
        test_cond("str_cmp(foo,foa)", str_cmp(x, y) > 0)

            str_free(y);
        str_free(x);
        x = str_new("bar");
        y = str_new("bar");
        test_cond("str_cmp(bar,bar)", str_cmp(x, y) == 0)

            str_free(y);
        str_free(x);
        x = str_new("aar");
        y = str_new("bar");
        test_cond("str_cmp(bar,bar)", str_cmp(x, y) < 0)

            str_free(y);
        str_free(x);
        x = str_newlen("\a\n\0foo\r", 7);
        y = str_catrepr(str_empty(), x, str_len(x));
        test_cond("str_catrepr(...data...)",
                  memcmp(y, "\"\\a\\n\\x00foo\\r\"", 15) == 0)

        {
            char* p;
            int step = 10, j, i;

            str_free(x);
            str_free(y);
            x = str_new("0");
            test_cond("str_new() free/len buffers", str_len(x) == 1 && str_avail(x) == 0);

            /* Run the test a few times in order to hit the first two
             * STR header types. */
            for (i = 0; i < 10; i++) {
                int oldlen = str_len(x);
                x = strMakeRoomFor(x, step);
                int type = x[-1] & STR_TYPE_MASK;

                test_cond("strMakeRoomFor() len", str_len(x) == oldlen);
                if (type != STR_TYPE_5) {
                    test_cond("strMakeRoomFor() free", str_avail(x) >= step);
                }
                p = x + oldlen;
                for (j = 0; j < step; j++) {
                    p[j] = 'A' + j;
                }
                strIncrLen(x, step);
            }
            test_cond("strMakeRoomFor() content",
                      memcmp("0ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ", x, 101) == 0);
            test_cond("strMakeRoomFor() final length", str_len(x) == 101);

            str_free(x);
        }
    }
    test_report() return 0;
}
#endif

#ifdef STR_TEST_MAIN
int main(void) {
    return strTest();
}
#endif