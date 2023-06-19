#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define BUFSIZE 8192
#define INET_ADDRSTRLEN 16
int main(int argc, char** argv) {
    int skfd;
    char buf[BUFSIZE];
    struct nlmsghdr *nlh;
    struct rtmsg *rtm;
    struct rtattr *attr;
    int len;

    // 创建Socket
    if ((skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
        perror("socket");
        return -1;
    }

    // 构造Netlink消息头
    nlh = (struct nlmsghdr *)buf;
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlh->nlmsg_type = RTM_GETROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = 100;
    nlh->nlmsg_pid = getpid();

    // 构造rtmsg消息体
    rtm = (struct rtmsg *)NLMSG_DATA(nlh);
    rtm->rtm_family = AF_UNSPEC;
    rtm->rtm_table = RT_TABLE_MAIN;

    // 发送Netlink消息
    if (send(skfd, buf, nlh->nlmsg_len, 0) < 0) {
        perror("send");
        close(skfd);
        return -1;
    }

    // 接收Netlink消息
    while ((len = recv(skfd, buf, BUFSIZE, 0)) > 0) {
        for (nlh = (struct nlmsghdr *)buf; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            // 判断是否为最后一个消息
            if (nlh->nlmsg_type == NLMSG_DONE) {
                close(skfd);
                return 0;
            }
            // 判断消息类型
            if (nlh->nlmsg_type != RTM_NEWROUTE) {
                continue;
            }
            // 解析rtmsg消息体中的属性
            rtm = (struct rtmsg *)NLMSG_DATA(nlh);
            attr = (struct rtattr *)RTM_RTA(rtm);
            len = RTM_PAYLOAD(nlh);

            char dst_addr[INET_ADDRSTRLEN] = "";
            char gateway[INET_ADDRSTRLEN] = "";
            unsigned int ifindex = -1;

            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
                switch (attr->rta_type) {
                    case RTA_DST:
                        inet_ntop(rtm->rtm_family, RTA_DATA(attr), dst_addr, INET_ADDRSTRLEN);
                        break;
                    case RTA_GATEWAY:
                        inet_ntop(rtm->rtm_family, RTA_DATA(attr), gateway, INET_ADDRSTRLEN);
                        break;
                    case RTA_OIF:
                        ifindex = *(unsigned int *)RTA_DATA(attr);
                        break;
                    default:
                        break;
                }
            }

            printf("Destination address: %s\n", dst_addr);
            printf("Gateway address: %s\n", gateway);
            printf("Output interface index: %d\n", ifindex);
        }
    }

    close(skfd);
    return 0;
}
