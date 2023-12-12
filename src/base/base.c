#include "base.h"

#include <stdatomic.h>

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif
#ifndef atomic_inc
#define atomic_inc(p) atomic_fetch_add(p, 1)
#endif

//--------------------alloc/free---------------------------
static atomic_long s_alloc_cnt = ATOMIC_VAR_INIT(0);
static atomic_long s_free_cnt = ATOMIC_VAR_INIT(0);

long ev_alloc_cnt() { return s_alloc_cnt; }

long ev_free_cnt() { return s_free_cnt; }

inline void* ev_malloc(size_t size) {
    atomic_inc(&s_alloc_cnt);
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_calloc(size_t nmemb, size_t size) {
    atomic_inc(&s_alloc_cnt);
    void* ptr = calloc(nmemb, size);
    if (!ptr) {
        fprintf(stderr, "calloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_realloc(void* oldptr, size_t newsize) {
    atomic_inc(&s_alloc_cnt);
    atomic_inc(&s_free_cnt);
    void* ptr = realloc(oldptr, newsize);

    if (!ptr) {
        fprintf(stderr, "realloc failed!\n");
        exit(-1);
    }
    return ptr;
}

void* ev_zalloc(size_t size) {
    atomic_inc(&s_alloc_cnt);
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "malloc failed!\n");
        exit(-1);
    }
    memset(ptr, 0, size);
    return ptr;
}

void* ev_zrealloc(void* oldptr, size_t newsize, size_t oldsize) {
    atomic_inc(&s_alloc_cnt);
    atomic_inc(&s_free_cnt);
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
        atomic_inc(&s_free_cnt);
    }
}

//--------------------random---------------------------

int rand_int(int min, int max) {
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

char *rand_str(char *buf, int len) {
    static char s_characters[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
        'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };

    if (buf == NULL) buf = (char *)calloc(1, len + 1);

    int i = 0;
    srand(time(NULL));
    for (; i < len; i++) {
        buf[i] = s_characters[rand() % (sizeof(s_characters))];
    }
    buf[i] = '\0';
    return buf;
}

//--------------------path---------------------------

bool path_exists(const char* path) {
    return access(path, 0) == 0;
}

bool path_isdir(const char* path) {
    if (access(path, 0) != 0) return false;
    struct stat st;
    memset(&st, 0, sizeof(st));
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

bool path_isfile(const char* path) {
    if (access(path, 0) != 0) return false;
    struct stat st;
    memset(&st, 0, sizeof(st));
    stat(path, &st);
    return S_ISREG(st.st_mode);
}

bool path_islink(const char* path) {
    if (access(path, 0) != 0) return false;
    struct stat st;
    memset(&st, 0, sizeof(st));
    lstat(path, &st);
    return S_ISLNK(st.st_mode);
}

size_t filesize(const char* filepath) {
    struct stat st;
    memset(&st, 0, sizeof(st));
    stat(filepath, &st);
    return st.st_size;
}

int mkdir_p(const char* dir) {
    if (access(dir, 0) == 0) {
        return EEXIST;
    }
    char tmp[MAX_PATH] = {0};
    strncpy(tmp, dir, sizeof(tmp));
    char* p = tmp;
    char delim = '/';
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0777);
            *p = delim;
        }
        ++p;
    }
    if (mkdir(tmp, 0777) != 0) {
        return EPERM;
    }
    return 0;
}

int rmdir_p(const char* dir) {
    if (access(dir, 0) != 0) {
        return ENOENT;
    }
    if (rmdir(dir) != 0) {
        return EPERM;
    }
    char tmp[MAX_PATH] = {0};
    strncpy(tmp, dir, sizeof(tmp));
    char* p = tmp;
    while (*p) ++p;
    while (--p >= tmp) {
        if (*p == '/') {
            *p = '\0';
            if (rmdir(tmp) != 0) {
                return 0;
            }
        }
    }
    return 0;
}