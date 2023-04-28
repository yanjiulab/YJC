#ifndef EV_BASE_H_
#define EV_BASE_H_

#include "defs.h"  // for printd
#include "export.h"
#include "platform.h"  // for bool

//--------------------alloc/free---------------------------
EXPORT void* ev_malloc(size_t size);
EXPORT void* ev_calloc(size_t nmemb, size_t size);
EXPORT void* ev_zalloc(size_t size);
EXPORT void* ev_realloc(void* oldptr, size_t newsize);
EXPORT void* ev_zrealloc(void* oldptr, size_t newsize, size_t oldsize);
EXPORT void ev_free(void* ptr);

#define ALLOC(size) ev_zalloc(size)
#define ALLOC_SIZEOF(ptr) ALLOC(sizeof(*(ptr)))
#define MALLOC(size) ev_malloc(size)
#define CALLOC(n, size) ev_calloc(n, size)
#define REALLOC(ptr, size) ev_realloc(ptr, size)
#define ZREALLOC(ptr, nsize, osize) ev_zrealloc(ptr, nsize, osize)
#define FREE(ptr) EV_FREE(ptr)

EXPORT long ev_alloc_cnt();
EXPORT long ev_free_cnt();
INLINE void ev_memcheck(void) { printf("Memcheck => alloc:%ld free:%ld\n", ev_alloc_cnt(), ev_free_cnt()); }
#define MEMCHECK atexit(ev_memcheck);

#define EV_ALLOC(ptr, size)                                                                                \
    do {                                                                                                   \
        *(void**)&(ptr) = ev_zalloc(size);                                                                 \
        printd("alloc(%p, size=%llu)\tat [%s:%d:%s]\n", ptr, (unsigned long long)size, __FILE__, __LINE__, \
               __FUNCTION__);                                                                              \
    } while (0)

#define EV_ALLOC_SIZEOF(ptr) EV_ALLOC(ptr, sizeof(*(ptr)))

#define EV_FREE(ptr)                                                                      \
    do {                                                                                  \
        if (ptr) {                                                                        \
            ev_free(ptr);                                                                 \
            printd("free( %p )\tat [%s:%d:%s]\n", ptr, __FILE__, __LINE__, __FUNCTION__); \
            ptr = NULL;                                                                   \
        }                                                                                 \
    } while (0)

#define STACK_OR_HEAP_ALLOC(ptr, size, stack_size) \
    unsigned char _stackbuf_[stack_size] = {0};    \
    if ((size) > (stack_size)) {                   \
        EV_ALLOC(ptr, size);                       \
    } else {                                       \
        *(unsigned char**)&(ptr) = _stackbuf_;     \
    }

#define STACK_OR_HEAP_FREE(ptr)                \
    if ((unsigned char*)(ptr) != _stackbuf_) { \
        EV_FREE(ptr);                          \
    }

#define EV_DEFAULT_STACKBUF_SIZE 1024
#define EV_STACK_ALLOC(ptr, size) STACK_OR_HEAP_ALLOC(ptr, size, EV_DEFAULT_STACKBUF_SIZE)
#define EV_STACK_FREE(ptr) STACK_OR_HEAP_FREE(ptr)

//--------------------string-------------------------------
EXPORT char* ev_strupper(char* str);
EXPORT char* ev_strlower(char* str);
EXPORT char* ev_strreverse(char* str);

EXPORT bool ev_strstartswith(const char* str, const char* start);
EXPORT bool ev_strendswith(const char* str, const char* end);
EXPORT bool ev_strcontains(const char* str, const char* sub);

// strncpy n = sizeof(dest_buf)-1
// ev_strncpy n = sizeof(dest_buf)
EXPORT char* ev_strncpy(char* dest, const char* src, size_t n);

// strncat n = sizeof(dest_buf)-1-strlen(dest)
// ev_strncpy n = sizeof(dest_buf)
EXPORT char* ev_strncat(char* dest, const char* src, size_t n);

#if !HAVE_STRLCPY
#define strlcpy ev_strncpy
#endif

#if !HAVE_STRLCAT
#define strlcat ev_strncat
#endif

EXPORT char* ev_strnchr(const char* s, char c, size_t n);

#define ev_strrchr_dot(str) strrchr(str, '.')
EXPORT char* ev_strrchr_dir(const char* filepath);

// basename
EXPORT const char* ev_basename(const char* filepath);
EXPORT const char* ev_suffixname(const char* filename);
// mkdir -p
EXPORT int ev_mkdir_p(const char* dir);
// rmdir -p
EXPORT int ev_rmdir_p(const char* dir);
// path
EXPORT bool ev_exists(const char* path);
EXPORT bool ev_isdir(const char* path);
EXPORT bool ev_isfile(const char* path);
EXPORT bool ev_islink(const char* path);
EXPORT size_t ev_filesize(const char* filepath);

EXPORT char* get_executable_path(char* buf, int size);
EXPORT char* get_executable_dir(char* buf, int size);
EXPORT char* get_executable_file(char* buf, int size);
EXPORT char* get_run_dir(char* buf, int size);

// random
EXPORT int ev_rand(int min, int max);
EXPORT char* ev_random_string(char* buf, int len);

// 1 y on yes true enable => true
EXPORT bool ev_getboolean(const char* str);
// 1T2G3M4K5B => ?B
EXPORT size_t ev_parse_size(const char* str);
// 1w2d3h4m5s => ?s
EXPORT time_t ev_parse_time(const char* str);

// scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
typedef enum {
    EV_URL_SCHEME,
    EV_URL_USERNAME,
    EV_URL_PASSWORD,
    EV_URL_HOST,
    EV_URL_PORT,
    EV_URL_PATH,
    EV_URL_QUERY,
    EV_URL_FRAGMENT,
    EV_URL_FIELD_NUM,
} hurl_field_e;

typedef struct hurl_s {
    struct {
        unsigned short off;
        unsigned short len;
    } fields[EV_URL_FIELD_NUM];
    unsigned short port;
} hurl_t;

EXPORT int ev_parse_url(hurl_t* stURL, const char* strURL);

#endif  // EV_BASE_H_
