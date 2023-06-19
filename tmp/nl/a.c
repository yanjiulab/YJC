#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#define BUF_SIZE 8192

int main(int argc, char* argv[]) {
    int sock_fd;
    struct sockaddr_nl sa_nl;
    char buf[BUF_SIZE];
    struct iovec iov = {buf, sizeof(buf)};
    struct msghdr msg = {(void *)&sa_nl, sizeof(sa_nl), &iov, 1, NULL, 0, 0};

    // 创建Netlink Socket
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    // 绑定到Netlink Socket
    memset(&sa_nl, 0, sizeof(sa_nl));
    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = RTMGRP_NEIGH;
    if (bind(sock_fd, (struct sockaddr *)&sa_nl, sizeof(sa_nl)) < 0) {
        perror("bind");
        close(sock_fd);
        return -1;
    }

    // 接收Netlink消息并解析ARP表
    while (1) {
        int ret;

        ret = recvmsg(sock_fd, &msg, MSG_DONTWAIT);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            break;
        } else if (ret == 0) {
            break;
        }

        struct nlmsghdr *nl_hdr;
        for (nl_hdr = (struct nlmsghdr *)buf; NLMSG_OK(nl_hdr, ret); nl_hdr = NLMSG_NEXT(nl_hdr, ret)) {
            if (nl_hdr->nlmsg_type == RTM_NEWNEIGH || nl_hdr->nlmsg_type == RTM_DELNEIGH) {
                struct ndmsg * nd_msg;
                struct rtattr * rta_tb[NDTPA_MAX+1];
                int attrlen;
                char ip_addr[INET6_ADDRSTRLEN], mac_addr[18];

                nd_msg = (struct ndmsg *)NLMSG_DATA(nl_hdr);
                attrlen = nl_hdr->nlmsg_len - NLMSG_LENGTH(sizeof(*nd_msg));
                memset(rta_tb, 0, sizeof(struct rtattr *) * (NDTPA_MAX + 1));
                for (struct rtattr * rta = NDMSG_TAIL(nd_msg); RTA_OK(rta, attrlen); rta = RTA_NEXT(rta, attrlen)) {
                    int type = rta->rta_type & ~NLA_F_NESTED;

                    if (type <= NDTPA_UNSPEC || type > NDTPA_MAX)
                        continue;
                    rta_tb[type] = rta;
                }

                // 获取IP地址和MAC地址
                if (nl_hdr->nlmsg_type == RTM_NEWNEIGH) {
                    inet_ntop(AF_INET6, RTA_DATA(rta_tb[NDTPA_IFINDEX]), ip_addr, INET6_ADDRSTRLEN);
                    snprintf(mac_addr, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[0],
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[1],
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[2],
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[3],
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[4],
                             NDA_LLADDR(RTA_DATA(rta_tb[NDTPA_LLADDR]))[5]);
                    printf("%s %s\n", ip_addr, mac_addr);
                }
            }
        }
    }

    close(sock_fd);

    return 0;
}
