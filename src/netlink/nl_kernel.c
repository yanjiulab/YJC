#include "nl_kernel.h"

#include "nl_debug.h"

static const struct message nlmsg_str[] = {{RTM_NEWROUTE, "RTM_NEWROUTE"},
                                           {RTM_DELROUTE, "RTM_DELROUTE"},
                                           {RTM_GETROUTE, "RTM_GETROUTE"},
                                           {RTM_NEWLINK, "RTM_NEWLINK"},
                                           {RTM_SETLINK, "RTM_SETLINK"},
                                           {RTM_DELLINK, "RTM_DELLINK"},
                                           {RTM_GETLINK, "RTM_GETLINK"},
                                           {RTM_NEWADDR, "RTM_NEWADDR"},
                                           {RTM_DELADDR, "RTM_DELADDR"},
                                           {RTM_GETADDR, "RTM_GETADDR"},
                                           {RTM_NEWNEIGH, "RTM_NEWNEIGH"},
                                           {RTM_DELNEIGH, "RTM_DELNEIGH"},
                                           {RTM_GETNEIGH, "RTM_GETNEIGH"},
                                           {RTM_NEWRULE, "RTM_NEWRULE"},
                                           {RTM_DELRULE, "RTM_DELRULE"},
                                           {RTM_GETRULE, "RTM_GETRULE"},
                                           {RTM_NEWNEXTHOP, "RTM_NEWNEXTHOP"},
                                           {RTM_DELNEXTHOP, "RTM_DELNEXTHOP"},
                                           {RTM_GETNEXTHOP, "RTM_GETNEXTHOP"},
                                           {RTM_NEWNETCONF, "RTM_NEWNETCONF"},
                                           {RTM_DELNETCONF, "RTM_DELNETCONF"},
                                           //    {RTM_NEWTUNNEL, "RTM_NEWTUNNEL"},
                                           //    {RTM_DELTUNNEL, "RTM_DELTUNNEL"},
                                           //    {RTM_GETTUNNEL, "RTM_GETTUNNEL"},
                                           {RTM_NEWQDISC, "RTM_NEWQDISC"},
                                           {RTM_DELQDISC, "RTM_DELQDISC"},
                                           {RTM_GETQDISC, "RTM_GETQDISC"},
                                           {RTM_NEWTCLASS, "RTM_NEWTCLASS"},
                                           {RTM_DELTCLASS, "RTM_DELTCLASS"},
                                           {RTM_GETTCLASS, "RTM_GETTCLASS"},
                                           {RTM_NEWTFILTER, "RTM_NEWTFILTER"},
                                           {RTM_DELTFILTER, "RTM_DELTFILTER"},
                                           {RTM_GETTFILTER, "RTM_GETTFILTER"},
                                           {RTM_NEWVLAN, "RTM_NEWVLAN"},
                                           {RTM_DELVLAN, "RTM_DELVLAN"},
                                           {RTM_GETVLAN, "RTM_GETVLAN"},
                                           {0}};
static const struct message rtproto_str[] = {
    {RTPROT_REDIRECT, "redirect"},
    {RTPROT_KERNEL, "kernel"},
    {RTPROT_BOOT, "boot"},
    {RTPROT_STATIC, "static"},
    {RTPROT_GATED, "GateD"},
    {RTPROT_RA, "router advertisement"},
    {RTPROT_MRT, "MRT"},
    {RTPROT_ZEBRA, "Zebra"},
#ifdef RTPROT_BIRD
    {RTPROT_BIRD, "BIRD"},
#endif /* RTPROT_BIRD */
    {RTPROT_MROUTED, "mroute"},
    {RTPROT_BGP, "BGP"},
    {RTPROT_OSPF, "OSPF"},
    {RTPROT_ISIS, "IS-IS"},
    {RTPROT_RIP, "RIP"},
    // {RTPROT_RIPNG, "RIPNG"},
    // {RTPROT_ZSTATIC, "static"},
    {0}};

static const struct message family_str[] = {{AF_INET, "ipv4"},
                                            {AF_INET6, "ipv6"},
                                            {AF_BRIDGE, "bridge"},
                                            {RTNL_FAMILY_IPMR, "ipv4MR"},
                                            {RTNL_FAMILY_IP6MR, "ipv6MR"},
                                            {0}};

static const struct message rttype_str[] = {{RTN_UNSPEC, "none"},
                                            {RTN_UNICAST, "unicast"},
                                            {RTN_LOCAL, "local"},
                                            {RTN_BROADCAST, "broadcast"},
                                            {RTN_ANYCAST, "anycast"},
                                            {RTN_MULTICAST, "multicast"},
                                            {RTN_BLACKHOLE, "blackhole"},
                                            {RTN_UNREACHABLE, "unreachable"},
                                            {RTN_PROHIBIT, "prohibited"},
                                            {RTN_THROW, "throw"},
                                            {RTN_NAT, "nat"},
                                            {RTN_XRESOLVE, "resolver"},
                                            {0}};

const char* nl_msg_type_to_str(uint16_t msg_type) {
    return lookup_msg(nlmsg_str, msg_type);
}

static void nl_socket_setup(struct nl_socket* nlsock, int nl_type, int groups) {
    // Netlink is a datagram-oriented service. Both SOCK_RAW and SOCK_DGRAM are valid values for socket_type.
    // However, the netlink protocol does not distinguish between datagram and raw sockets.
    nlsock->nl_fd = socket(PF_NETLINK, SOCK_RAW, nl_type);  // e.g. NETLINK_ROUTE

    if (nlsock->nl_fd < 0) {
        perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_XX)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = PF_NETLINK;
    saddr.nl_groups = groups;
    /* multiple netlink sockets will have different nl_pid */
    // saddr.nl_pid = getpid();

    nlsock->snl = saddr;
}

nl_socket_t* nl_socket_new(int nl_type, const char* nl_name, int groups) {
    struct nl_socket* nlsock = calloc(1, sizeof(struct nl_socket));
    nlsock->name = strdup(nl_name);
    nlsock->nl_fd = -1;

    nl_socket_setup(nlsock, nl_type, groups);

    if (bind(nlsock->nl_fd, (struct sockaddr*)&nlsock->snl, sizeof(nlsock->snl)) < 0) {
        perror("Failed to bind to netlink socket");
        close(nlsock->nl_fd);
        exit(EXIT_FAILURE);
    }

    return nlsock;
}

void nl_socket_free(struct nl_socket* nlsock) {
    if (nlsock->nl_fd >= 0)
        close(nlsock->nl_fd);

    if (nlsock->name != NULL)
        free(nlsock->name);

    memset(nlsock, 0, sizeof(*nlsock)); /* paranoia to catch bugs*/
    free(nlsock);
}

int nl_socket_set_groups(struct nl_socket* nlsock, int group) {
    nlsock->snl.nl_groups = group;
    return 0;
}

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 *
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
ssize_t netlink_sendmsg(struct nl_socket* nl, void* buf, size_t buflen) {
    struct sockaddr_nl snl = {};  // message destination
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
ssize_t netlink_recvmsg(int fd, struct msghdr* msg, char** answer) {
    struct iovec* iov = msg->msg_iov;
    char* buf;
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

#ifdef DEBUG_NL
    nl_dump(buf, len);
#endif  // DEBUG_NL

    return len;
}

int netlink_request(struct nl_socket* nlsock, void* req) {
    struct nlmsghdr* n = (struct nlmsghdr*)req;

    /* Check netlink socket. */
    if (nlsock->nl_fd < 0) {
        log_error("%s socket isn't active.", nlsock->name);
        return -1;
    }

    /* Fill common fields for all requests. */
    n->nlmsg_pid = nlsock->snl.nl_pid;
    n->nlmsg_seq = ++nlsock->seq;

    if (netlink_sendmsg(nlsock, req, n->nlmsg_len) == -1) {
        log_debug("Send netlink request failed.");
        return -1;
    } else {
        log_debug("Send %d bytes netlink request to kernel.", n->nlmsg_len);
        // print_data(req, n->nlmsg_len);
    }
    return 0;
}

// TODO: proc buf, move it in nlsock
int netlink_parse_info(struct nl_socket* nlsock,
                       int (*filter)(struct nlmsghdr*)) {
    int status;
    int error;
    int ret = 0;

    while (1) {
        struct sockaddr_nl nladdr;
        struct iovec iov;
        struct msghdr msg = {
            .msg_name = &nladdr,
            .msg_namelen = sizeof(nladdr),
            .msg_iov = &iov,
            .msg_iovlen = 1,
        };
        char* buf;  // == msg.msg_iov->iov_base
        status = netlink_recvmsg(nlsock->nl_fd, &msg, &buf);

        if (status == -1)
            return -1;
        else if (status == 0)
            break;

        log_debug("Recv %d bytes netlink response from kernel.", status);

        struct nlmsghdr* h;
        for (h = (struct nlmsghdr*)buf; ((status >= 0) && NLMSG_OK(h, status));
             h = NLMSG_NEXT(h, status)) {
            /* Finish of reading. */
            if (h->nlmsg_type == NLMSG_DONE)
                return ret;

            /* Error handling. */
            if (h->nlmsg_type == NLMSG_ERROR) {
                int err = netlink_parse_error(nlsock, h);
                if (err == 1) {
                    if (!(h->nlmsg_flags & NLM_F_MULTI))
                        return 0;
                    continue;
                } else
                    return err;
            }

            /*
             * The kernel is telling us that the dump request was interrupted
             * and we more than likely are out of luck and have missed data from
             * the kernel.
             */
            if (h->nlmsg_flags & NLM_F_DUMP_INTR) {
                log_error("Dump was interrupted");
            }

            /* OK we got netlink message. */
            log_debug("%s type %s(%u), len=%d, seq=%u, pid=%u",
                      nlsock->name, nl_msg_type_to_str(h->nlmsg_type),
                      h->nlmsg_type, h->nlmsg_len,
                      h->nlmsg_seq, h->nlmsg_pid);

            /* Ignore messages that maybe sent fromother actors besides the kernel */
            if (nladdr.nl_pid != 0) {
                log_debug("Ignoring message from pid %u", nladdr.nl_pid);
                continue;
            }

            /* Function to call to read the results */
            if (filter) {
                error = (*filter)(h);
                if (error < 0) {
                    log_debug("%s filter function error", nlsock->name);
                    ret = error;
                }
            }
        }

        /* After error care. */
        if (msg.msg_flags & MSG_TRUNC) {
            log_error("%s error: message truncated", nlsock->name);
            continue;
        }
        if (status) {
            log_error("%s error: data remnant size %d", nlsock->name, status);
            return -1;
        }
        free(buf);
    }

    return ret;
}

int netlink_parse_error(const struct nl_socket* nlsock, struct nlmsghdr* h) {
    struct nlmsgerr* err = (struct nlmsgerr*)NLMSG_DATA(h);
    int errnum = err->error;
    int msg_type = err->msg.nlmsg_type;

    if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
        log_error("%s error: message truncated", nlsock->name);
        return -1;
    }

    /*
     * Parse the extended information before we actually handle it. At this
     * point in time we do not do anything other than report the issue.
     */
    if (h->nlmsg_flags & NLM_F_ACK_TLVS) {
        // netlink_parse_extended_ack(h);
    }

    /* If the error field is zero, then this is an ACK. */
    if (err->error == 0) {
        log_debug("%s ACK: type=%s(%u), seq=%u, pid=%u",
                  nlsock->name, nl_msg_type_to_str(err->msg.nlmsg_type),
                  err->msg.nlmsg_type, err->msg.nlmsg_seq,
                  err->msg.nlmsg_pid);
        return 1;
    }

    /*
     * Deal with errors that occur because of races in link handling
     * or types are not supported in kernel.
     */
    if (((msg_type == RTM_DELROUTE && (-errnum == ENODEV || -errnum == ESRCH)) ||
         (msg_type == RTM_NEWROUTE && (-errnum == ENETDOWN || -errnum == EEXIST)) ||
         (-errnum == EOPNOTSUPP))) {
        log_debug("%s: error: %s type=%s(%u), seq=%u, pid=%u",
                  nlsock->name, strerror(-errnum),
                  nl_msg_type_to_str(msg_type), msg_type,
                  err->msg.nlmsg_seq, err->msg.nlmsg_pid);
        return 0;
    }

    /*
     * We see RTM_DELNEIGH when shutting down an interface with an IPv4
     * link-local.  The kernel should have already deleted the neighbor so
     * do not log these as an error.
     */
    if (msg_type == RTM_DELNEIGH || (msg_type == RTM_NEWROUTE && (-errnum == ESRCH || -errnum == ENETUNREACH))) {
        /*
         * This is known to happen in some situations, don't log as error.
         */
        log_debug("%s error: %s, type=%s(%u), seq=%u, pid=%u",
                  nlsock->name, safe_strerror(-errnum),
                  nl_msg_type_to_str(msg_type), msg_type,
                  err->msg.nlmsg_seq, err->msg.nlmsg_pid);
    } else {
        if ((msg_type != RTM_GETNEXTHOP && msg_type != RTM_GETVLAN))
            log_error("%s error: %s, type=%s(%u), seq=%u, pid=%u",
                      nlsock->name, strerror(-errnum),
                      nl_msg_type_to_str(msg_type), msg_type,
                      err->msg.nlmsg_seq, err->msg.nlmsg_pid);
    }
}