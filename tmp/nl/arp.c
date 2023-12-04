#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/rtnetlink.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

/* convert the netlink multicast group number into a bit map */
/* (  e.g. 4 => 16, 5 => 32  ) */
static uint32_t nl_mgrp(uint32_t group) {
    if (group > 31) {
        printf("Netlink: Use setsockopt() for this group: %d\n", group);

        return 0;
    }

    return (group ? (1 << (group - 1)) : 0);
}

void process_message(char *buf, int raw_msg_len) {
    struct nlmsghdr *nlmp;
    struct ndmsg *rtmp;
    struct rtattr *rtatp;
    int rtattrlen, len, req_len;
    struct in_addr *inp;
    char ipv4string[INET_ADDRSTRLEN];

    printf("process_message: raw_msg_len: %d\n", raw_msg_len);

    for (nlmp = (struct nlmsghdr *)buf; raw_msg_len > sizeof(*nlmp);) {
        int len = nlmp->nlmsg_len;
        int req_len = len - sizeof(*nlmp);

        if ((req_len < 0) || (len > raw_msg_len) ||
            (!NLMSG_OK(nlmp, raw_msg_len))) {
            printf("error\n");
            // Should processing of the message continue if there are certain
            // types of problems?
        }

        rtmp = (struct ndmsg *)NLMSG_DATA(nlmp);
        rtatp = (struct rtattr *)IFA_RTA(rtmp);

        rtattrlen = IFA_PAYLOAD(nlmp);

        for (; RTA_OK(rtatp, rtattrlen); rtatp = RTA_NEXT(rtatp, rtattrlen)) {
            if (rtatp->rta_type == NDA_DST) {
                inp = (struct in_addr *)RTA_DATA(rtatp);
                inet_ntop(AF_INET, inp, ipv4string, INET_ADDRSTRLEN);
                printf("addr: %s\n", ipv4string);
            }
        }

        raw_msg_len -= NLMSG_ALIGN(len);
        nlmp = (struct nlmsghdr *)((char *)nlmp + NLMSG_ALIGN(len));
    }

    // How is the event to be parsed?
}

int main(int argc, char **argv) {
    int sock;
    static struct sockaddr_nl g_addr;
    char buffer[4096];
    int received_bytes = 0;
    int ret;

    /* Zeroing addr */
    bzero(&g_addr, sizeof(g_addr));
    g_addr.nl_family = AF_NETLINK;
    g_addr.nl_groups = nl_mgrp(RTNLGRP_NEIGH);

    if ((sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0) {
        printf("socket() error: %s\n", strerror(errno));

        return -1;
    }

    if (bind(sock, (struct sockaddr *)&g_addr, sizeof(g_addr)) < 0) {
        printf("bind() error: %s\n", strerror(errno));

        return -1;
    }

    while (1) {
        received_bytes = recv(sock, buffer, sizeof(buffer), 0);

        if (received_bytes > 0) {
            printf("\n- Event -\n");
            process_message(buffer, received_bytes);
        }
    }
}