// https://gohalo.me/post/linux-libev.html
#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <linux/netlink.h>
#include <net/ethernet.h>  //For ether_header
#include <net/if.h>
#include <netdb.h>
#include <netinet/if_ether.h>  // for ETH_P_ALL
#include <netinet/ip.h>        //Provides declarations for ip header
#include <netinet/ip_icmp.h>   //Provides declarations for icmp header
#include <netinet/tcp.h>       //Provides declarations for tcp header
#include <netinet/udp.h>       //Provides declarations for udp header
#include <stdint.h>
#include <stdio.h>   //For standard things
#include <stdlib.h>  //malloc
#include <string.h>  //strlen
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "base.h"
#include "eventloop.h"
#include "inet.h"
#include "ini.h"
#include "log.h"
#include "socket.h"
#include "str.h"

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
    if (STR_EQUAL((char*)buf, "quit")) {
        // if (strncmp((char*)buf, "quit", 4) == 0) {
        eloop_stop(event_loop(io));
    }
}

int main(int argc, char* argv[]) {
    // memcheck atexit
    MEMCHECK;
    // struct in_addr i;
    // log_info("%.8x", inet_atoi_n("192.168.0.1"));
    // log_info("%.8x", inet_atoi_h("192.168.0.1"));
    // log_info("%s, %s", inet_itoa_h(0xc0a81717), inet_itoa_h(0xc0a81718));
    // log_info("%s", inet_itoa_n(0x0102a8c0));

    eloop_t* loop = eloop_new(0);

    // test idle and priority
    for (int i = EVENT_LOWEST_PRIORITY; i <= EVENT_HIGHEST_PRIORITY; ++i) {
        eidle_t* idle = eidle_add(loop, on_idle, 10);
        event_set_priority(idle, i);
    }

    // test timer timeout
    for (int i = 1; i <= 10; ++i) {
        etimer_t* timer = etimer_add(loop, on_timer, i * 10000, 3);
        event_set_userdata(timer, (void*)(intptr_t)i);
    }

    // test nonblock stdin
    printf("input 'quit' to quit loop\n");
    char buf[64];
    hread(loop, 0, buf, sizeof(buf), on_stdin);

    // test io
    eio_t* io = eloop_create_udp_server(loop, ANYADDR, 8080);

    eloop_run(loop);
    eloop_free(&loop);
    return 0;

    // log_info("hello");
    // if (argc != 2) {
    //     return;
    // }

    // int UDP_PORT = 0x2222;
    // log_info("%d", UDP_PORT);

    // int fd, rt;
    // char recvline[1024] = {0};
    // char sendline[1024] = "test test test\n";

    // // 创建 UDP 套接字
    // fd = socket(PF_INET, SOCK_DGRAM, 0);

    // // 绑定网卡
    // struct ifreq ifr;
    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, argv[1], IF_NAMESIZE);
    // rt = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char*)&ifr, sizeof(ifr));
    // if (rt < 0) {
    //     log_info("%s", strerror(errno));
    // }

    // // 设置接收端口
    // struct sockaddr_in servaddr;
    // bzero(&servaddr, sizeof(servaddr));
    // servaddr.sin_family = PF_INET;
    // // servaddr.sin_addr.s_addr = inet_addr("192.168.50.10");
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // servaddr.sin_port = htons(UDP_PORT);
    // rt = bind(fd, &servaddr, sizeof(servaddr));
    // if (rt < 0) {
    //     log_info("bind: %s", strerror(errno));
    // }

    // rt = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &rt, sizeof(rt));
    // if (rt < 0) {
    //     log_info("reuse: %s", strerror(errno));
    // }

    // 添加组
    // struct sockaddr_in grpaddr;
    // bzero(&grpaddr, sizeof(grpaddr));
    // grpaddr.sin_family = PF_INET;
    // grpaddr.sin_addr.s_addr = inet_addr("224.0.0.9");
    // memcpy(&mreq.imr_multiaddr, &grpaddr, sizeof(grpaddr));

    // struct ip_mreq mreq;
    // bzero(&mreq, sizeof(mreq));
    // mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.9");
    // mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // // ioctl(fd, SIOCGIFADDR, &ifr);
    // // memcpy(&mreq.imr_interface, &((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));

    // rt = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    // if (rt < 0) {
    //     log_info("multi: %s", strerror(errno));
    // }

    // // 等待数据
    // struct sockaddr_in cliaddr;
    // socklen_t addrlen = sizeof(cliaddr);
    // while (1) {
    //     int n = recvfrom(fd, recvline, 1024, 0, &cliaddr, &addrlen);
    //     log_info("RECV:");
    //     print_data(recvline, n);
    //     int s = sendto(fd, sendline, 28, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    //     log_info("SEND:");
    //     print_data(sendline, 16);
    // }
}