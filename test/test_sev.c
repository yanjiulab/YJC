#include "sev.h"
#include "test.h"
#include <stdio.h>

static void on_stdin(int fd) {
    char* line = NULL;
    size_t size;
    int ret = getline(&line, &size, stdin);
    printf("on_stdin fd=%d readbytes=%d\n", fd, ret);
    printf("> %s\n", (char*)line);
}

static void period_hello(void *arg) {
    printf("Hello %s\n", (char*)arg);
    
}

void test_sev() {
    evloop_t* loop;
    loop = evloop_new(10);

    evio_add(loop, 0, on_stdin);
    evtimer_add(loop, period_hello, "World", 1000);

    evloop_run(loop);
}
