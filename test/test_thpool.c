#include <stdint.h>

#include "test.h"
#include "thpool.h"
#include "datetime.h"


void task(void *arg) {
    printf("Thread #%u working on %d\n", (int)pthread_self(), (int)arg);
}

void test_thpool() {
    puts("Making threadpool with 4 threads");
    puts("Adding 40 tasks to threadpool");
    threadpool thpool = thpool_init(4);
    unsigned long long start = gettimeofday_us();

    int i;
    for (i = 0; i < 40; i++) {
        thpool_add_work(thpool, task, (void *)(uintptr_t)i);
    };

    thpool_wait(thpool);
    unsigned long long end = gettimeofday_us();
    thpool_destroy(thpool);
    printf("Killing threadpool, time: %lld us\n", (end - start));

    return 0;
}
