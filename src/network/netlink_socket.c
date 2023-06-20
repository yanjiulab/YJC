#include "netlink_socket.h"

#include <net/if.h>

#include "ethernet.h"
#include "inet.h"
#include "log.h"
#include "ip_address.h"

static void netlink_socket_setup(struct netlink_socket *nlsock, int nl_type) {
    nlsock->nl_fd = socket(PF_NETLINK, SOCK_RAW, nl_type);  // NETLINK_ROUTE
    if (nlsock->nl_fd < 0) {
        perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_XX)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK;
    // saddr.nl_pid = getpid();

    nlsock->snl = saddr;

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
    if (nlsock->nl_fd >= 0)
        close(nlsock->nl_fd);

    if (nlsock->name != NULL)
        free(nlsock->name);

    memset(nlsock, 0, sizeof(*nlsock)); /* paranoia to catch bugs*/
    free(nlsock);
}

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 *
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
static ssize_t netlink_sendmsg(struct netlink_socket *nl, void *buf,
                               size_t buflen) {
    struct sockaddr_nl snl = {};
    struct iovec iov = {};
    struct msghdr msg = {};
    ssize_t status;
    int save_errno = 0;

    iov.iov_base = buf;
    iov.iov_len = buflen;
    msg.msg_name = &snl;
    msg.msg_namelen = sizeof(snl);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    snl.nl_family = AF_NETLINK;

    /* Send message to netlink interface. */
    status = sendmsg(nl->nl_fd, &msg, 0);
    save_errno = errno;
    // print_data(buf, buflen);
    if (status == -1) {
        printf("error: %s", strerror(errno));
        return -1;
    }

    return status;
}

/*
 * netlink_recvmsg - receive a netlink message.
 *
 * Returns -1 on error, 0 if read would block or the number of bytes received.
 */
static int netlink_recvmsg(int fd, struct msghdr *msg, char **answer) {
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

/* Issue request message to kernel via netlink socket. GET messages
 * are issued through this interface.
 */
int netlink_request(struct netlink_socket *nlsock, void *req) {
    struct nlmsghdr *n = (struct nlmsghdr *)req;

    /* Check netlink socket. */
    if (nlsock->nl_fd < 0) {
        // error
        return -1;
    }

    /* Fill common fields for all requests. */
    n->nlmsg_pid = nlsock->snl.nl_pid;
    n->nlmsg_seq = ++nlsock->seq;

    if (netlink_sendmsg(nlsock, req, n->nlmsg_len) == -1)
        return -1;

    return 0;
}

void netlink_parse_rtattr_flags(struct rtattr **tb, int max, struct rtattr *rta,
                                int len, unsigned short flags) {
    unsigned short type;

    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while (RTA_OK(rta, len)) {
        type = rta->rta_type & ~flags;
        if ((type <= max) && (!tb[type]))
            tb[type] = rta;
        rta = RTA_NEXT(rta, len);
    }
}

void netlink_parse_rtattr(struct rtattr **tb, int max, struct rtattr *rta,
                          int len) {
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while (RTA_OK(rta, len)) {
        if (rta->rta_type <= max)
            tb[rta->rta_type] = rta;
        rta = RTA_NEXT(rta, len);
    }
}

/**
 * netlink_parse_rtattr_nested() - Parses a nested route attribute
 * @tb:         Pointer to array for storing rtattr in.
 * @max:        Max number to store.
 * @rta:        Pointer to rtattr to look for nested items in.
 */
void netlink_parse_rtattr_nested(struct rtattr **tb, int max,
                                 struct rtattr *rta) {
    netlink_parse_rtattr(tb, max, RTA_DATA(rta), RTA_PAYLOAD(rta));
}

/* Request for specific route information from the kernel */
int netlink_request_route(struct netlink_socket *nlsock, int family, int type) {
    struct {
        struct nlmsghdr n;
        struct rtmsg rtm;
    } req;

    /* Form the request, specifying filter (rtattr) if needed. */
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_type = type;
    req.n.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.rtm.rtm_family = family;

    return netlink_request(nlsock, &req);
}

/* Request for MAC FDB information from the kernel */
int netlink_request_macs(struct netlink_socket *nlsock, int family, int type) {
    struct {
        struct nlmsghdr n;
        struct ifinfomsg ifm;
        char buf[256];
    } req;

    /* Form the request, specifying filter (rtattr) if needed. */
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_type = type;
    req.n.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.ifm.ifi_family = family;
    // if (master_ifindex)
    //     nl_attr_put32(&req.n, sizeof(req), IFLA_MASTER, master_ifindex);

    return netlink_request(nlsock, &req);
}

/******************** parse ******************************/
int netlink_rtm_parse_route(struct nlmsghdr *nl_header_answer) {
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

    netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);

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
            "%s/%u",
            inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_DST]), buf, sizeof(buf)),
            r->rtm_dst_len);

    } else if (r->rtm_dst_len) {
        printf("0/%u", r->rtm_dst_len);
    } else {
        printf("default");
    }

    if (tb[RTA_GATEWAY]) {
        printf(" via %s", inet_ntop(r->rtm_family, RTA_DATA(tb[RTA_GATEWAY]),
                                    buf, sizeof(buf)));
    }

    if (tb[RTA_OIF]) {
        char if_nam_buf[IF_NAMESIZE];
        int ifidx = *(__u32 *)RTA_DATA(tb[RTA_OIF]);

        printf(" dev %s", if_indextoname(ifidx, if_nam_buf));
    }

    if (r->rtm_protocol) {
        printf(" proto %d", r->rtm_protocol);
    }

    printf(" scope %d", r->rtm_scope);
    // printf(" type %d", r->rtm_type); // unicast

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

    return 0;
}

int netlink_macfdb_table(struct nlmsghdr *h) {
    // print_data(h, h->nlmsg_len);
    // return 0;

    struct ndmsg *ndm;

    struct interface *ifp;
    struct zebra_if *zif;
    struct rtattr *tb[NDA_MAX + 1];
    struct interface *br_if;
    ether_addr_t mac;
    // vlanid_t vid = 0;
    struct in_addr vtep_ip;
    int vid_present = 0, dst_present = 0;
    char vid_buf[20];
    char dst_buf[30];
    bool sticky;
    bool local_inactive = false;
    bool dp_static = false;
    uint32_t nhg_id = 0;

    ndm = NLMSG_DATA(h);
    int len = h->nlmsg_len;
    len -= NLMSG_LENGTH(sizeof(*ndm));

    /* Parse attributes and extract fields of interest. Do basic
     * validation of the fields.
     */

    netlink_parse_rtattr_flags(tb, NDA_MAX, NDA_RTA(ndm), len,
                               NLA_F_NESTED);
    if (tb[NDA_DST]) {
        /* TODO: Only IPv4 supported now. */
        dst_present = 1;
        memcpy(&vtep_ip.s_addr, RTA_DATA(tb[NDA_DST]),
               4);
        char ipstr[ADDR_STR_LEN];
        inet_ntop(AF_INET, &vtep_ip, ipstr, ADDR_STR_LEN);
        printf("%s", ipstr);
    }

    char ifname[IF_NAMESIZE];
    printf(" dev %s", if_indextoname(ndm->ndm_ifindex, ifname));

    if (tb[NDA_LLADDR]) {
        memcpy(&mac, RTA_DATA(tb[NDA_LLADDR]), ETH_ALEN);
        printf(" lladdr %s", ether_addr_to_string(&mac, NULL));
    }

    // if (tb[NDA_NH_ID]) {
    //     nhg_id = *(uint32_t *)RTA_DATA(tb[NDA_NH_ID]);
    //     printf(" nh_id %x", nhg_id);
    // }

    printf(" state %u", ndm->ndm_state);

    printf("\n");
}

int netlink_parse_info(struct netlink_socket *nlsock,
                       int (*filter)(struct nlmsghdr *)) {
    int status;
    int error;
    struct sockaddr_nl nladdr;
    struct iovec iov;
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    char *buf;  // == msg.msg_iov->iov_base
    status = netlink_recvmsg(nlsock->nl_fd, &msg, &buf);

    int msglen = status;
    struct nlmsghdr *h;

    for (h = (struct nlmsghdr *)buf; NLMSG_OK(h, msglen);
         h = NLMSG_NEXT(h, msglen)) {
        /* Finish of reading. */
        if (h->nlmsg_type == NLMSG_DONE)
            break;

        /* Error handling. */
        if (h->nlmsg_type == NLMSG_ERROR) {
            perror("netlink reported error");
            free(buf);
        }
        /*
         * The kernel is telling us that the dump request was interrupted
         * and we more than likely are out of luck and have missed data from
         * the kernel.
         */
        if (h->nlmsg_flags & NLM_F_DUMP_INTR) {
            fprintf(stderr, "Dump was interrupted\n");
            free(buf);
            return -1;
        }

        /* OK we got netlink message. */
        log_trace("type %s(%u), len=%d, seq=%u, pid=%u", "route", h->nlmsg_type,
                  h->nlmsg_len, h->nlmsg_seq, h->nlmsg_pid);

        /* Ignore messages that maybe sent from others besides the kernel */
        if (nladdr.nl_pid != 0)
            continue;

        /* Function to call to read the results */
        if (filter) {
            error = (*filter)(h);
            if (error < 0) {
                printf("filter function error\n");
                status = error;
            }
        }
    }

    free(buf);

    return status;
}
