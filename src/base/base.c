#include "base.h"

#include "concurrency.h"

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif
//--------------------alloc/free---------------------------
static atomic_long s_alloc_cnt = ATOMIC_VAR_INIT(0);
static atomic_long s_free_cnt = ATOMIC_VAR_INIT(0);

long ev_alloc_cnt() { return s_alloc_cnt; }

long ev_free_cnt() { return s_free_cnt; }

inline void* ev_malloc(size_t size) {
    hatomic_inc(&s_alloc_cnt);
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_calloc(size_t nmemb, size_t size) {
    hatomic_inc(&s_alloc_cnt);
    void* ptr = calloc(nmemb, size);
    if (!ptr) {
        fprintf(stderr, "calloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_realloc(void* oldptr, size_t newsize) {
    hatomic_inc(&s_alloc_cnt);
    hatomic_inc(&s_free_cnt);
    void* ptr = realloc(oldptr, newsize);

    if (!ptr) {
        fprintf(stderr, "realloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_zalloc(size_t size) {
    hatomic_inc(&s_alloc_cnt);
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed!\n");
        exit(-1);
    }
    memset(ptr, 0, size);
    return ptr;
}

void* ev_zrealloc(void* oldptr, size_t newsize, size_t oldsize) {
    hatomic_inc(&s_alloc_cnt);
    hatomic_inc(&s_free_cnt);
    void* ptr = realloc(oldptr, newsize);
    if (!ptr) {
        fprintf(stderr, "realloc failed!\n");
        exit(-1);
    }
    if (newsize > oldsize) {
        memset((char*)ptr + oldsize, 0, newsize - oldsize);
    }
    return ptr;
}

void ev_free(void* ptr) {
    if (ptr) {
        free(ptr);
        ptr = NULL;
        hatomic_inc(&s_free_cnt);
    }
}

//--------------------string---------------------------

// n = sizeof(dest_buf)
char* ev_strncpy(char* dest, const char* src, size_t n) {
    assert(dest != NULL && src != NULL);
    char* ret = dest;
    while (*src != '\0' && --n > 0) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}

// n = sizeof(dest_buf)
char* ev_strncat(char* dest, const char* src, size_t n) {
    assert(dest != NULL && src != NULL);
    char* ret = dest;
    while (*dest) {
        ++dest;
        --n;
    }
    while (*src != '\0' && --n > 0) {
        *dest++ = *src++;
    }
    *dest = '\0';
    return ret;
}

bool ev_strstartswith(const char* str, const char* start) {
    assert(str != NULL && start != NULL);
    while (*str && *start && *str == *start) {
        ++str;
        ++start;
    }
    return *start == '\0';
}

bool ev_strendswith(const char* str, const char* end) {
    assert(str != NULL && end != NULL);
    int len1 = 0;
    int len2 = 0;
    while (*str) {
        ++str;
        ++len1;
    }
    while (*end) {
        ++end;
        ++len2;
    }
    if (len1 < len2) return false;
    while (len2-- > 0) {
        --str;
        --end;
        if (*str != *end) {
            return false;
        }
    }
    return true;
}

char* ev_strnchr(const char* s, char c, size_t n) {
    assert(s != NULL);
    const char* p = s;
    while (*p != '\0' && n-- > 0) {
        if (*p == c) return (char*)p;
        ++p;
    }
    return NULL;
}

char* ev_strrchr_dir(const char* filepath) {
    char* p = (char*)filepath;
    while (*p) ++p;
    while (--p >= filepath) {
#ifdef OS_WIN
        if (*p == '/' || *p == '\\')
#else
        if (*p == '/')
#endif
            return p;
    }
    return NULL;
}

char* get_run_dir(char* buf, int size) { return getcwd(buf, size); }

int ev_rand(int min, int max) {
    static int s_seed = 0;
    assert(max > min);

    if (s_seed == 0) {
        s_seed = time(NULL);
        srand(s_seed);
    }

    int _rand = rand();
    _rand = min + (int)((double)((double)(max) - (min) + 1.0) * ((_rand) / ((RAND_MAX) + 1.0)));
    return _rand;
}
