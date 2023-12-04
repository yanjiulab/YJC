#ifndef CONCURRENCY_H_
#define CONCURRENCY_H_

#include "datetime.h"
#include "export.h"
#include "platform.h"

//-----------------------------pthread----------------------------------------------
#define ev_getpid   (long)getpid
#ifdef HAVE_PTHREAD_H
#define gettid (long)pthread_self
#endif

typedef pthread_t thread_t;
typedef void* (*pthread_routine)(void*);

#define THREAD_RETTYPE void*
#define THREAD_ROUTINE(fname) void* fname(void* userdata)

static inline pthread_t thread_create(pthread_routine fn, void* userdata) {
    pthread_t th;
    pthread_create(&th, NULL, fn, userdata);
    return th;
}

//-----------------------------atomic----------------------------------------------
#ifdef HAVE_STDATOMIC_H
// c11

#include <stdatomic.h>

#define ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
#define ATOMIC_FLAG_CLEAR atomic_flag_clear
#define ATOMIC_ADD atomic_fetch_add
#define ATOMIC_SUB atomic_fetch_sub
#define ATOMIC_INC(p) ATOMIC_ADD(p, 1)
#define ATOMIC_DEC(p) ATOMIC_SUB(p, 1)
#else
typedef volatile bool atomic_bool;
typedef volatile char atomic_char;
typedef volatile unsigned char atomic_uchar;
typedef volatile short atomic_short;
typedef volatile unsigned short atomic_ushort;
typedef volatile int atomic_int;
typedef volatile unsigned int atomic_uint;
typedef volatile long atomic_long;
typedef volatile unsigned long atomic_ulong;
typedef volatile long long atomic_llong;
typedef volatile unsigned long long atomic_ullong;
typedef volatile size_t atomic_size_t;

typedef struct atomic_flag {
    atomic_bool _Value;
} atomic_flag;

#ifdef __GNUC__

#define ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
static inline bool atomic_flag_test_and_set(atomic_flag* p) {
    return !__sync_bool_compare_and_swap(&p->_Value, 0, 1);
}

#define ATOMIC_ADD __sync_fetch_and_add
#define ATOMIC_SUB __sync_fetch_and_sub
#define ATOMIC_INC(p) ATOMIC_ADD(p, 1)
#define ATOMIC_DEC(p) ATOMIC_SUB(p, 1)

#endif
#endif  // HAVE_STDATOMIC_H

#ifndef ATOMIC_FLAG_INIT
#define ATOMIC_FLAG_INIT \
    { 0 }
#endif

#ifndef ATOMIC_VAR_INIT
#define ATOMIC_VAR_INIT(value) (value)
#endif

#ifndef ATOMIC_FLAG_TEST_AND_SET
#define ATOMIC_FLAG_TEST_AND_SET atomic_flag_test_and_set
static inline bool atomic_flag_test_and_set(atomic_flag* p) {
    bool ret = p->_Value;
    p->_Value = 1;
    return ret;
}
#endif

#ifndef ATOMIC_FLAG_CLEAR
#define ATOMIC_FLAG_CLEAR atomic_flag_clear
static inline void atomic_flag_clear(atomic_flag* p) {
    p->_Value = 0;
}
#endif

#ifndef ATOMIC_ADD
#define ATOMIC_ADD(p, n) (*(p) += (n))
#endif

#ifndef ATOMIC_SUB
#define ATOMIC_SUB(p, n) (*(p) -= (n))
#endif

#ifndef ATOMIC_INC
#define ATOMIC_INC(p) ((*(p))++)
#endif

#ifndef ATOMIC_DEC
#define ATOMIC_DEC(p) ((*(p))--)
#endif

typedef atomic_flag hatomic_flag_t;
#define HATOMIC_FLAG_INIT ATOMIC_FLAG_INIT
#define hatomic_flag_test_and_set ATOMIC_FLAG_TEST_AND_SET
#define hatomic_flag_clear ATOMIC_FLAG_CLEAR

typedef atomic_long hatomic_t;
#define HATOMIC_VAR_INIT ATOMIC_VAR_INIT
#define hatomic_add ATOMIC_ADD
#define hatomic_sub ATOMIC_SUB
#define hatomic_inc ATOMIC_INC
#define hatomic_dec ATOMIC_DEC

//-----------------------------mutex----------------------------------------------

#define hmutex_t pthread_mutex_t
#define hmutex_init(pmutex) pthread_mutex_init(pmutex, NULL)
#define hmutex_destroy pthread_mutex_destroy
#define hmutex_lock pthread_mutex_lock
#define hmutex_unlock pthread_mutex_unlock

#define hrecursive_mutex_t pthread_mutex_t
#define hrecursive_mutex_init(pmutex)                              \
    do {                                                           \
        pthread_mutexattr_t attr;                                  \
        pthread_mutexattr_init(&attr);                             \
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
        pthread_mutex_init(pmutex, &attr);                         \
    } while (0)
#define hrecursive_mutex_destroy pthread_mutex_destroy
#define hrecursive_mutex_lock pthread_mutex_lock
#define hrecursive_mutex_unlock pthread_mutex_unlock

#if HAVE_PTHREAD_SPIN_LOCK
#define hspinlock_t pthread_spinlock_t
#define hspinlock_init(pspin) pthread_spin_init(pspin, PTHREAD_PROCESS_PRIVATE)
#define hspinlock_destroy pthread_spin_destroy
#define hspinlock_lock pthread_spin_lock
#define hspinlock_unlock pthread_spin_unlock
#else
#define hspinlock_t pthread_mutex_t
#define hspinlock_init(pmutex) pthread_mutex_init(pmutex, NULL)
#define hspinlock_destroy pthread_mutex_destroy
#define hspinlock_lock pthread_mutex_lock
#define hspinlock_unlock pthread_mutex_unlock
#endif

#define hrwlock_t pthread_rwlock_t
#define hrwlock_init(prwlock) pthread_rwlock_init(prwlock, NULL)
#define hrwlock_destroy pthread_rwlock_destroy
#define hrwlock_rdlock pthread_rwlock_rdlock
#define hrwlock_rdunlock pthread_rwlock_unlock
#define hrwlock_wrlock pthread_rwlock_wrlock
#define hrwlock_wrunlock pthread_rwlock_unlock

#define htimed_mutex_t pthread_mutex_t
#define htimed_mutex_init(pmutex) pthread_mutex_init(pmutex, NULL)
#define htimed_mutex_destroy pthread_mutex_destroy
#define htimed_mutex_lock pthread_mutex_lock
#define htimed_mutex_unlock pthread_mutex_unlock
static inline void timespec_after(struct timespec* ts, unsigned int ms) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts->tv_sec = tv.tv_sec + ms / 1000;
    ts->tv_nsec = tv.tv_usec * 1000 + ms % 1000 * 1000000;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec += 1;
    }
}
// true:  OK
// false: ETIMEDOUT
static inline int htimed_mutex_lock_for(htimed_mutex_t* mutex, unsigned int ms) {
#if HAVE_PTHREAD_MUTEX_TIMEDLOCK
    struct timespec ts;
    timespec_after(&ts, ms);
    return pthread_mutex_timedlock(mutex, &ts) != ETIMEDOUT;
#else
    int ret = 0;
    unsigned int end = gettick_ms() + ms;
    while ((ret = pthread_mutex_trylock(mutex)) != 0) {
        if (gettick_ms() >= end) {
            break;
        }
        hv_msleep(1);
    }
    return ret == 0;
#endif
}

#define hcondvar_t pthread_cond_t
#define hcondvar_init(pcond) pthread_cond_init(pcond, NULL)
#define hcondvar_destroy pthread_cond_destroy
#define hcondvar_wait pthread_cond_wait
#define hcondvar_signal pthread_cond_signal
#define hcondvar_broadcast pthread_cond_broadcast
// true:  OK
// false: ETIMEDOUT
static inline int hcondvar_wait_for(hcondvar_t* cond, hmutex_t* mutex, unsigned int ms) {
    struct timespec ts;
    timespec_after(&ts, ms);
    return pthread_cond_timedwait(cond, mutex, &ts) != ETIMEDOUT;
}

#define honce_t pthread_once_t
#define HONCE_INIT PTHREAD_ONCE_INIT
#define honce pthread_once

#include <semaphore.h>
#define hsem_t sem_t
#define hsem_init(psem, value) sem_init(psem, 0, value)
#define hsem_destroy sem_destroy
#define hsem_wait sem_wait
#define hsem_post sem_post
// true:  OK
// false: ETIMEDOUT
static inline int hsem_wait_for(hsem_t* sem, unsigned int ms) {
#if HAVE_SEM_TIMEDWAIT
    struct timespec ts;
    timespec_after(&ts, ms);
    return sem_timedwait(sem, &ts) != ETIMEDOUT;
#else
    int ret = 0;
    unsigned int end = gettick_ms() + ms;
    while ((ret = sem_trywait(sem)) != 0) {
        if (gettick_ms() >= end) {
            break;
        }
        hv_msleep(1);
    }
    return ret == 0;
#endif
}

#endif  // HV_MUTEX_H_
