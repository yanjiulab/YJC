/* thread.h */

#ifndef __THREAD_H__
#define __THREAD_H__

/* Headers */
#define __USE_XOPEN2K 1 // for posix extension
#include <errno.h>
#include <features.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

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

/* ---------------------------- pthread ---------------------------- */
typedef pthread_t thread_t;
typedef void* (*thread_routine)(void*);

#define thread_id pthread_self       // obtain ID of the calling thread
#define gettid (long)pthread_self    // obtain ID of the calling thread in proc style
#define thread_equal pthread_equal   // compare thread IDs
#define thread_exit pthread_exit     // terminate calling thread
#define thread_join pthread_join     // join with a terminated thread
#define thread_cancel pthread_cancel // send a cancellation request to a thread
#define thread_atexit_push \
    pthread_cleanup_push // push thread cancellation clean-up handlers
#define thread_atexit_pop \
    pthread_cleanup_pop // pop thread cancellation clean-up handlers

#define THREAD_ROUTINE(fname) void* fname(void* userdata)
static inline thread_t thread_create(thread_routine fn, void* userdata) {
    thread_t th;
    pthread_create(&th, NULL, fn, userdata);
    return th;
}

/* ---------------------------- pthread mutex ---------------------------- */
#define mutex_t pthread_mutex_t
#define mutex_init(pmutex) pthread_mutex_init(pmutex, NULL)
// make recursive mutex
#define recursive_mutex_init(pmutex)                               \
    do {                                                           \
        pthread_mutexattr_t attr;                                  \
        pthread_mutexattr_init(&attr);                             \
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
        pthread_mutex_init(pmutex, &attr);                         \
    } while (0)
#define mutex_destroy pthread_mutex_destroy
#define mutex_lock pthread_mutex_lock
#define mutex_trylock pthread_mutex_trylock
static inline int thread_mutex_lock_for(pthread_mutex_t* mutex,
                                        unsigned int ms) {
    struct timespec ts;
    timespec_after(&ts, ms);
    return pthread_mutex_timedlock(mutex, &ts) != ETIMEDOUT;
}
// lock for t ms
#define mutex_timedlock(pmutex, ms) thread_mutex_lock_for(pmutex, ms)
#define mutex_unlock pthread_mutex_unlock

/* ---------------------------- pthread recursive mutex ---------------------------- */
#define recursive_mutex_t pthread_mutex_t
#define recursive_mutex_init(pmutex)                               \
    do {                                                           \
        pthread_mutexattr_t attr;                                  \
        pthread_mutexattr_init(&attr);                             \
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); \
        pthread_mutex_init(pmutex, &attr);                         \
    } while (0)
#define recursive_mutex_destroy pthread_mutex_destroy
#define recursive_mutex_lock pthread_mutex_lock
#define recursive_mutex_trylock pthread_mutex_trylock
#define recursive_mutex_unlock pthread_mutex_unlock

/* ---------------------------- pthread rwlock ---------------------------- */
#define rwlock_t pthread_rwlock_t
#define rwlock_init(prwlock) pthread_rwlock_init(prwlock, NULL)
#define rwlock_destroy pthread_rwlock_destroy
#define rwlock_rdlock pthread_rwlock_rdlock
#define rwlock_tryrdlock pthread_rwlock_tryrdlock
#define rwlock_rdunlock pthread_rwlock_unlock
#define rwlock_wrlock pthread_rwlock_wrlock
#define rwlock_trywrlock pthread_rwlock_trywrlock
#define rwlock_wrunlock pthread_rwlock_unlock

/* ---------------------------- pthread spin ---------------------------- */
#define spinlock_t pthread_spinlock_t
#define spinlock_init(pspin) pthread_spin_init(pspin, PTHREAD_PROCESS_PRIVATE)
#define spinlock_destroy pthread_spin_destroy
#define spinlock_lock pthread_spin_lock
#define spinlock_trylock pthread_spin_trylock
#define spinlock_unlock pthread_spin_unlock

/* ---------------------------- pthread barrier ---------------------------- */
#define barrier_t pthread_barrier_t;
#define barrier_init(pbarrier, cnt) pthread_barrier_init(pbarrier, NULL, cnt)
#define barrier_destroy pthread_barrier_destroy
#define barrier_wait pthread_barrier_wait

/* ---------------------------- pthread condvar ---------------------------- */
#define condvar_t pthread_cond_t
#define condvar_init(pcond) pthread_cond_init(pcond, NULL)
#define condvar_destroy pthread_cond_destroy
#define condvar_wait pthread_cond_wait
#define condvar_signal pthread_cond_signal
#define condvar_broadcast pthread_cond_broadcast
// true:  OK
// false: ETIMEDOUT
static inline int condvar_wait_for(condvar_t* cond, mutex_t* mutex,
                                   unsigned int ms) {
    struct timespec ts;
    timespec_after(&ts, ms);
    return pthread_cond_timedwait(cond, mutex, &ts) != ETIMEDOUT;
}

/* ---------------------------- pthread once ---------------------------- */
#define thread_once_t pthread_once_t
#define THREAD_ONCE_INIT PTHREAD_ONCE_INIT
#define thread_once pthread_once // dynamic package initialization

/* ---------------------------- semaphore ---------------------------- */
#include <semaphore.h>
#define sem_t sem_t // actually a counter
// sem_init(psem, 1) == mutex
#define sem_init(psem, value) sem_init(psem, PTHREAD_PROCESS_PRIVATE, value)
#define sem_destroy sem_destroy
// if counter > 0, return immediately and counter - 1; else block current thread
#define sem_wait sem_wait
// counter + 1
#define sem_post sem_post
// true:  OK
// false: ETIMEDOUT
static inline int sem_wait_for(sem_t* sem, unsigned int ms) {
    struct timespec ts;
    timespec_after(&ts, ms);
    return sem_timedwait(sem, &ts) != ETIMEDOUT;
}

#ifdef __cplusplus
}
#endif

#endif /* __THREAD_H__ */
