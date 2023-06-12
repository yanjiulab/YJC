#include <stdio.h>

#include "log.h"
#include "test.h"
#include "thread.h"
THREAD_ROUTINE(my_routine) {
    log_info("new thread: %lu, args addr: %p", thread_id(), NULL);
    return (void *)1;
}

void test_thread() {
    log_info("main thread: %lu", thread_id());

    void *rval;
    thread_t tid = thread_create(my_routine, NULL);
    thread_join(tid, &rval);
    log_info("thread %lu exit code %ld", tid, (long)rval);

    mutex_t mt = PTHREAD_MUTEX_INITIALIZER;
    rwlock_t rwl;
    // rwlock_init(rwl);
}