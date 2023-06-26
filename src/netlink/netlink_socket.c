#include "netlink_socket.h"

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
    if (nlsock->nl_fd >= 0) close(nlsock->nl_fd);

    if (nlsock->name != NULL) free(nlsock->name);

    memset(nlsock, 0, sizeof(*nlsock)); /* paranoia to catch bugs*/
    free(nlsock);
}

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 *
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
ssize_t netlink_sendmsg(struct netlink_socket *nl, void *buf, size_t buflen) {
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
ssize_t netlink_recvmsg(int fd, struct msghdr *msg, char **answer) {
    struct iovec *iov = msg->msg_iov;
    char *buf;
    ssize_t len;

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
    else
        print_data(req, n->nlmsg_len);
    return 0;
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
    print_data(buf, status);
    int msglen = status;
    struct nlmsghdr *h;

    for (h = (struct nlmsghdr *)buf; NLMSG_OK(h, msglen);
         h = NLMSG_NEXT(h, msglen)) {
        /* Finish of reading. */
        if (h->nlmsg_type == NLMSG_DONE) break;

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
        if (nladdr.nl_pid != 0) continue;

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

const char *nlmsg_type2str(uint16_t type) {
    switch (type) {
        /* Generic */
        case NLMSG_NOOP:
            return "NOOP";
        case NLMSG_ERROR:
            return "ERROR";
        case NLMSG_DONE:
            return "DONE";
        case NLMSG_OVERRUN:
            return "OVERRUN";

        /* RTM */
        case RTM_NEWLINK:
            return "NEWLINK";
        case RTM_DELLINK:
            return "DELLINK";
        case RTM_GETLINK:
            return "GETLINK";
        case RTM_SETLINK:
            return "SETLINK";

        case RTM_NEWADDR:
            return "NEWADDR";
        case RTM_DELADDR:
            return "DELADDR";
        case RTM_GETADDR:
            return "GETADDR";

        case RTM_NEWROUTE:
            return "NEWROUTE";
        case RTM_DELROUTE:
            return "DELROUTE";
        case RTM_GETROUTE:
            return "GETROUTE";

        case RTM_NEWNEIGH:
            return "NEWNEIGH";
        case RTM_DELNEIGH:
            return "DELNEIGH";
        case RTM_GETNEIGH:
            return "GETNEIGH";

        case RTM_NEWRULE:
            return "NEWRULE";
        case RTM_DELRULE:
            return "DELRULE";
        case RTM_GETRULE:
            return "GETRULE";

        case RTM_NEWNETCONF:
            return "RTM_NEWNETCONF";
        case RTM_DELNETCONF:
            return "RTM_DELNETCONF";

        default:
            return "UNKNOWN";
    }
}

// const char *nlmsg_flags2str(uint16_t flags, char *buf, size_t buflen)
// {
// 	const char *bufp = buf;

// 	*buf = 0;
// 	/* Specific flags. */
// 	flag_write(flags, NLM_F_REQUEST, "REQUEST", buf, buflen);
// 	flag_write(flags, NLM_F_MULTI, "MULTI", buf, buflen);
// 	flag_write(flags, NLM_F_ACK, "ACK", buf, buflen);
// 	flag_write(flags, NLM_F_ECHO, "ECHO", buf, buflen);
// 	flag_write(flags, NLM_F_DUMP, "DUMP", buf, buflen);

// 	/* Netlink family type dependent. */
// 	flag_write(flags, 0x0100, "(ROOT|REPLACE|CAPPED)", buf, buflen);
// 	flag_write(flags, 0x0200, "(MATCH|EXCLUDE|ACK_TLVS)", buf, buflen);
// 	flag_write(flags, 0x0400, "(ATOMIC|CREATE)", buf, buflen);
// 	flag_write(flags, 0x0800, "(DUMP|APPEND)", buf, buflen);

// 	return (bufp);
// }

// void netlink_dump(void *msg, size_t msglen) {
//     struct nlmsghdr *nlmsg = msg;
//     struct nlmsgerr *nlmsgerr;
//     struct rtgenmsg *rtgen;
//     struct ifaddrmsg *ifa;
//     struct ndmsg *ndm;
//     struct rtmsg *rtm;
//     // struct nhmsg *nhm;
//     // struct netconfmsg *ncm;
//     struct ifinfomsg *ifi;
//     // struct tunnel_msg *tnlm;
//     // struct fib_rule_hdr *frh;
//     char fbuf[128];
//     char ibuf[128];

// next_header:
//     log_debug(
//         "nlmsghdr [len=%u type=(%d) %s flags=(0x%04x) {%s} seq=%u pid=%u]",
//         nlmsg->nlmsg_len, nlmsg->nlmsg_type, nlmsg_type2str(nlmsg->nlmsg_type),
//         nlmsg->nlmsg_flags,
//         nlmsg_flags2str(nlmsg->nlmsg_flags, fbuf, sizeof(fbuf)),
//         nlmsg->nlmsg_seq, nlmsg->nlmsg_pid);

//     switch (nlmsg->nlmsg_type) {
//         /* Generic. */
//         case NLMSG_NOOP:
//             break;
//         case NLMSG_ERROR:
//             nlmsgerr = NLMSG_DATA(nlmsg);
//             log_debug("  nlmsgerr [error=(%d) %s]", nlmsgerr->error,
//                        strerror(-nlmsgerr->error));
//             break;
//         case NLMSG_DONE:
//             return;
//         case NLMSG_OVERRUN:
//             break;

//         /* RTM. */
//         case RTM_NEWLINK:
//         case RTM_DELLINK:
//         case RTM_SETLINK:
//             ifi = NLMSG_DATA(nlmsg);
//             log_debug(
//                 "  ifinfomsg [family=%d type=(%d) %s index=%d flags=0x%04x "
//                 "{%s}]",
//                 ifi->ifi_family, ifi->ifi_type, ifi_type2str(ifi->ifi_type),
//                 ifi->ifi_index, ifi->ifi_flags,
//                 if_flags2str(ifi->ifi_flags, ibuf, sizeof(ibuf)));
//             nllink_dump(ifi, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi)));
//             break;
//         case RTM_GETLINK:
//             rtgen = NLMSG_DATA(nlmsg);
//             log_debug("  rtgen [family=(%d) %s]", rtgen->rtgen_family,
//                        af_type2str(rtgen->rtgen_family));
//             break;

//         case RTM_NEWROUTE:
//         case RTM_DELROUTE:
//         case RTM_GETROUTE:
//             rtm = NLMSG_DATA(nlmsg);
//             log_debug(
//                 "  rtmsg [family=(%d) %s dstlen=%d srclen=%d tos=%d table=%d "
//                 "protocol=(%d) %s scope=(%d) %s type=(%d) %s flags=0x%04x "
//                 "{%s}]",
//                 rtm->rtm_family, af_type2str(rtm->rtm_family), rtm->rtm_dst_len,
//                 rtm->rtm_src_len, rtm->rtm_tos, rtm->rtm_table,
//                 rtm->rtm_protocol, rtm_protocol2str(rtm->rtm_protocol),
//                 rtm->rtm_scope, rtm_scope2str(rtm->rtm_scope), rtm->rtm_type,
//                 rtm_type2str(rtm->rtm_type), rtm->rtm_flags,
//                 rtm_flags2str(rtm->rtm_flags, fbuf, sizeof(fbuf)));
//             nlroute_dump(rtm, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*rtm)));
//             break;

//         case RTM_NEWNEIGH:
//         case RTM_DELNEIGH:
//             ndm = NLMSG_DATA(nlmsg);
//             log_debug(
//                 "  ndm [family=%d (%s) ifindex=%d state=0x%04x {%s} "
//                 "flags=0x%04x {%s} type=%d (%s)]",
//                 ndm->ndm_family, af_type2str(ndm->ndm_family), ndm->ndm_ifindex,
//                 ndm->ndm_state,
//                 neigh_state2str(ndm->ndm_state, ibuf, sizeof(ibuf)),
//                 ndm->ndm_flags,
//                 neigh_flags2str(ndm->ndm_flags, fbuf, sizeof(fbuf)),
//                 ndm->ndm_type, rtm_type2str(ndm->ndm_type));
//             nlneigh_dump(ndm, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*ndm)));
//             break;

//         // case RTM_NEWRULE:
//         // case RTM_DELRULE:
//         //     frh = NLMSG_DATA(nlmsg);
//         //     log_debug(
//         //         "  frh [family=%d (%s) dst_len=%d src_len=%d tos=%d table=%d "
//         //         "res1=%d res2=%d action=%d (%s) flags=0x%x]",
//         //         frh->family, af_type2str(frh->family), frh->dst_len,
//         //         frh->src_len, frh->tos, frh->table, frh->res1, frh->res2,
//         //         frh->action, frh_action2str(frh->action), frh->flags);
//         //     nlrule_dump(frh, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*frh)));
//         //     break;

//         case RTM_NEWADDR:
//         case RTM_DELADDR:
//             ifa = NLMSG_DATA(nlmsg);
//             log_debug(
//                 "  ifa [family=(%d) %s prefixlen=%d flags=0x%04x {%s} scope=%d "
//                 "index=%u]",
//                 ifa->ifa_family, af_type2str(ifa->ifa_family),
//                 ifa->ifa_prefixlen, ifa->ifa_flags,
//                 if_flags2str(ifa->ifa_flags, fbuf, sizeof(fbuf)),
//                 ifa->ifa_scope, ifa->ifa_index);
//             nlifa_dump(ifa, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*ifa)));
//             break;

//         // case RTM_NEWNEXTHOP:
//         // case RTM_DELNEXTHOP:
//         // case RTM_GETNEXTHOP:
//         //     nhm = NLMSG_DATA(nlmsg);
//         //     log_debug(
//         //         "  nhm [family=(%d) %s scope=(%d) %s protocol=(%d) %s "
//         //         "flags=0x%08x {%s}]",
//         //         nhm->nh_family, af_type2str(nhm->nh_family), nhm->nh_scope,
//         //         rtm_scope2str(nhm->nh_scope), nhm->nh_protocol,
//         //         rtm_protocol2str(nhm->nh_protocol), nhm->nh_flags,
//         //         nh_flags2str(nhm->nh_flags, fbuf, sizeof(fbuf)));
//         //     nlnh_dump(nhm, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*nhm)));
//         //     break;

//         // case RTM_NEWTUNNEL:
//         // case RTM_DELTUNNEL:
//         // case RTM_GETTUNNEL:
//         //     tnlm = NLMSG_DATA(nlmsg);
//         //     log_debug("  tnlm [family=(%d) %s ifindex=%d ", tnlm->family,
//         //                af_type2str(tnlm->family), tnlm->ifindex);
//         //     nltnl_dump(tnlm, nlmsg->nlmsg_len -
//         //                          NLMSG_LENGTH(sizeof(struct tunnel_msg)));
//         //     break;

//         // case RTM_NEWNETCONF:
//         // case RTM_DELNETCONF:
//         //     ncm = NLMSG_DATA(nlmsg);
//         //     log_debug(" ncm [family=%s (%d)]", af_type2str(ncm->ncm_family),
//         //                ncm->ncm_family);
//         //     nlncm_dump(ncm, nlmsg->nlmsg_len - NLMSG_LENGTH(sizeof(*ncm)));
//         //     break;

//         default:
//             break;
//     }

//     /*
//      * Try to get the next header. There should only be more
//      * messages if this header was flagged as MULTI, otherwise just
//      * end it here.
//      */
//     nlmsg = NLMSG_NEXT(nlmsg, msglen);
//     if (NLMSG_OK(nlmsg, msglen) == 0) return;

//     goto next_header;
// }