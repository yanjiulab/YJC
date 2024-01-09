#include "nl_kernel.h"

#include "nl_debug.h"

#include <sys/socket.h>

uint32_t rcvbufsize = 128 * 1024;

static const struct message nlmsg_str[] = {
    {RTM_NEWROUTE, "RTM_NEWROUTE"},
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
    {RTPROT_DHCP, "DHCP"},
    {RTPROT_KEEPALIVED, "KeepaliveD"},
    {RTPROT_MROUTED, "mroute"},
    {RTPROT_BGP, "BGP"},
    {RTPROT_ISIS, "IS-IS"},
    {RTPROT_OSPF, "OSPF"},
    {RTPROT_RIP, "RIP"},
    {RTPROT_EIGRP, "EIGRP"},
    // {RTPROT_RIPNG, "RIPNG"},
    // {RTPROT_ZSTATIC, "static"},
    {0}};

static const struct message family_str[] = {
    {AF_INET, "ipv4"},
    {AF_INET6, "ipv6"},
    {AF_BRIDGE, "bridge"},
    {RTNL_FAMILY_IPMR, "ipv4MR"},
    {RTNL_FAMILY_IP6MR, "ipv6MR"},
    {0}};

static const struct message rttype_str[] = {
    {RTN_UNSPEC, "none"},
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

static const struct message rttable_str[] = {
    {RT_TABLE_UNSPEC, "none"},
    {RT_TABLE_COMPAT, "compatibility"},
    {RT_TABLE_DEFAULT, "default"},
    {RT_TABLE_MAIN, "main"},
    {RT_TABLE_LOCAL, "local"},
    {0}};

void netlink_parse_rtattr_flags(struct rtattr** tb, int max, struct rtattr* rta,
                                int len, unsigned short flags) {
    unsigned short type;

    memset(tb, 0, sizeof(struct rtattr*) * (max + 1));
    while (RTA_OK(rta, len)) {
        type = rta->rta_type & ~flags;
        if ((type <= max) && (!tb[type]))
            tb[type] = rta;
        rta = RTA_NEXT(rta, len);
    }
}

void netlink_parse_rtattr(struct rtattr** tb, int max, struct rtattr* rta,
                          int len) {
    memset(tb, 0, sizeof(struct rtattr*) * (max + 1));
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
void netlink_parse_rtattr_nested(struct rtattr** tb, int max,
                                 struct rtattr* rta) {
    netlink_parse_rtattr(tb, max, RTA_DATA(rta), RTA_PAYLOAD(rta));
}

bool nl_addraw_l(struct nlmsghdr* n, unsigned int maxlen, const void* data,
                 unsigned int len) {
    if (NLMSG_ALIGN(n->nlmsg_len) + NLMSG_ALIGN(len) > maxlen) {
        log_error("ERROR message exceeded bound of %d", maxlen);
        return false;
    }

    memcpy(NLMSG_TAIL(n), data, len);
    memset((uint8_t*)NLMSG_TAIL(n) + len, 0, NLMSG_ALIGN(len) - len);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + NLMSG_ALIGN(len);

    return true;
}

bool nl_attr_put(struct nlmsghdr* n, unsigned int maxlen, int type,
                 const void* data, unsigned int alen) {
    int len;
    struct rtattr* rta;

    len = RTA_LENGTH(alen);

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen)
        return false;

    rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;

    if (data)
        memcpy(RTA_DATA(rta), data, alen);
    else
        assert(alen == 0);

    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);

    return true;
}

bool nl_attr_put8(struct nlmsghdr* n, unsigned int maxlen, int type,
                  uint8_t data) {
    return nl_attr_put(n, maxlen, type, &data, sizeof(uint8_t));
}

bool nl_attr_put16(struct nlmsghdr* n, unsigned int maxlen, int type,
                   uint16_t data) {
    return nl_attr_put(n, maxlen, type, &data, sizeof(uint16_t));
}

bool nl_attr_put32(struct nlmsghdr* n, unsigned int maxlen, int type,
                   uint32_t data) {
    return nl_attr_put(n, maxlen, type, &data, sizeof(uint32_t));
}

bool nl_attr_put64(struct nlmsghdr* n, unsigned int maxlen, int type,
                   uint64_t data) {
    return nl_attr_put(n, maxlen, type, &data, sizeof(uint64_t));
}

struct rtattr* nl_attr_nest(struct nlmsghdr* n, unsigned int maxlen, int type) {
    struct rtattr* nest = NLMSG_TAIL(n);

    if (!nl_attr_put(n, maxlen, type, NULL, 0))
        return NULL;

    nest->rta_type |= NLA_F_NESTED;
    return nest;
}

int nl_attr_nest_end(struct nlmsghdr* n, struct rtattr* nest) {
    nest->rta_len = (uint8_t*)NLMSG_TAIL(n) - (uint8_t*)nest;
    return n->nlmsg_len;
}

struct rtnexthop* nl_attr_rtnh(struct nlmsghdr* n, unsigned int maxlen) {
    struct rtnexthop* rtnh = (struct rtnexthop*)NLMSG_TAIL(n);

    if (NLMSG_ALIGN(n->nlmsg_len) + RTNH_ALIGN(sizeof(struct rtnexthop)) > maxlen)
        return NULL;

    memset(rtnh, 0, sizeof(struct rtnexthop));
    n->nlmsg_len =
        NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(sizeof(struct rtnexthop));

    return rtnh;
}

void nl_attr_rtnh_end(struct nlmsghdr* n, struct rtnexthop* rtnh) {
    rtnh->rtnh_len = (uint8_t*)NLMSG_TAIL(n) - (uint8_t*)rtnh;
}

bool nl_rta_put(struct rtattr* rta, unsigned int maxlen, int type,
                const void* data, int alen) {
    struct rtattr* subrta;
    int len = RTA_LENGTH(alen);

    if (RTA_ALIGN(rta->rta_len) + RTA_ALIGN(len) > maxlen) {
        log_error("ERROR max allowed bound %d exceeded for rtattr",
                  maxlen);
        return false;
    }
    subrta = (struct rtattr*)(((char*)rta) + RTA_ALIGN(rta->rta_len));
    subrta->rta_type = type;
    subrta->rta_len = len;
    if (alen)
        memcpy(RTA_DATA(subrta), data, alen);
    rta->rta_len = NLMSG_ALIGN(rta->rta_len) + RTA_ALIGN(len);

    return true;
}

bool nl_rta_put16(struct rtattr* rta, unsigned int maxlen, int type,
                  uint16_t data) {
    return nl_rta_put(rta, maxlen, type, &data, sizeof(uint16_t));
}

bool nl_rta_put64(struct rtattr* rta, unsigned int maxlen, int type,
                  uint64_t data) {
    return nl_rta_put(rta, maxlen, type, &data, sizeof(uint64_t));
}

struct rtattr* nl_rta_nest(struct rtattr* rta, unsigned int maxlen, int type) {
    struct rtattr* nest = RTA_TAIL(rta);

    if (nl_rta_put(rta, maxlen, type, NULL, 0))
        return NULL;

    nest->rta_type |= NLA_F_NESTED;

    return nest;
}

int nl_rta_nest_end(struct rtattr* rta, struct rtattr* nest) {
    nest->rta_len = (uint8_t*)RTA_TAIL(rta) - (uint8_t*)nest;

    return rta->rta_len;
}

const char* nl_msg_type_to_str(uint16_t msg_type) {
    return lookup_msg(nlmsg_str, msg_type);
}

const char* nl_rtproto_to_str(uint8_t rtproto) {
    return lookup_msg(rtproto_str, rtproto);
}

const char* nl_rttable_to_str(uint8_t rttable) {
    return lookup_msg(rttable_str, rttable);
}

const char* nl_family_to_str(uint8_t family) {
    return lookup_msg(family_str, family);
}

const char* nl_rttype_to_str(uint8_t rttype) {
    return lookup_msg(rttype_str, rttype);
}

#define NLA_OK(nla, len) \
    ((len) >= (int)sizeof(struct nlattr) && (nla)->nla_len >= sizeof(struct nlattr) && (nla)->nla_len <= (len))
#define NLA_NEXT(nla, attrlen)               \
    ((attrlen) -= NLA_ALIGN((nla)->nla_len), \
     (struct nlattr*)(((char*)(nla)) + NLA_ALIGN((nla)->nla_len)))
#define NLA_LENGTH(len) (NLA_ALIGN(sizeof(struct nlattr)) + (len))
#define NLA_DATA(nla) ((struct nlattr*)(((char*)(nla)) + NLA_LENGTH(0)))

#define ERR_NLA(err, inner_len) \
    ((struct nlattr*)(((char*)(err)) + NLMSG_ALIGN(sizeof(struct nlmsgerr)) + NLMSG_ALIGN((inner_len))))

static int netlink_recvbuf(struct nlsock* nl, uint32_t newsize) {
    uint32_t oldsize;
    socklen_t newlen = sizeof(newsize);
    socklen_t oldlen = sizeof(oldsize);
    int ret;

    ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &oldsize, &oldlen);
    if (ret < 0) {
        log_error("Can't get %s receive buffer size: %s", nl->name, strerror(errno));
        return -1;
    }

    /* Try force option (linux >= 2.6.14) and fall back to normal set */
    ret = setsockopt(nl->sock, SOL_SOCKET, SO_RCVBUFFORCE, &rcvbufsize, sizeof(rcvbufsize));
    if (ret < 0)
        ret = setsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &rcvbufsize, sizeof(rcvbufsize));
    if (ret < 0) {
        log_error("Can't set %s receive buffer size: %s", nl->name, strerror(errno));
        return -1;
    }

    ret = getsockopt(nl->sock, SOL_SOCKET, SO_RCVBUF, &newsize, &newlen);
    if (ret < 0) {
        log_error("Can't get %s receive buffer size: %s", nl->name, strerror(errno));
        return -1;
    }
    return 0;
}

static void nl_socket_setup(struct nlsock* nl, int nl_type, int groups) {
    // Netlink is a datagram-oriented service. Both SOCK_RAW and SOCK_DGRAM are valid values for socket_type.
    // However, the netlink protocol does not distinguish between datagram and raw sockets.
    nl->sock = socket(PF_NETLINK, SOCK_RAW, nl_type); // e.g. NETLINK_ROUTE

    if (nl->sock < 0) {
        perror("socket(PF_NETLINK, SOCK_RAW, NETLINK_XX)");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = PF_NETLINK;
    saddr.nl_groups = groups;
    /* multiple netlink sockets will have different nl_pid */
    // saddr.nl_pid = getpid();

    nl->snl = saddr;

    /* Register kernel socket. */
    if (fcntl(nl->sock, F_SETFL, O_NONBLOCK) < 0)
        log_error("Can't set %s socket flags: %s", nl->name, strerror(errno));

    /* Set receive buffer size if it's set from command line */
    if (rcvbufsize) {
        netlink_recvbuf(nl, rcvbufsize);
    }
}

nl_socket_t* nl_socket_new(int nl_type, const char* nl_name, int groups /*, ns_id_t ns_id*/) {
    struct nlsock* nl = calloc(1, sizeof(struct nlsock));
    nl->name = strdup(nl_name);
    nl->sock = -1;

    nl_socket_setup(nl, nl_type, groups);

    if (bind(nl->sock, (struct sockaddr*)&nl->snl, sizeof(nl->snl)) < 0) {
        perror("Failed to bind to netlink socket");
        close(nl->sock);
        exit(EXIT_FAILURE);
    }

    return nl;
}

void nl_socket_free(struct nlsock* nl) {
    if (nl->sock >= 0)
        close(nl->sock);

    if (nl->name != NULL)
        free(nl->name);

    memset(nl, 0, sizeof(*nl)); /* paranoia to catch bugs*/
    free(nl);
}

int nl_socket_set_groups(struct nlsock* nl, int group) {
    nl->snl.nl_groups = group;
    return 0;
}

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 *
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
ssize_t netlink_sendmsg(struct nlsock* nl, void* buf, size_t buflen) {
    struct sockaddr_nl snl = {}; // message destination
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
    status = sendmsg(nl->sock, &msg, 0);
    save_errno = errno;
#ifdef NETLINK_DEBUG
    log_debug("%s: >> netlink message dump [sent]", __func__);
    nl_dump(buf, buflen);
#endif // DEBUG_NL

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
ssize_t netlink_recvmsg(struct nlsock* nl, struct msghdr* msg) {

    struct iovec iov;
    int status;

    iov.iov_base = nl->buf;
    iov.iov_len = nl->buflen;
    msg->msg_iov = &iov;
    msg->msg_iovlen = 1;

    do {
        int bytes;

        bytes = recv(nl->sock, NULL, 0, MSG_PEEK | MSG_TRUNC);

        if (bytes >= 0 && (size_t)bytes > nl->buflen) {
            nl->buf = realloc(nl->buf, bytes);
            nl->buflen = bytes;
            iov.iov_base = nl->buf;
            iov.iov_len = nl->buflen;
        }

        status = recvmsg(nl->sock, msg, 0);
    } while (status == -1 && errno == EINTR);

    if (status == -1) {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        log_error("%s recvmsg overrun: %s", nl->name, strerror(errno));
        exit(-1);
    }

    if (status == 0) {
        log_error("%s EOF", nl->name);
        return -1;
    }

    if (msg->msg_namelen != sizeof(struct sockaddr_nl)) {
        log_error("%s sender address length error: length %d", nl->name, msg->msg_namelen);
        return -1;
    }

#ifdef NETLINK_DEBUG
    log_debug("%s: << netlink message dump [recv]", __func__);
    nl_dump(nl->buf, status);
#endif /* NETLINK_DEBUG */

    return status;
}

int netlink_request(struct nlsock* nl, void* req) {
    struct nlmsghdr* n = (struct nlmsghdr*)req;

    /* Check netlink socket. */
    if (nl->sock < 0) {
        log_error("%s socket isn't active.", nl->name);
        return -1;
    }

    /* Fill common fields for all requests. */
    n->nlmsg_pid = nl->snl.nl_pid;
    n->nlmsg_seq = ++nl->seq;

    if (netlink_sendmsg(nl, req, n->nlmsg_len) == -1) {
        log_error("Send netlink request failed.");
        return -1;
    } else {
        log_info("Send %d bytes netlink request to kernel.", n->nlmsg_len);
        // print_data(req, n->nlmsg_len);
    }
    return 0;
}

/*
 * Parse a netlink error message
 *
 * Returns 1 if this message is acknowledgement, 0 if this error should be
 * ignored, -1 otherwise.
 */
static int netlink_parse_error(const struct nlsock* nl, struct nlmsghdr* h) {
    struct nlmsgerr* err = (struct nlmsgerr*)NLMSG_DATA(h);
    int errnum = err->error;
    int msg_type = err->msg.nlmsg_type;

    if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr))) {
        log_error("%s error: message truncated", nl->name);
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
                  nl->name, nl_msg_type_to_str(err->msg.nlmsg_type),
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
                  nl->name, strerror(-errnum),
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
                  nl->name, strerror(-errnum),
                  nl_msg_type_to_str(msg_type), msg_type,
                  err->msg.nlmsg_seq, err->msg.nlmsg_pid);
    } else {
        if ((msg_type != RTM_GETNEXTHOP && msg_type != RTM_GETVLAN))
            log_error("%s error: %s, type=%s(%u), seq=%u, pid=%u",
                      nl->name, strerror(-errnum),
                      nl_msg_type_to_str(msg_type), msg_type,
                      err->msg.nlmsg_seq, err->msg.nlmsg_pid);
    }
}

// TODO: proc buf, move it in nlsock
int netlink_parse_info(struct nlsock* nl,
                       int (*filter)(struct nlmsghdr*)) {
    int status;
    int error;
    int ret = 0;

    while (1) {
        struct sockaddr_nl nladdr;
        struct msghdr msg = {
            .msg_name = &nladdr,
            .msg_namelen = sizeof(nladdr)};

        status = netlink_recvmsg(nl, &msg);

        if (status == -1)
            return -1;
        else if (status == 0)
            break;

        log_info("Recv %d bytes netlink response from kernel.", status);

        struct nlmsghdr* h;
        for (h = (struct nlmsghdr*)nl->buf;
             ((status >= 0) && NLMSG_OK(h, status));
             h = NLMSG_NEXT(h, status)) {
            /* Finish of reading. */
            if (h->nlmsg_type == NLMSG_DONE)
                return ret;

            /* Error handling. */
            if (h->nlmsg_type == NLMSG_ERROR) {
                int err = netlink_parse_error(nl, h);
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
                      nl->name, nl_msg_type_to_str(h->nlmsg_type),
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
                    log_debug("%s filter function error", nl->name);
                    ret = error;
                }
            }
        }

        /* After error care. */
        if (msg.msg_flags & MSG_TRUNC) {
            log_error("%s error: message truncated", nl->name);
            continue;
        }
        if (status) {
            log_error("%s error: data remnant size %d", nl->name, status);
            return -1;
        }
    }

    return ret;
}

int netlink_talk(struct nlsock* nl, struct nlmsghdr* n, int (*filter)(struct nlmsghdr*)) {

    /* Increment sequence number before capturing snapshot of ns socket
     * info.
     */
    nl->seq++;

    n->nlmsg_pid = nl->snl.nl_pid;

    log_debug("netlink_talk: %s type %s(%u), len=%d seq=%u flags 0x%x",
              nl->name, nl_msg_type_to_str(n->nlmsg_type),
              n->nlmsg_type, n->nlmsg_len, n->nlmsg_seq,
              n->nlmsg_flags);

    if (netlink_sendmsg(nl, n, n->nlmsg_len) == -1)
        return -1;

    /*
     * Get reply from netlink socket.
     * The reply should either be an acknowlegement or an error.
     */
    return netlink_parse_info(nl, filter);
}