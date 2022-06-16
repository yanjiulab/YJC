// https://gohalo.me/post/linux-libev.html
#include <ev.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ini.h"
#include "ipa.h"
#include "proto.h"
#include "sock.h"
#include "str.h"
#include "timer.h"

#define RECV_BUF_SIZE 4096
char recv_buf[RECV_BUF_SIZE];

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void stdin_cb(EV_P_ ev_io *w, int revents) {
    char *line = NULL;
    size_t size;
    int ret = getline(&line, &size, stdin);
    printf("> %s", line);
    // for one-shot events, one must manually stop the watcher
    // with its corresponding stop function.
    // ev_io_stop(EV_A_ w);

    // this causes all nested ev_run's to stop iterating
    // ev_break(EV_A_ EVBREAK_ALL);
}

// another callback, this time for a time-out
static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    static int retry = 0;
    printf("timeout: ");
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    printf("%d:%d\n", curtime.tv_sec, curtime.tv_usec);

    // this causes the innermost ev_run to stop iterating
    // if (++retry == 3) ev_break(EV_A_ EVBREAK_ONE);
    if (++retry == 3) ev_timer_stop(loop, w);
}

static void sock_cb(EV_P_ ev_io *w, int revents) {
    ssize_t len;

    struct sockaddr_in from;
    socklen_t fromlen;

    fromlen = sizeof(from);

    // while (() < 0) {
    // }

    len = recvfrom(w->fd, recv_buf, RECV_BUF_SIZE, 0, (struct sockaddr *)&from, &fromlen);

    // struct sockaddr_ll sll;
    // bzero(&sll, sizeof(sll));
    // sll.sll_ifindex = if_nametoindex("ens33");
    
    // int ret = sendto(w->fd, recv_buf, len, 0, (struct sockaddr *) &sll, sizeof(sll));
    // printf("send %d \n", ret);

    print_frame(len);
    struct ethhdr *ethh = parse_ethhdr(recv_buf, len);
    print_ethhdr(ethh, 0);
    struct iphdr *iph = parse_iphdr(recv_buf, len);


}

int main(int argc, char *argv[]) {
    printf("Hello YJC\n");

    for (int i = 0; i < argc; i++) {
        printf("args(%d/%d): %s\n", i, argc, argv[i]);
    }
    int pktfd = sock_packet("ens33");
    printf("%d", pktfd);
    //  every watcher type has its own typedef'd struct
    //  with the name ev_TYPE

    // use the default event loop unless you have special needs
    struct ev_loop *loop = EV_DEFAULT;

    // initialise an io watcher, then start it
    // this one will watch for stdin to become readable
    ev_io stdin_watcher;
    ev_io_init(&stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(loop, &stdin_watcher);

    ev_io sock_watcher;
    ev_io_init(&sock_watcher, sock_cb, pktfd, EV_READ);
    ev_io_start(loop, &sock_watcher);

    // initialise a timer watcher, then start it simple non-repeating 5.5 second timeout, and repeating 3 second
    ev_timer timeout_watcher;
    ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 3.);
    ev_timer_start(loop, &timeout_watcher);

    // now wait for events to arrive
    ev_run(loop, 0);

    // break was called, so exit
    return 0;
}
