#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "log.h"
#include "network/sock.h"
#include "test.h"

#define UDP_PORT 0x2222

void test_udp() {
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
    //     log_info("%s", strerror(errno));
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