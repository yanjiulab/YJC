#include <stdint.h>
#include <stdio.h>

#include "log.h"
#include "test.h"
#include "thread.h"
#include "datetime.h"
THREAD_ROUTINE(my_routine) {
    printf("Thread #%u working on %d\n", (int)pthread_self(), (int)userdata);
}

void test_thread() {
    log_info("main thread: %lu", thread_id());

    int i;
    thread_t tids[40];
    for (i = 0; i < 40; i++) {
        tids[i] = thread_create(my_routine, (void *)(uintptr_t)i);
    };

    unsigned long long start = gettimeofday_us();
    for (i = 0; i < 40; i++) {
        thread_join(tids[i], NULL);
    };
    unsigned long long end = gettimeofday_us();

    printf("Killing, time: %lld us\n", (end - start));
    // rwlock_init(rwl);
}