#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFSIZE 8192
#define IF_NAMESIZE 16
int main(int argc, char **argv) {
    int skfd;
    char buf[BUFSIZE];
    struct nlmsghdr *nlh;
    struct ifinfomsg *ifinfo;
    struct rtattr *attr;
    int len;

    // 创建Socket
    if ((skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
        perror("socket");
        return -1;
    }

    // 构造Netlink消息头
    nlh = (struct nlmsghdr *)buf;
    nlh->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    nlh->nlmsg_type = RTM_GETLINK;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = 100;
    nlh->nlmsg_pid = getpid();

    // 构造ifinfomsg消息体
    ifinfo = (struct ifinfomsg *)NLMSG_DATA(nlh);
    ifinfo->ifi_family = AF_INET;

    // 发送Netlink消息
    if (send(skfd, buf, nlh->nlmsg_len, 0) < 0) {
        perror("send");
        close(skfd);
        return -1;
    }

    // 接收Netlink消息
    while ((len = recv(skfd, buf, BUFSIZE, 0)) > 0) {
        printf("len: %d\n", len);
        for (nlh = (struct nlmsghdr *)buf; NLMSG_OK(nlh, len);
             nlh = NLMSG_NEXT(nlh, len)) {
            // 判断是否为最后一个消息
            if (nlh->nlmsg_type == NLMSG_DONE) {
                close(skfd);
                return 0;
            }
            // 判断消息类型
            if (nlh->nlmsg_type != RTM_NEWLINK) {
                continue;
            }
            // 解析ifinfomsg消息体中的属性
            ifinfo = (struct ifinfomsg *)NLMSG_DATA(nlh);
            attr = (struct rtattr *)IFLA_RTA(ifinfo);
            len = IFLA_PAYLOAD(nlh);

            char ifname[IF_NAMESIZE] = "";
            int mtu = -1;
            unsigned char mac_addr[6] = {0};

            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len)) {
                switch (attr->rta_type) {
                    case IFLA_IFNAME:
                        strncpy(ifname, (char *)RTA_DATA(attr), IF_NAMESIZE);
                        break;
                    case IFLA_MTU:
                        mtu = *(int *)RTA_DATA(attr);
                        break;
                    case IFLA_ADDRESS:
                        memcpy(mac_addr, RTA_DATA(attr), 6);
                        break;
                    default:
                        break;
                }
            }

            printf("Interface name: %s\n", ifname);
            printf("MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", mac_addr[0],
                   mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                   mac_addr[5]);
            printf("MTU: %d\n", mtu);
        }
    }

    close(skfd);
    return 0;
}