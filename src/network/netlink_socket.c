#include "netlink_socket.h"

#include <net/if.h>

#include "inet.h"

static void netlink_socket_setup(struct netlink_socket *nlsock, int nl_type) {
    nlsock->nl_fd = socket(PF_NETLINK, SOCK_RAW, nl_type);  // NETLINK_ROUTE
    if (nlsock->nl_fd < 0) {
        perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_XX)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;
    saddr.nl_pid = getpid();

    if (bind(nlsock->nl_fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("Failed to bind to netlink socket");
        close(nlsock->nl_fd);
        exit(EXIT_FAILURE);
    }
}

netlink_socket_t *netlink_socket_new(int nl_type, const char *nl_name) {
    struct netlink_socket *nlsock = calloc(1, sizeof(struct netlink_socket));

    nlsock->name = strdup(nl_name);
    nlsock->nl_fd = -1;

    netlink_socket_setup(nlsock, nl_type);

    return nlsock;
}

void netlink_socket_free(struct netlink_socket *nlsock) {
    if (nlsock->nl_fd >= 0) close(nlsock->nl_fd);

    if (nlsock->name != NULL) free(nlsock->name);

    memset(nlsock, 0, sizeof(*nlsock)); /* paranoia to catch bugs*/
    free(nlsock);
}

/* Route */
int netlink_rt_get_req(struct netlink_socket *nlsock) {
    struct {
        struct nlmsghdr nlh;
        struct rtmsg rtm;
    } nl_request;

    nl_request.nlh.nlmsg_type = RTM_GETROUTE;
    nl_request.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nl_request.nlh.nlmsg_len = sizeof(nl_request);
    nl_request.nlh.nlmsg_seq = time(NULL);
    nl_request.rtm.rtm_family = AF_INET;

    return send(nlsock->nl_fd, &nl_request, sizeof(nl_request), 0);
}

// void netlink_rt_add_req();
// void netlink_rt_del_req();

void nlmsg_parse_route(struct nlmsghdr *nl_header_answer) {
    struct rtmsg *r = NLMSG_DATA(nl_header_answer);
    int len = nl_header_answer->nlmsg_len;
    struct rtattr *tb[RTA_MAX + 1];
    int table;
    char buf[256];

    len -= NLMSG_LENGTH(sizeof(*r));

    if (len < 0) {
        perror("Wrong message length");
        return;
    }

    memset(tb, 0, sizeof(struct rtattr *) * (RTA_MAX + 1));

    printf("proto: %d\t", r->rtm_protocol);
    printf("scope: %d\t", r->rtm_scope);
    printf("type: %d\t", r->rtm_type);

    for (struct rtattr *rta = RTM_RTA(r); RTA_OK(rta, len);
         rta = RTA_NEXT(rta, len)) {
        // printf("%d\n", rta->rta_type);
        if (rta->rta_type <= RTA_MAX) {
            tb[rta->rta_type] = rta;
        }
    }

    table = r->rtm_table;
    if (tb[RTA_TABLE]) {
        table = *(uint32_t *)RTA_DATA(tb[RTA_TABLE]);
    }

    if (r->rtm_family != AF_INET && table != RT_TABLE_MAIN) {
        return;
    }

    if (tb[RTA_DST]) {
        if ((r->rtm_dst_len != 24) && (r->rtm_dst_len != 16)) {
            return;
        }

        printf(
            "%s/%u ",
            inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_DST]), buf, sizeof(buf)),
            r->rtm_dst_len);

    } else if (r->rtm_dst_len) {
        printf("0/%u ", r->rtm_dst_len);
    } else {
        printf("default ");
    }

    if (tb[RTA_GATEWAY]) {
        printf("via %s", inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_GATEWAY]),
                                   buf, sizeof(buf)));
    }

    if (tb[RTA_OIF]) {
        char if_nam_buf[IF_NAMESIZE];
        int ifidx = *(__u32 *)RTA_DATA(tb[RTA_OIF]);

        printf(" dev %s", if_indextoname(ifidx, if_nam_buf));
    }

    if (tb[RTA_SRC]) {
        printf(" src %s", inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_SRC]), buf,
                                    sizeof(buf)));
    }

    if (tb[RTA_PREFSRC]) {
        printf(" src %s", inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_PREFSRC]),
                                    buf, sizeof(buf)));
    }

    if (tb[RTA_PRIORITY]) {
        int pri = *(__u32 *)RTA_DATA(tb[RTA_PRIORITY]);
        printf(" metric %d", pri);
    }

    printf("\n");
}

static int _netlink_recvmsg(int fd, struct msghdr *msg, char **answer) {
    struct iovec *iov = msg->msg_iov;
    char *buf;
    int len;

    iov->iov_base = NULL;
    iov->iov_len = 0;

    do {
        len = recvmsg(fd, msg, MSG_PEEK | MSG_TRUNC);
    } while (len < 0 && (errno == EINTR || errno == EAGAIN));

    if (len < 0) {
        return len;
    }

    buf = malloc(len);

    if (!buf) {
        perror("malloc failed");
        return -ENOMEM;
    }

    iov->iov_base = buf;
    iov->iov_len = len;

    do {
        len = recvmsg(fd, msg, 0);
    } while (len < 0 && (errno == EINTR || errno == EAGAIN));

    if (len < 0) {
        free(buf);
        return len;
    }

    *answer = buf;

    return len;
}

int netlink_recvmsg(struct netlink_socket *nlsock) {
    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    char *buf;
    int dump_intr = 0;

    int status = _netlink_recvmsg(nlsock->nl_fd, &msg, &buf);

    struct nlmsghdr *h;
    int msglen = status;

    struct rtmsg *rm;
    struct rtattr *rta;
    int i = 0;
    for (h = (struct nlmsghdr *)buf; NLMSG_OK(h, msglen);
         h = NLMSG_NEXT(h, msglen)) {
        // if (h->nlmsg_type == NLMSG_DONE) break;
        if (h->nlmsg_flags & NLM_F_DUMP_INTR) {
            fprintf(stderr, "Dump was interrupted\n");
            free(buf);
            return -1;
        }

        if (nladdr.nl_pid != 0) {
            continue;
        }

        if (h->nlmsg_type == NLMSG_ERROR) {
            perror("netlink reported error");
            free(buf);
        }

        nlmsg_parse_route(h);
    }

    free(buf);

    return status;
}
