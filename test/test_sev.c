#include <stdio.h>

#include "sev.h"
#include "test.h"

static void on_stdin(int fd) {
    char* line = NULL;
    size_t size;
    int ret = getline(&line, &size, stdin);
    printf("on_stdin fd=%d readbytes=%d\n", fd, ret);
    printf("> %s\n", (char*)line);
}

static void period_hello(evtimer_t* timer, void* arg) {
    printf("Hello %s\n", (char*)arg);
    evtimer_add(event_loop(timer), period_hello, "World", 1000);
}

void test_sev() {
    evloop_t* loop = evloop_new(10);

    evio_add(loop, 0, on_stdin);
    evtimer_add(loop, period_hello, "World", 1000);

    evloop_run(loop);
}
