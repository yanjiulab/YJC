#include "base.h"
#include "concurrency.h"

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

static atomic_long s_alloc_cnt = ATOMIC_VAR_INIT(0);
static atomic_long s_free_cnt = ATOMIC_VAR_INIT(0);

long ev_alloc_cnt() { return s_alloc_cnt; }

long ev_free_cnt() { return s_free_cnt; }

void* ev_malloc(size_t size) {
    hatomic_inc(&s_alloc_cnt);
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_realloc(void* oldptr, size_t newsize, size_t oldsize) {
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

void* ev_calloc(size_t nmemb, size_t size) {
    hatomic_inc(&s_alloc_cnt);
    void* ptr = calloc(nmemb, size);
    if (!ptr) {
        fprintf(stderr, "calloc failed!\n");
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

void ev_free(void* ptr) {
    if (ptr) {
        free(ptr);
        ptr = NULL;
        hatomic_inc(&s_free_cnt);
    }
}

char* ev_strupper(char* str) {
    char* p = str;
    while (*p != '\0') {
        if (*p >= 'a' && *p <= 'z') {
            *p &= ~0x20;
        }
        ++p;
    }
    return str;
}

char* ev_strlower(char* str) {
    char* p = str;
    while (*p != '\0') {
        if (*p >= 'A' && *p <= 'Z') {
            *p |= 0x20;
        }
        ++p;
    }
    return str;
}

char* ev_strreverse(char* str) {
    if (str == NULL) return NULL;
    char* b = str;
    char* e = str;
    while (*e) {
        ++e;
    }
    --e;
    char tmp;
    while (e > b) {
        tmp = *e;
        *e = *b;
        *b = tmp;
        --e;
        ++b;
    }
    return str;
}

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

bool ev_strcontains(const char* str, const char* sub) {
    assert(str != NULL && sub != NULL);
    return strstr(str, sub) != NULL;
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

char* ev_random_string(char* buf, int len) {
    static char s_characters[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
        'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };
    int i = 0;
    for (; i < len; i++) {
        buf[i] = s_characters[ev_rand(0, sizeof(s_characters) - 1)];
    }
    buf[i] = '\0';
    return buf;
}

size_t ev_parse_size(const char* str) {
    size_t size = 0, n = 0;
    const char* p = str;
    char c;
    while ((c = *p) != '\0') {
        if (c >= '0' && c <= '9') {
            n = n * 10 + c - '0';
        } else {
            switch (c) {
                case 'K':
                case 'k':
                    n <<= 10;
                    break;
                case 'M':
                case 'm':
                    n <<= 20;
                    break;
                case 'G':
                case 'g':
                    n <<= 30;
                    break;
                case 'T':
                case 't':
                    n <<= 40;
                    break;
                default:
                    break;
            }
            size += n;
            n = 0;
        }
        ++p;
    }
    return size + n;
}

time_t ev_parse_time(const char* str) {
    time_t time = 0, n = 0;
    const char* p = str;
    char c;
    while ((c = *p) != '\0') {
        if (c >= '0' && c <= '9') {
            n = n * 10 + c - '0';
        } else {
            switch (c) {
                case 's':
                    break;
                case 'm':
                    n *= 60;
                    break;
                case 'h':
                    n *= 60 * 60;
                    break;
                case 'd':
                    n *= 24 * 60 * 60;
                    break;
                case 'w':
                    n *= 7 * 24 * 60 * 60;
                    break;
                default:
                    break;
            }
            time += n;
            n = 0;
        }
        ++p;
    }
    return time + n;
}
