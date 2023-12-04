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

int main(int argc, char* argv[]) {
    log_info("hello");
    // if (argc != 2) {
    //     return;
    // }

    int UDP_PORT = 0x2222;
    log_info("%d", UDP_PORT);

    int fd, rt;
    char recvline[1024] = {0};
    char sendline[1024] = "test test test\n";

    // 创建 UDP 套接字
    fd = socket(PF_INET, SOCK_DGRAM, 0);

    // 绑定网卡
    // struct ifreq ifr;
    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, argv[1], IF_NAMESIZE);
    // rt = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char*)&ifr, sizeof(ifr));
    // if (rt < 0) {
    //     log_info("%s", strerror(errno));
    // }

    // 设置接收端口
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = PF_INET;
    // servaddr.sin_addr.s_addr = inet_addr("192.168.50.10");
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(UDP_PORT);

    int opt = 1;
    rt = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (rt < 0) {
        log_info("reuse: %s", strerror(errno));
    }

    rt = bind(fd, &servaddr, sizeof(servaddr));
    if (rt < 0) {
        log_info("bind: %s", strerror(errno));
    }

    // 等待数据
    struct sockaddr_in cliaddr;
    socklen_t addrlen = sizeof(cliaddr);
    while (1) {
        int n = recvfrom(fd, recvline, 1024, 0, &cliaddr, &addrlen);
        log_info("RECV:");
        print_data(recvline, n);
        int s = sendto(fd, sendline, 28, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
        log_info("SEND:");
        print_data(sendline, 16);
    }
}