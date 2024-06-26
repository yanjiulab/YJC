// https://gohalo.me/post/linux-libev.html
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <linux/netlink.h>
#include <net/ethernet.h> //For ether_header
#include <net/if.h>
#include <netdb.h>
#include <netinet/if_ether.h> // for ETH_P_ALL
#include <netinet/ip.h>       //Provides declarations for ip header
#include <netinet/ip_icmp.h>  //Provides declarations for icmp header
#include <netinet/tcp.h>      //Provides declarations for tcp header
#include <netinet/udp.h>      //Provides declarations for udp header
#include <stdint.h>
#include <stdio.h>  //For standard things
#include <stdlib.h> //malloc
#include <string.h> //strlen
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "base.h"
#include "eventloop.h"
#include "iniparser.h"
#include "log.h"
#include "net_utils.h"
#include "socket.h"
#include "str.h"
#include "version.h"

void on_idle(eidle_t* idle) {
    printf("on_idle: event_id=%llu\tpriority=%d\tuserdata=%ld\n", LLU(event_id(idle)), event_priority(idle),
           (long)(intptr_t)(event_userdata(idle)));
}

void on_timer(etimer_t* timer) {
    eloop_t* loop = event_loop(timer);
    printf("on_timer: event_id=%llu\tpriority=%d\tuserdata=%ld\ttime=%llus\thrtime=%lluus\n", LLU(event_id(timer)),
           event_priority(timer), (long)(intptr_t)(event_userdata(timer)), LLU(eloop_now(loop)),
           LLU(eloop_now_hrtime(loop)));
}

void on_stdin(eio_t* io, void* buf, int readbytes) {
    printf("on_stdin fd=%d readbytes=%d\n", eio_fd(io), readbytes);
    printf("> %s\n", (char*)buf);
    if (strmatch((char*)buf, "quit")) {
        // if (strncmp((char*)buf, "quit", 4) == 0) {
        eloop_stop(event_loop(io));
    }
}

void on_recvfrom(eio_t* io, void* buf, int readbytes) {
    printf("on_recvfrom fd=%d readbytes=%d\n", eio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
           SOCKADDR_STR(eio_localaddr(io), localaddrstr),
           SOCKADDR_STR(eio_peeraddr(io), peeraddrstr));

    char* str = (char*)buf;
    printf("< %.*s", readbytes, str);
}

void on_recvfrom2(eio_t* io, void* buf, int readbytes) {
    printf("on_recvfrom2 fd=%d readbytes=%d\n", eio_fd(io), readbytes);
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printf("[%s] <=> [%s]\n",
           SOCKADDR_STR(eio_localaddr(io), localaddrstr),
           SOCKADDR_STR(eio_peeraddr(io), peeraddrstr));

    char* str = (char*)buf;
    printf("< %.*s", readbytes, str);
}

int main(int argc, char* argv[]) {

    printf("%s", DEFAULT_MOTD);
    // memcheck atexit
    MEMCHECK;

    eloop_t* loop = eloop_new(0);

    // test idle and priority
    // for (int i = EVENT_LOWEST_PRIORITY; i <= EVENT_HIGHEST_PRIORITY; ++i) {
    //     eidle_t* idle = eidle_add(loop, on_idle, 1); // repeate times: 1
    //     event_set_priority(idle, i);
    // }

    // test timer timeout
    printf("now: %llu\n", LLU(eloop_now(loop)));
    for (int i = 1; i <= 3; ++i) {
        etimer_t* timer = etimer_add(loop, on_timer, i * 1000, 2);
        event_set_userdata(timer, (void*)(intptr_t)i);
    }

    // test nonblock stdin
    printf("input 'quit' to quit loop\n");
    char buf[64];
    hread(loop, 0, buf, sizeof(buf), on_stdin);

    // test io
    eio_t* io = eloop_create_udp_server(loop, ANYADDR, 520);

    // sockopt_broadcast(sock);
    // sockopt_reuseaddr(sock);
    // sockopt_reuseport(sock);
    // setsockopt_ipv4_multicast_loop(sock, 0);
    int on = 1;
    int val = 0;
    udp_broadcast(eio_fd(io), on);
    so_reuseaddr(eio_fd(io), on);
    so_reuseport(eio_fd(io), on);
    setsockopt(eio_fd(io), IPPROTO_IP, IP_MULTICAST_LOOP, (void *)&val, sizeof(val));
    eio_setcb_read(io, on_recvfrom);

    eio_read(io);

    // eio_t* io2 = eloop_create_udp_server(loop, "192.168.50.12", 520);
    // eio_setcb_read(io2, on_recvfrom2);
    // eio_read(io2);

    eloop_run(loop);
    eloop_free(&loop);
    return 0;
}
