#include "rt_netlink.h"

int netlink_rtattr_add(struct nlmsghdr* n, int maxlen, int type,
                       const void* data, int alen) {
    int len = RTA_LENGTH(alen);
    struct rtattr* rta;

    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
        fprintf(stderr, "rtattr_add error: message exceeded bound of %d\n",
                maxlen);
        return -1;
    }

    rta = NLMSG_TAIL(n);
    rta->rta_type = type;
    rta->rta_len = len;

    if (alen) {
        memcpy(RTA_DATA(rta), data, alen);
    }

    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);

    return 0;
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

/* Request for specific route information from the kernel */
int netlink_request_route(struct nl_socket* nlsock, int family, int type) {
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

    return netlink_send_request(nlsock, &req);
}

int netlink_request_route_add(struct nl_socket* nlsock, int type,
                              struct ip_address* dst, ip_address_t* gw,
                              int default_gw, int ifidx) {
    struct {
        struct nlmsghdr n;
        struct rtmsg rtm;
        char buf[256];
    } req;

    /* Initialize request structure */
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = type;
    req.rtm.rtm_family = dst->address_family;
    req.rtm.rtm_table = RT_TABLE_MAIN;
    req.rtm.rtm_scope = RT_SCOPE_NOWHERE;

    /* Set additional flags if NOT deleting route */
    if (type != RTM_DELROUTE) {
        req.rtm.rtm_protocol = RTPROT_BOOT;
        req.rtm.rtm_type = RTN_UNICAST;
    }

    /* Select scope, for simplicity we supports here only IPv6 and IPv4 */
    if (req.rtm.rtm_family == AF_INET6) {
        req.rtm.rtm_scope = RT_SCOPE_UNIVERSE;
    } else {
        req.rtm.rtm_scope = RT_SCOPE_LINK;
    }

    unsigned char addr_bitlen = dst->address_family == AF_INET ? 32 : 128;

    /* Set destination network */
    netlink_rtattr_add(&req.n, sizeof(req), /*RTA_NEWDST*/ RTA_DST, &dst->ip,
                       addr_bitlen / 8);

    /* Set gateway */
    if (gw) {
        netlink_rtattr_add(&req.n, sizeof(req), RTA_GATEWAY, &gw->ip, 4);
        req.rtm.rtm_scope = 0;
        req.rtm.rtm_family = gw->address_family;
    }

    /* Set interface */
    netlink_rtattr_add(&req.n, sizeof(req), RTA_OIF, &ifidx, sizeof(int));

    // /* Don't set destination and interface in case of default gateways */
    // if (!default_gw) {
    //     /* Set destination network */
    //     netlink_rtattr_add(&req.n, sizeof(req), /*RTA_NEWDST*/ RTA_DST,
    //                        &dst->ip, 4);

    //     /* Set interface */
    //     netlink_rtattr_add(&req.n, sizeof(req), RTA_OIF, &ifidx,
    //     sizeof(int));
    // }

    return netlink_send_request(nlsock, &req);
}

/* Request for MAC FDB information from the kernel */
int netlink_request_macs(struct nl_socket* nlsock, int family, int type) {
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

    return netlink_send_request(nlsock, &req);
}

/******************** parse ******************************/
int netlink_rtm_parse_route(struct nlmsghdr* nl_header_answer) {
    struct rtmsg* r = NLMSG_DATA(nl_header_answer);
    int len = nl_header_answer->nlmsg_len;
    struct rtattr* tb[RTA_MAX + 1];
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
        table = *(uint32_t*)RTA_DATA(tb[RTA_TABLE]);
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
        int ifidx = *(uint32_t*)RTA_DATA(tb[RTA_OIF]);

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
        int pri = *(uint32_t*)RTA_DATA(tb[RTA_PRIORITY]);
        printf(" metric %d", pri);
    }

    printf("\n");

    return 0;
}

int netlink_macfdb_table(struct nlmsghdr* h) {
    struct ndmsg* ndm;

    struct interface* ifp;
    struct zebra_if* zif;
    struct rtattr* tb[NDA_MAX + 1];
    struct interface* br_if;
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
    netlink_parse_rtattr_flags(tb, NDA_MAX, NDA_RTA(ndm), len, NLA_F_NESTED);
    if (tb[NDA_DST]) {
        /* TODO: Only IPv4 supported now. */
        dst_present = 1;
        memcpy(&vtep_ip.s_addr, RTA_DATA(tb[NDA_DST]), 4);
        char ipstr[ADDR_STR_LEN];
        inet_ntop(AF_INET, &vtep_ip, ipstr, ADDR_STR_LEN);
        printf("%s", ipstr);
    }

    char ifname[IF_NAMESIZE];
    printf(" dev %s", if_indextoname(ndm->ndm_ifindex, ifname));

    if (tb[NDA_LLADDR]) {
        memcpy(&mac, RTA_DATA(tb[NDA_LLADDR]), ETH_ALEN);
        printf(" lladdr %s", ether_to_string(&mac, NULL));
    }

    // if (tb[NDA_NH_ID]) {
    //     nhg_id = *(uint32_t *)RTA_DATA(tb[NDA_NH_ID]);
    //     printf(" nh_id %x", nhg_id);
    // }

    printf(" state %u", ndm->ndm_state);

    printf("\n");
}

