// https://gohalo.me/post/linux-libev.html
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ev.h"
#include "eventloop.h"
#include "inet.h"
#include "ini.h"
#include "log.h"
#include "proto.h"
#include "socket.h"
#include "str.h"
// #include "time.h"
// #include "timer.h"

// int main(int argc, char *argv[]) {

//     for (int i = 0; i < argc; i++) {
//         printf("args(%d/%d): %s\n", i, argc, argv[i]);
//     }

//     return 0;
// }

#include "base.h"
#include "eventloop.h"

void on_timer(etimer_t* timer) {
    eloop_t* loop = event_loop(timer);
    printf("on_timer: event_id=%llu\tpriority=%d\tuserdata=%ld\ttime=%llus\thrtime=%lluus\n", LLU(event_id(timer)),
           event_priority(timer), (long)(intptr_t)(event_userdata(timer)), LLU(eloop_now(loop)),
           LLU(eloop_now_hrtime(loop)));
}

void on_stdin(eio_t* io, void* buf, int readbytes) {
    printf("on_stdin fd=%d readbytes=%d\n", eio_fd(io), readbytes);
    printf("> %s\n", (char*)buf);
    if (strncmp((char*)buf, "quit", 4) == 0) {
        eloop_stop(event_loop(io));
    }
}

int main() {
    // memcheck atexit
    EV_MEMCHECK;

    eloop_t* loop = eloop_new(0);

    // // test idle and priority
    // for (int i = EVENT_LOWEST_PRIORITY; i <= EVENT_HIGHEST_PRIORITY; ++i) {
    //     eidle_t* idle = eidle_add(loop, on_idle, 10);
    //     event_set_priority(idle, i);
    // }

    // test timer timeout
    for (int i = 1; i <= 10; ++i) {
        etimer_t* timer = etimer_add(loop, on_timer, i * 1000, 3);
        event_set_userdata(timer, (void*)(intptr_t)i);
    }

    // test nonblock stdin
    printf("input 'quit' to quit loop\n");
    char buf[64];
    hread(loop, 0, buf, sizeof(buf), on_stdin);

    eloop_run(loop);
    eloop_free(&loop);
    return 0;
}