#include "network/sockopt.h"
#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "macros.h"
#define BUF_SIZE 1024
#define PORT     12345

void test_sockopt() {
    test_sockopt_send();
    test_sockopt_recv();
}

void test_sockopt_send() {

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    printf("tos: %d\n", go_sendtos(sockfd));
    printf("ttl: %d\n", go_sendttl(sockfd));

    so_sendtos(sockfd, 33);
    so_sendttl(sockfd, 28);
    uint8_t ip_options[] = {0, 0, 0, 0}; // 示例数据
    so_options(sockfd, ip_options, sizeof(ip_options));

    printf("tos: %d\n", go_sendtos(sockfd));
    printf("ttl: %d\n", go_sendttl(sockfd));

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    dest_addr.sin_port = htons(12345);

    const char* message = "Hello, World!";
    ssize_t num_bytes = sendto(sockfd, message, strlen(message), 0, (const struct sockaddr*)&dest_addr, sizeof(dest_addr));
    printf("UDP packet sent successfully.\n");

    close(sockfd);

    return 0;
}

void test_sockopt_recv() {
    // 创建UDP套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // 设置IP_RECVTTL套接字选项
    int optval = 1;
    so_recvttl(sockfd, optval);
    so_recvtos(sockfd, optval);
    so_pktinfo(sockfd, optval);
    so_recvopts(sockfd, optval);

    // 绑定套接字到本地地址
    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PORT);
    bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

    // 接收UDP数据包
    char buffer[BUF_SIZE];
    struct msghdr* message = malloc(sizeof(struct msghdr));
    char* control_buffer = malloc(BUF_SIZE);
    int control_length = BUF_SIZE;

    struct iovec iov;
    iov.iov_base = buffer;
    iov.iov_len = BUF_SIZE;

    memset(message, 0, sizeof(*message));
    message->msg_iov = malloc(sizeof(struct iovec));
    message->msg_iov = &iov;
    message->msg_iovlen = 1;
    message->msg_control = control_buffer;
    message->msg_controllen = control_length;

    struct sockaddr_in client_addr;
    char* ttlptr;
    int ttl;
    uint8_t tos = 12;
    ssize_t num_bytes;

    num_bytes = recvmsg(sockfd, message, 0);

    printf("Received packet from %s:%d\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    printf("Received %zd bytes: %s\n", num_bytes, buffer);

    // 从辅助数据中提取TTL值
    struct cmsghdr* cmsg;
    for (cmsg = CMSG_FIRSTHDR(message); cmsg != NULL; cmsg = CMSG_NXTHDR(message, cmsg)) {
        printf("len:%d, level: %d, type: %d, data: %d\n",
               cmsg->cmsg_len, cmsg->cmsg_level, cmsg->cmsg_type, *CMSG_DATA(cmsg));

        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL) {
            ttlptr = CMSG_DATA(cmsg);
            ttl = (int)*ttlptr;
            printf("Received packet TTL: %d,\n", ttl);
        }

        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TOS) {
            tos = (int)*CMSG_DATA(cmsg);
            printf("Received packet TOS: %d\n", tos);
        }

        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
            struct in_pktinfo* pkt_info = (struct in_pktinfo*)CMSG_DATA(cmsg);
            printf("Received pktinfo: ifindex=%u, spec_dst=%x, addr=%x\n",
                   pkt_info->ipi_ifindex,
                   pkt_info->ipi_spec_dst,
                   pkt_info->ipi_addr);
        }

        if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_RECVOPTS) {
            unsigned char* ip_opts = (unsigned char*)CMSG_DATA(cmsg);
            int opt_len = cmsg->cmsg_len - CMSG_LEN(0);

            printf("Received IP options: ");
            for (int i = 0; i < opt_len; i++) {
                printf("%02X ", ip_opts[i]);
            }
            printf("\n");
        }

        // 关闭套接字
        close(sockfd);
    }
}