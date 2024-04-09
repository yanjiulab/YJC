#ifndef EV_BASE_H_
#define EV_BASE_H_

#include "datetime.h"
#include "defs.h" // for printd
#include "errors.h"
#include "export.h"
#include "log.h"
#include "macros.h"
#include "math.h"
#include "platform.h" // for bool
#include "str.h"
#include "types.h"
#include "version.h"
#include "atomic.h"
#include "proc.h"
#include "thread.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <syslog.h>

#define MAXLINE  4096 /* max text line length */
#define BUFFSIZE 8192 /* buffer size for reads and writes */

//--------------------alloc/free---------------------------
EXPORT void* ev_malloc(size_t size);
EXPORT void* ev_calloc(size_t nmemb, size_t size);
EXPORT void* ev_zalloc(size_t size);
EXPORT void* ev_realloc(void* oldptr, size_t newsize);
EXPORT void* ev_zrealloc(void* oldptr, size_t newsize, size_t oldsize);
EXPORT void ev_free(void* ptr);

EXPORT long ev_alloc_cnt();
EXPORT long ev_free_cnt();
static inline void ev_memcheck(void) {
    printd("Memcheck => alloc:%ld free:%ld", ev_alloc_cnt(), ev_free_cnt());
}
#define MEMCHECK atexit(ev_memcheck);

#define EV_ALLOC(ptr, size)                                                                                \
    do {                                                                                                   \
        *(void**)&(ptr) = ev_zalloc(size);                                                                 \
        printd("alloc(%p, size=%llu)\tat [%s:%d:%s]", ptr, (unsigned long long)size, __FILE__, __LINE__, \
               __FUNCTION__);                                                                              \
    } while (0)

#define EV_ALLOC_SIZEOF(ptr) EV_ALLOC(ptr, sizeof(*(ptr)))

#define EV_FREE(ptr)                                                                      \
    do {                                                                                  \
        if (ptr) {                                                                        \
            ev_free(ptr);                                                                 \
            printd("free( %p )\tat [%s:%d:%s]", ptr, __FILE__, __LINE__, __FUNCTION__); \
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

#define EV_DEFAULT_STACKBUF_SIZE  1024
#define EV_STACK_ALLOC(ptr, size) STACK_OR_HEAP_ALLOC(ptr, size, EV_DEFAULT_STACKBUF_SIZE)
#define EV_STACK_FREE(ptr)        STACK_OR_HEAP_FREE(ptr)

//--------------------file-------------------------------
#define FILE_MODE                 (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)   /* default file access permissions for new files */
#define DIR_MODE                  (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH) /* default permissions for new directories */

//--------------------path-------------------------------
EXPORT int mkdir_p(const char* dir); // mkdir -p
EXPORT int rmdir_p(const char* dir); // rmdir -p
EXPORT bool path_exists(const char* path);
EXPORT bool path_isdir(const char* path);
EXPORT bool path_isfile(const char* path);
EXPORT bool path_islink(const char* path);
EXPORT size_t filesize(const char* filepath);

//--------------------random-------------------------------
EXPORT int rand_int(int min, int max);
// Generates a random string of len in heap if buf is NULL, otherwise in buf.
EXPORT char* rand_str(char* buf, int len);

//--------------------daemon-------------------------------
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
int already_running(const char* fname);
void daemonize(const char* cmd);

//--------------------signal-------------------------------
typedef void Sigfunc(int); /* for signal handlers */
Sigfunc* signal_intr(int signo, Sigfunc* func);

#endif // EV_BASE_H_
