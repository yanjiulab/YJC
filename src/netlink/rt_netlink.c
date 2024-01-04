#include "rt_netlink.h"

#include "ethernet.h"
#include "if.h"
#include "network.h"
#include "nl_debug.h"
#include "prefix.h"

/* convert the netlink multicast group number into a bit map */
/* (  e.g. 4 => 16, 5 => 32  ) */
static uint32_t nl_mgrp(uint32_t group) {
    if (group > 31) {
        printf("Netlink: Use setsockopt() for this group: %d\n", group);
        return 0;
    }
    return (group ? (1 << (group - 1)) : 0);
}

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

// void netlink_parse_rtattr(struct rtattr** tb, int max, struct rtattr* rta,
//                           int len) {
//     memset(tb, 0, sizeof(struct rtattr*) * (max + 1));
//     while (RTA_OK(rta, len)) {
//         if (rta->rta_type <= max)
//             tb[rta->rta_type] = rta;
//         rta = RTA_NEXT(rta, len);
//     }
// }

// void netlink_parse_rtattr_flags(struct rtattr** tb, int max, struct rtattr* rta,
//                                 int len, unsigned short flags) {
//     unsigned short type;

//     memset(tb, 0, sizeof(struct rtattr*) * (max + 1));
//     while (RTA_OK(rta, len)) {
//         type = rta->rta_type & ~flags;
//         if ((type <= max) && (!tb[type]))
//             tb[type] = rta;
//         rta = RTA_NEXT(rta, len);
//     }
// }

// /**
//  * netlink_parse_rtattr_nested() - Parses a nested route attribute
//  * @tb:         Pointer to array for storing rtattr in.
//  * @max:        Max number to store.
//  * @rta:        Pointer to rtattr to look for nested items in.
//  */
// void netlink_parse_rtattr_nested(struct rtattr** tb, int max,
//                                  struct rtattr* rta) {
//     netlink_parse_rtattr(tb, max, RTA_DATA(rta), RTA_PAYLOAD(rta));
// }

#define ROUTE_START

/* Request for specific route information from the kernel */
int netlink_request_route(struct nlsock* nl, int family, int type) {
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

    return netlink_request(nl, &req);
}

int netlink_route_read(struct nlsock* nl) {
    int ret;

    /* Get IPv4 routing table. */
    ret = netlink_request_route(nl, AF_INET, RTM_GETROUTE);
    if (ret < 0)
        return ret;
    ret = netlink_parse_info(nl, netlink_route_change);
    if (ret < 0)
        return ret;

    /* Get IPv6 routing table. */
    ret = netlink_request_route(nl, AF_INET6, RTM_GETROUTE);
    if (ret < 0)
        return ret;
    ret = netlink_parse_info(nl, netlink_route_change);
    if (ret < 0)
        return ret;

    return 0;
}

int netlink_route_add(struct nlsock* nl, struct ipaddr* dst, int dst_mlen,
                      ipaddr_t* gw, int ifidx, int default_gw) {
    return netlink_route_update(nl, RTM_NEWROUTE, dst, dst_mlen, gw, ifidx, RT_METRIC_DEFAULT,
                                default_gw, RT_TABLE_MAIN, RTPROT_STATIC,
                                RT_SCOPE_UNIVERSE, RTN_UNICAST,
                                0, 0);
}

int netlink_route_del(struct nlsock* nl, struct ipaddr* dst, int dst_mlen,
                      ipaddr_t* gw, int ifidx, int default_gw) {
    return netlink_route_update(nl, RTM_DELROUTE, dst, dst_mlen, gw, ifidx, 0,
                                default_gw, RT_TABLE_MAIN, RTPROT_UNSPEC,
                                RT_SCOPE_UNIVERSE, RTN_UNSPEC,
                                0, 0);
}

int netlink_route_update(struct nlsock* nl, int cmd,
                         struct ipaddr* dst, int dst_mlen, ipaddr_t* gw,
                         int ifidx, int metric, int default_gw,
                         int table, int proto, int scope, int type,
                         vrf_id_t vrf, ns_id_t ns_id) {
    struct {
        struct nlmsghdr n;
        struct rtmsg rtm;
        char buf[256];
    } req;

    /* Initialize request structure */
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_type = cmd;
    req.n.nlmsg_flags =
        cmd == RTM_NEWROUTE ? NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL : NLM_F_REQUEST;
    req.rtm.rtm_family = dst->address_family;
    req.rtm.rtm_dst_len = dst_mlen;
    // req.rtm.rtm_src_len;
    // req.rtm.rtm_tos;
    req.rtm.rtm_table = table;    /* Routing table id */
    req.rtm.rtm_protocol = proto; /* Routing protocol */
    req.rtm.rtm_scope = scope;
    req.rtm.rtm_type = type;
    // req.rtm.rtm_flags;

    unsigned char addr_bitlen = dst->address_family == AF_INET ? 32 : 128;

    /* Set gateway */
    if (gw) {
        netlink_rtattr_add(&req.n, sizeof(req), RTA_GATEWAY, &gw->ip, addr_bitlen / 8);
        req.rtm.rtm_scope = 0;
        req.rtm.rtm_family = gw->address_family;
    }

    /* Don't set destination and interface in case of default gateways */
    if (default_gw) {
        req.rtm.rtm_dst_len = 0;
    } else {
        /* Set destination network */
        netlink_rtattr_add(&req.n, sizeof(req), RTA_DST, &dst->ip, addr_bitlen / 8);

        /* Set interface */
        netlink_rtattr_add(&req.n, sizeof(req), RTA_OIF, &ifidx, sizeof(int));
    }

    /* Set metric */
    netlink_rtattr_add(&req.n, sizeof(req), RTA_PRIORITY, &metric, sizeof(int));

    return netlink_talk(nl, &req, netlink_route_change);
}

static int netlink_route_change_read_unicast(struct nlmsghdr* h, ns_id_t ns_id) {
    int len;
    struct rtmsg* rtm;
    struct rtattr* tb[RTA_MAX + 1];
    uint32_t flags = 0;
    struct prefix p;
    struct prefix_ipv6 src_p = {};
    vrf_id_t vrf_id;
    bool selfroute;

    char anyaddr[16] = {0};

    // int proto = ZEBRA_ROUTE_KERNEL;
    int index = 0;
    int table;
    int metric = 0;
    uint32_t mtu = 0;
    uint8_t distance = 0;
    // route_tag_t tag = 0;
    uint32_t nhe_id = 0;

    void* dest = NULL;
    void* gate = NULL;
    void* prefsrc = NULL; /* IPv4 preferred source host address */
    void* src = NULL;     /* IPv6 srcdest   source prefix */
    // enum blackhole_type bh_type = BLACKHOLE_UNSPEC;
    char buf[256];

    char* dump = NULL;
    size_t size = 0;
    FILE* s = open_memstream(&dump, &size); /* output string */

    rtm = NLMSG_DATA(h);

    // switch (rtm->rtm_type) {
    // case RTN_UNICAST:
    //     break;
    // // case RTN_BLACKHOLE:
    // //     bh_type = BLACKHOLE_NULL;
    // //     break;
    // // case RTN_UNREACHABLE:
    // //     bh_type = BLACKHOLE_REJECT;
    // //     break;
    // // case RTN_PROHIBIT:
    // //     bh_type = BLACKHOLE_ADMINPROHIB;
    // //     break;
    // default:
    //     log_debug("Route rtm_type: %s(%d) intentionally ignoring",
    //               nl_rttype_to_str(rtm->rtm_type),
    //               rtm->rtm_type);
    //     return 0;
    // }

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
    if (len < 0) {
        log_error("%s: Message received from netlink is of a broken size %d %zu",
                  __func__, h->nlmsg_len,
                  (size_t)NLMSG_LENGTH(sizeof(struct rtmsg)));
        return -1;
    }

    netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

    // if (rtm->rtm_flags & RTM_F_CLONED)
    //     return 0;
    // if (rtm->rtm_protocol == RTPROT_REDIRECT)
    //     return 0;
    // if (rtm->rtm_protocol == RTPROT_KERNEL)
    //     return 0;

    /* We don't care about change notifications for the MPLS table. */
    /* TODO: Revisit this. */
    if (rtm->rtm_family == AF_MPLS)
        return 0;

    /* Table corresponding to route. */
    if (tb[RTA_TABLE])
        table = *(int*)RTA_DATA(tb[RTA_TABLE]);
    else
        table = rtm->rtm_table;

    //     /* Map to VRF */
    //     vrf_id = 0;
    //     // vrf_id = vrf_lookup_by_table(table, ns_id);
    //     // if (vrf_id == VRF_DEFAULT) {
    //     //     if (!is_zebra_valid_kernel_table(table) && !is_zebra_main_routing_table(table))
    //     //         return 0;
    //     // }

    //     if (rtm->rtm_flags & RTM_F_TRAP)
    //         flags |= ZEBRA_FLAG_TRAPPED;
    //     if (rtm->rtm_flags & RTM_F_OFFLOAD)
    //         flags |= ZEBRA_FLAG_OFFLOADED;
    //     if (rtm->rtm_flags & RTM_F_OFFLOAD_FAILED)
    //         flags |= ZEBRA_FLAG_OFFLOAD_FAILED;

    //     if (h->nlmsg_flags & NLM_F_APPEND)
    //         flags |= ZEBRA_FLAG_OUTOFSYNC;

    //     /* Route which inserted by Zebra. */
    //     if (selfroute) {
    //         flags |= ZEBRA_FLAG_SELFROUTE;
    //         proto = proto2zebra(rtm->rtm_protocol, rtm->rtm_family, false);
    //     }
    if (tb[RTA_DST]) {
        dest = RTA_DATA(tb[RTA_DST]);
        fprintf(s, "%s/%u",
                inet_ntop(rtm->rtm_family, dest, buf, sizeof(buf)),
                rtm->rtm_dst_len);
    } else if (rtm->rtm_dst_len) {
        fprintf(s, "0/%u", rtm->rtm_dst_len);
    } else {
        dest = anyaddr;
        fprintf(s, "default");
    }

    if (tb[RTA_GATEWAY]) {
        gate = RTA_DATA(tb[RTA_GATEWAY]);
        fprintf(s, " via %s", inet_ntop(rtm->rtm_family, gate, buf, sizeof(buf)));
    }

    if (tb[RTA_OIF]) {
        char if_nam_buf[IF_NAMESIZE];
        index = *(int*)RTA_DATA(tb[RTA_OIF]);
        fprintf(s, " dev %s", if_indextoname(index, if_nam_buf));
    }

    if (rtm->rtm_protocol) {
        fprintf(s, " proto %s", rtm_protocol2str(rtm->rtm_protocol));
    }

    fprintf(s, " scope %s", rtm_scope2str(rtm->rtm_scope));

    if (tb[RTA_SRC]) {
        src = RTA_DATA(tb[RTA_SRC]);
        fprintf(s, " src %s", inet_ntop(rtm->rtm_family, src, buf, sizeof(buf)));
    } else {
        src = anyaddr;
    }

    if (tb[RTA_PREFSRC]) {
        prefsrc = RTA_DATA(tb[RTA_PREFSRC]);
        fprintf(s, " src %s", inet_ntop(rtm->rtm_family, prefsrc, buf, sizeof(buf)));
    } else {
        src = anyaddr;
    }

    if (tb[RTA_NH_ID]) {
        nhe_id = *(uint32_t*)RTA_DATA(tb[RTA_NH_ID]);
        fprintf(s, " nh_id %d", nhe_id);
    }

    if (tb[RTA_PRIORITY]) {
        metric = *(int*)RTA_DATA(tb[RTA_PRIORITY]);
        fprintf(s, " metric %d", metric);
    }

    fputc('\n', s);

    fclose(s);
    printf("%s", dump);

    // #if defined(SUPPORT_REALMS)
    //     if (tb[RTA_FLOW])
    //         tag = *(uint32_t*)RTA_DATA(tb[RTA_FLOW]);
    // #endif

    // if (tb[RTA_METRICS]) {
    //     struct rtattr* mxrta[RTAX_MAX + 1];

    //     netlink_parse_rtattr(mxrta, RTAX_MAX, RTA_DATA(tb[RTA_METRICS]),
    //                          RTA_PAYLOAD(tb[RTA_METRICS]));

    //     if (mxrta[RTAX_MTU])
    //         mtu = *(uint32_t*)RTA_DATA(mxrta[RTAX_MTU]);
    // }

    //     if (rtm->rtm_family == AF_INET) {
    //         p.family = AF_INET;
    //         if (rtm->rtm_dst_len > IPV4_MAX_BITLEN) {
    //             log_error(
    //                 "Invalid destination prefix length: %u received from kernel route change",
    //                 rtm->rtm_dst_len);
    //             return -1;
    //         }
    //         memcpy(&p.u.prefix4, dest, 4);
    //         p.prefixlen = rtm->rtm_dst_len;

    //         if (rtm->rtm_src_len != 0) {
    //             flog_warn(
    //                 EC_ZEBRA_UNSUPPORTED_V4_SRCDEST,
    //                 "unsupported IPv4 sourcedest route (dest %pFX vrf %u)",
    //                 &p, vrf_id);
    //             return 0;
    //         }

    //         /* Force debug below to not display anything for source */
    //         src_p.prefixlen = 0;
    //     } else if (rtm->rtm_family == AF_INET6) {
    //         p.family = AF_INET6;
    //         if (rtm->rtm_dst_len > IPV6_MAX_BITLEN) {
    //             log_error(
    //                 "Invalid destination prefix length: %u received from kernel route change",
    //                 rtm->rtm_dst_len);
    //             return -1;
    //         }
    //         memcpy(&p.u.prefix6, dest, 16);
    //         p.prefixlen = rtm->rtm_dst_len;

    //         src_p.family = AF_INET6;
    //         if (rtm->rtm_src_len > IPV6_MAX_BITLEN) {
    //             log_error(
    //                 "Invalid source prefix length: %u received from kernel route change",
    //                 rtm->rtm_src_len);
    //             return -1;
    //         }
    //         memcpy(&src_p.prefix, src, 16);
    //         src_p.prefixlen = rtm->rtm_src_len;
    //     } else {
    //         /* We only handle the AFs we handle... */
    //         if (IS_ZEBRA_DEBUG_KERNEL)
    //             zlog_debug("%s: unknown address-family %u", __func__,
    //                        rtm->rtm_family);
    //         return 0;
    //     }

    //     /*
    //      * For ZEBRA_ROUTE_KERNEL types:
    //      *
    //      * The metric/priority of the route received from the kernel
    //      * is a 32 bit number.  We are going to interpret the high
    //      * order byte as the Admin Distance and the low order 3 bytes
    //      * as the metric.
    //      *
    //      * This will allow us to do two things:
    //      * 1) Allow the creation of kernel routes that can be
    //      *    overridden by zebra.
    //      * 2) Allow the old behavior for 'most' kernel route types
    //      *    if a user enters 'ip route ...' v4 routes get a metric
    //      *    of 0 and v6 routes get a metric of 1024.  Both of these
    //      *    values will end up with a admin distance of 0, which
    //      *    will cause them to win for the purposes of zebra.
    //      */
    //     if (proto == ZEBRA_ROUTE_KERNEL) {
    //         distance = (metric >> 24) & 0xFF;
    //         metric = (metric & 0x00FFFFFF);
    //     }

    //     if (IS_ZEBRA_DEBUG_KERNEL) {
    //         char buf2[PREFIX_STRLEN];

    //         zlog_debug(
    //             "%s %pFX%s%s vrf %s(%u) table_id: %u metric: %d Admin Distance: %d",
    //             nl_msg_type_to_str(h->nlmsg_type), &p,
    //             src_p.prefixlen ? " from " : "",
    //             src_p.prefixlen ? prefix2str(&src_p, buf2, sizeof(buf2))
    //                             : "",
    //             vrf_id_to_name(vrf_id), vrf_id, table, metric,
    //             distance);
    //     }

    //     afi_t afi = AFI_IP;
    //     if (rtm->rtm_family == AF_INET6)
    //         afi = AFI_IP6;

    //     if (h->nlmsg_type == RTM_NEWROUTE) {
    //         struct route_entry* re;
    //         struct nexthop_group* ng = NULL;

    //         re = zebra_rib_route_entry_new(vrf_id, proto, 0, flags, nhe_id,
    //                                        table, metric, mtu, distance,
    //                                        tag);
    //         if (!nhe_id)
    //             ng = nexthop_group_new();

    //         if (!tb[RTA_MULTIPATH]) {
    //             struct nexthop *nexthop, nh;

    //             if (!nhe_id) {
    //                 nh = parse_nexthop_unicast(
    //                     ns_id, rtm, tb, bh_type, index, prefsrc,
    //                     gate, afi, vrf_id);

    //                 nexthop = nexthop_new();
    //                 *nexthop = nh;
    //                 nexthop_group_add_sorted(ng, nexthop);
    //             }
    //         } else {
    //             /* This is a multipath route */
    //             struct rtnexthop* rtnh =
    //                 (struct rtnexthop*)RTA_DATA(tb[RTA_MULTIPATH]);

    //             if (!nhe_id) {
    //                 uint8_t nhop_num;

    //                 /* Use temporary list of nexthops; parse
    //                  * message payload's nexthops.
    //                  */
    //                 nhop_num =
    //                     parse_multipath_nexthops_unicast(
    //                         ns_id, ng, rtm, rtnh, tb,
    //                         prefsrc, vrf_id);

    //                 zserv_nexthop_num_warn(
    //                     __func__, (const struct prefix*)&p,
    //                     nhop_num);

    //                 if (nhop_num == 0) {
    //                     nexthop_group_delete(&ng);
    //                     ng = NULL;
    //                 }
    //             }
    //         }
    //         if (nhe_id || ng) {
    //             dplane_rib_add_multipath(afi, SAFI_UNICAST, &p, &src_p,
    //                                      re, ng, startup, ctx);
    //             if (ng)
    //                 nexthop_group_delete(&ng);
    //         } else {
    //             /*
    //              * I really don't see how this is possible
    //              * but since we are testing for it let's
    //              * let the end user know why the route
    //              * that was just received was swallowed
    //              * up and forgotten
    //              */
    //             log_error(
    //                 "%s: %pFX multipath RTM_NEWROUTE has a invalid nexthop group from the kernel",
    //                 __func__, &p);
    //             XFREE(MTYPE_RE, re);
    //         }
    //     } else {
    //         if (ctx) {
    //             log_error(
    //                 "%s: %pFX RTM_DELROUTE received but received a context as well",
    //                 __func__, &p);
    //             return 0;
    //         }

    //         if (nhe_id) {
    //             rib_delete(afi, SAFI_UNICAST, vrf_id, proto, 0, flags,
    //                        &p, &src_p, NULL, nhe_id, table, metric,
    //                        distance, true);
    //         } else {
    //             if (!tb[RTA_MULTIPATH]) {
    //                 struct nexthop nh;

    //                 nh = parse_nexthop_unicast(
    //                     ns_id, rtm, tb, bh_type, index, prefsrc,
    //                     gate, afi, vrf_id);
    //                 rib_delete(afi, SAFI_UNICAST, vrf_id, proto, 0,
    //                            flags, &p, &src_p, &nh, 0, table,
    //                            metric, distance, true);
    //             } else {
    //                 /* XXX: need to compare the entire list of
    //                  * nexthops here for NLM_F_APPEND stupidity */
    //                 rib_delete(afi, SAFI_UNICAST, vrf_id, proto, 0,
    //                            flags, &p, &src_p, NULL, 0, table,
    //                            metric, distance, true);
    //             }
    //         }
    //     }

    //     return 1;
}

static int netlink_route_change_read_multicast(struct nlmsghdr* h, ns_id_t ns_id) {
    int len;
    struct rtmsg* rtm;
    struct rtattr* tb[RTA_MAX + 1];
    // struct mcast_route_data* m;
    int iif = 0;
    int count;
    int oif[256];
    int oif_count = 0;
    char oif_list[256] = "\0";
    vrf_id_t vrf;
    int table;
    void* src = NULL;
    void* grp = NULL;
    struct rta_mfc_stats* mfc_stats;

    // assert(mroute);
    // m = mroute;

    char buf[256];
    char* dump = NULL;
    size_t size = 0;
    FILE* s = open_memstream(&dump, &size); /* output string */

    rtm = NLMSG_DATA(h);

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));

    netlink_parse_rtattr(tb, RTA_MAX, RTM_RTA(rtm), len);

    if (tb[RTA_TABLE])
        table = *(int*)RTA_DATA(tb[RTA_TABLE]);
    else
        table = rtm->rtm_table;

    // vrf = vrf_lookup_by_table(table, ns_id);
    if (tb[RTA_SRC]) {
        src = RTA_DATA(tb[RTA_SRC]);
        if (rtm->rtm_family == RTNL_FAMILY_IPMR) {
            fprintf(s, "(" NIP4_FMT ", ", NIP4(*(struct in_addr*)src));
        } else {
            fprintf(s, "(" NIP6_FMT ", ", NIP6(*(struct in6_addr*)src));
        }
    } else {
        fprintf(s, "(*, ");
    }

    if (tb[RTA_DST]) {
        grp = RTA_DATA(tb[RTA_DST]);
        if (rtm->rtm_family == RTNL_FAMILY_IPMR) {
            fprintf(s, NIP4_FMT ")", NIP4(*(struct in_addr*)grp));
        } else {
            fprintf(s, NIP6_FMT ")", NIP6(*(struct in6_addr*)grp));
        }
    } else {
        fprintf(s, "*)");
    }

    if (tb[RTA_IIF]) {
        char if_nam_buf[IF_NAMESIZE];
        iif = *(int*)RTA_DATA(tb[RTA_IIF]);
        fprintf(s, " Iif: %s", if_indextoname(iif, if_nam_buf));
    }

    if (tb[RTA_MULTIPATH]) {
        struct rtnexthop* rtnh = (struct rtnexthop*)RTA_DATA(tb[RTA_MULTIPATH]);

        len = RTA_PAYLOAD(tb[RTA_MULTIPATH]);
        for (;;) {
            if (len < (int)sizeof(*rtnh) || rtnh->rtnh_len > len)
                break;

            oif[oif_count] = rtnh->rtnh_ifindex;
            oif_count++;

            if (rtnh->rtnh_len == 0)
                break;

            len -= NLMSG_ALIGN(rtnh->rtnh_len);
            rtnh = RTNH_NEXT(rtnh);
        }
    }

    for (count = 0; count < oif_count; count++) {
        // ifp = if_lookup_by_index(oif[count], vrf);
        char t[16];
        char temp[256];
        if_indextoname(oif[count], t);
        snprintf(temp, sizeof(temp), "%s(%d) ", oif[count] ? t : "Unknown", oif[count]);
        strlcat(oif_list, temp, sizeof(oif_list));
    }

    if (oif_count) {
        fprintf(s, " Oifs: %s", oif_list);
    } else {
        fprintf(s, " Oifs: null");
    }

    if (tb[RTA_MFC_STATS]) {
        mfc_stats = (struct rta_mfc_stats*)RTA_DATA(tb[RTA_MFC_STATS]);
        fprintf(s, " State: pkts=%ju bytes=%ju wrong_if=%ju",
                (uintmax_t)mfc_stats->mfcs_packets,
                (uintmax_t)mfc_stats->mfcs_bytes,
                (uintmax_t)mfc_stats->mfcs_wrong_if);
    }

    if (tb[RTA_EXPIRES]) {
        fprintf(s, " expires=%lld", *(unsigned long long*)RTA_DATA(tb[RTA_EXPIRES]));
    }

    fputc('\n', s);
    fclose(s);
    printf("%s", dump);

    return 0;

    // if (tb[RTA_SRC]) {
    //     if (rtm->rtm_family == RTNL_FAMILY_IPMR)
    //         m->src.ipaddr_v4 =
    //             *(struct in_addr*)RTA_DATA(tb[RTA_SRC]);
    //     else
    //         m->src.ipaddr_v6 =
    //             *(struct in6_addr*)RTA_DATA(tb[RTA_SRC]);
    // }

    // if (tb[RTA_DST]) {
    //     if (rtm->rtm_family == RTNL_FAMILY_IPMR)
    //         m->grp.ipaddr_v4 =
    //             *(struct in_addr*)RTA_DATA(tb[RTA_DST]);
    //     else
    //         m->grp.ipaddr_v6 =
    //             *(struct in6_addr*)RTA_DATA(tb[RTA_DST]);
    // }

    // if (tb[RTA_EXPIRES])
    //     m->lastused = *(unsigned long long*)RTA_DATA(tb[RTA_EXPIRES]);

    // if (rtm->rtm_family == RTNL_FAMILY_IPMR) {
    //     SET_IPADDR_V4(&m->src);
    //     SET_IPADDR_V4(&m->grp);
    // } else if (rtm->rtm_family == RTNL_FAMILY_IP6MR) {
    //     SET_IPADDR_V6(&m->src);
    //     SET_IPADDR_V6(&m->grp);
    // } else {
    //     zlog_warn("%s: Invalid rtm_family received", __func__);
    //     return 0;
    // }

    // if (IS_ZEBRA_DEBUG_KERNEL) {
    //     struct interface* ifp = NULL;
    //     struct zebra_vrf* zvrf = NULL;

    //     for (count = 0; count < oif_count; count++) {
    //         ifp = if_lookup_by_index(oif[count], vrf);
    //         char temp[256];

    //         snprintf(temp, sizeof(temp), "%s(%d) ",
    //                  ifp ? ifp->name : "Unknown", oif[count]);
    //         strlcat(oif_list, temp, sizeof(oif_list));
    //     }
    //     zvrf = zebra_vrf_lookup_by_id(vrf);
    //     ifp = if_lookup_by_index(iif, vrf);
    //     zlog_debug(
    //         "MCAST VRF: %s(%d) %s (%pIA,%pIA) IIF: %s(%d) OIF: %s jiffies: %lld",
    //         zvrf_name(zvrf), vrf, nl_msg_type_to_str(h->nlmsg_type),
    //         &m->src, &m->grp, ifp ? ifp->name : "Unknown", iif,
    //         oif_list, m->lastused);
    // }
}

int netlink_route_change(struct nlmsghdr* h, ns_id_t ns_id) {
    int len;
    struct rtmsg* rtm;

    rtm = NLMSG_DATA(h);

    if (!(h->nlmsg_type == RTM_NEWROUTE || h->nlmsg_type == RTM_DELROUTE)) {
        /* If this is not route add/delete message print warning. */
        log_warn("Kernel message: %s NS %u", nl_msg_type_to_str(h->nlmsg_type), ns_id);
        return 0;
    }

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct rtmsg));
    if (len < 0) {
        log_error("%s: Message received from netlink is of a broken size: %d %zu",
                  __func__, h->nlmsg_len, (size_t)NLMSG_LENGTH(sizeof(struct rtmsg)));
        return -1;
    }

    log_debug("%s %s %s table %s netns %u",
              nl_msg_type_to_str(h->nlmsg_type),
              nl_family_to_str(rtm->rtm_family),
              nl_rttype_to_str(rtm->rtm_type),
              nl_rttable_to_str(rtm->rtm_table), ns_id);

    switch (rtm->rtm_family) {
    case AF_INET:
    case AF_INET6:
        /* unicast routing table contains some entries with multicast type,
         * but these are "magic" kernel-managed *unicast* routes used for
         * outputting locally generated multicast traffic (which uses unicast
         * handling on Linux).
         */
        // if (rtm->rtm_type == RTN_MULTICAST) {
        //     return 0;
        // } else {
        return netlink_route_change_read_unicast(h, ns_id);
        // }
    case RTNL_FAMILY_IPMR:
    case RTNL_FAMILY_IP6MR:
        /* typically we consider these entries as the "real" multicast routing. */
        return netlink_route_change_read_multicast(h, ns_id);
    default:
        log_warn("Invalid address family: %u received from kernel route change: %s",
                 rtm->rtm_family, nl_msg_type_to_str(h->nlmsg_type));
        return 0;
    }
}

#define MAC_START

/* Request for MAC FDB information from the kernel */
int netlink_request_macs(struct nlsock* nl, int family, int type, ifindex_t master_ifindex) {
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

    return netlink_request(nl, &req);
}

int netlink_macfdb_table(struct nlmsghdr* h) {
    struct ndmsg* ndm;

    struct interface* ifp;
    struct zebra_if* zif;
    struct rtattr* tb[NDA_MAX + 1];
    struct interface* br_if;
    ethaddr_t mac;
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
        printf(" lladdr %s", ether_mac2str(&mac, NULL, 0));
    }

    if (tb[NDA_NH_ID]) {
        nhg_id = *(uint32_t*)RTA_DATA(tb[NDA_NH_ID]);
        printf(" nh_id %x", nhg_id);
    }

    printf(" state %u", ndm->ndm_state);

    printf("\n");
}
/**
 * MAC forwarding database read using netlink interface.
 * ==> bridge fdb
 */
int netlink_macfdb_read(struct nlsock* nl) {
    int ret;

    /* Get bridge FDB table. */
    ret = netlink_request_macs(nl, AF_BRIDGE, RTM_GETNEIGH, 0);
    if (ret < 0)
        return ret;
    /* We are reading entire table. */
    ret = netlink_parse_info(nl, netlink_macfdb_table);

    return ret;
}

int netlink_macfdb_update() {
    // TODO
}

int netlink_macfdb_change(struct nlmsghdr* h, int len, ns_id_t ns_id) {
    // struct ndmsg* ndm;
    // struct interface* ifp;
    // struct zebra_if* zif;
    // struct rtattr* tb[NDA_MAX + 1];
    // struct interface* br_if;
    // struct ethaddr mac;
    // vlanid_t vid = 0;
    // struct in_addr vtep_ip;
    // int vid_present = 0, dst_present = 0;
    // char vid_buf[20];
    // char dst_buf[30];
    // bool sticky;
    // bool local_inactive = false;
    // bool dp_static = false;
    // vni_t vni = 0;
    // uint32_t nhg_id = 0;
    // bool vni_mcast_grp = false;

    // ndm = NLMSG_DATA(h);

    // /* We only process macfdb notifications if EVPN is enabled */
    // if (!is_evpn_enabled())
    //     return 0;

    // /* Parse attributes and extract fields of interest. Do basic
    //  * validation of the fields.
    //  */
    // netlink_parse_rtattr_flags(tb, NDA_MAX, NDA_RTA(ndm), len,
    //                            NLA_F_NESTED);

    // if (!tb[NDA_LLADDR]) {
    //     if (IS_ZEBRA_DEBUG_KERNEL)
    //         zlog_debug("%s AF_BRIDGE IF %u - no LLADDR",
    //                    nl_msg_type_to_str(h->nlmsg_type),
    //                    ndm->ndm_ifindex);
    //     return 0;
    // }

    // if (RTA_PAYLOAD(tb[NDA_LLADDR]) != ETH_ALEN) {
    //     if (IS_ZEBRA_DEBUG_KERNEL)
    //         zlog_debug(
    //             "%s AF_BRIDGE IF %u - LLADDR is not MAC, len %lu",
    //             nl_msg_type_to_str(h->nlmsg_type), ndm->ndm_ifindex,
    //             (unsigned long)RTA_PAYLOAD(tb[NDA_LLADDR]));
    //     return 0;
    // }

    // memcpy(&mac, RTA_DATA(tb[NDA_LLADDR]), ETH_ALEN);

    // if (tb[NDA_VLAN]) {
    //     vid_present = 1;
    //     vid = *(uint16_t*)RTA_DATA(tb[NDA_VLAN]);
    //     snprintf(vid_buf, sizeof(vid_buf), " VLAN %u", vid);
    // }

    // if (tb[NDA_DST]) {
    //     /* TODO: Only IPv4 supported now. */
    //     dst_present = 1;
    //     memcpy(&vtep_ip.s_addr, RTA_DATA(tb[NDA_DST]),
    //            IPV4_MAX_BYTELEN);
    //     snprintfrr(dst_buf, sizeof(dst_buf), " dst %pI4",
    //                &vtep_ip);
    // } else
    //     memset(&vtep_ip, 0, sizeof(vtep_ip));

    // if (tb[NDA_NH_ID])
    //     nhg_id = *(uint32_t*)RTA_DATA(tb[NDA_NH_ID]);

    // if (ndm->ndm_state & NUD_STALE)
    //     local_inactive = true;

    // if (tb[NDA_FDB_EXT_ATTRS]) {
    //     struct rtattr* attr = tb[NDA_FDB_EXT_ATTRS];
    //     struct rtattr* nfea_tb[NFEA_MAX + 1] = {0};

    //     netlink_parse_rtattr_nested(nfea_tb, NFEA_MAX, attr);
    //     if (nfea_tb[NFEA_ACTIVITY_NOTIFY]) {
    //         uint8_t nfy_flags;

    //         nfy_flags = *(uint8_t*)RTA_DATA(
    //             nfea_tb[NFEA_ACTIVITY_NOTIFY]);
    //         if (nfy_flags & FDB_NOTIFY_BIT)
    //             dp_static = true;
    //         if (nfy_flags & FDB_NOTIFY_INACTIVE_BIT)
    //             local_inactive = true;
    //     }
    // }

    // if (tb[NDA_SRC_VNI])
    //     vni = *(vni_t*)RTA_DATA(tb[NDA_SRC_VNI]);

    // if (IS_ZEBRA_DEBUG_KERNEL)
    //     zlog_debug(
    //         "Rx %s AF_BRIDGE IF %u%s st 0x%x fl 0x%x MAC %pEA%s nhg %d vni %d",
    //         nl_msg_type_to_str(h->nlmsg_type), ndm->ndm_ifindex,
    //         vid_present ? vid_buf : "", ndm->ndm_state,
    //         ndm->ndm_flags, &mac, dst_present ? dst_buf : "",
    //         nhg_id, vni);

    // /* The interface should exist. */
    // ifp = if_lookup_by_index_per_ns(zebra_ns_lookup(ns_id),
    //                                 ndm->ndm_ifindex);
    // if (!ifp || !ifp->info)
    //     return 0;

    // /* The interface should be something we're interested in. */
    // if (!IS_ZEBRA_IF_BRIDGE_SLAVE(ifp))
    //     return 0;

    // zif = (struct zebra_if*)ifp->info;
    // if ((br_if = zif->brslave_info.br_if) == NULL) {
    //     if (IS_ZEBRA_DEBUG_KERNEL)
    //         zlog_debug(
    //             "%s AF_BRIDGE IF %s(%u) brIF %u - no bridge master",
    //             nl_msg_type_to_str(h->nlmsg_type), ifp->name,
    //             ndm->ndm_ifindex,
    //             zif->brslave_info.bridge_ifindex);
    //     return 0;
    // }

    // /* For per vni device, vni comes from device itself */
    // if (IS_ZEBRA_IF_VXLAN(ifp) && IS_ZEBRA_VXLAN_IF_VNI(zif)) {
    //     struct zebra_vxlan_vni* vnip;

    //     vnip = zebra_vxlan_if_vni_find(zif, 0);
    //     vni = vnip->vni;
    // }

    // sticky = !!(ndm->ndm_flags & NTF_STICKY);

    // if (filter_vlan && vid != filter_vlan) {
    //     if (IS_ZEBRA_DEBUG_KERNEL)
    //         zlog_debug("        Filtered due to filter vlan: %d",
    //                    filter_vlan);
    //     return 0;
    // }

    // /*
    //  * Check if this is a mcast group update (svd case)
    //  */
    // vni_mcast_grp = is_mac_vni_mcast_group(&mac, vni, vtep_ip);

    // /* If add or update, do accordingly if learnt on a "local" interface; if
    //  * the notification is over VxLAN, this has to be related to
    //  * multi-homing,
    //  * so perform an implicit delete of any local entry (if it exists).
    //  */
    // if (h->nlmsg_type == RTM_NEWNEIGH) {
    //     /* Drop "permanent" entries. */
    //     if (!vni_mcast_grp && (ndm->ndm_state & NUD_PERMANENT)) {
    //         if (IS_ZEBRA_DEBUG_KERNEL)
    //             zlog_debug(
    //                 "        Dropping entry because of NUD_PERMANENT");
    //         return 0;
    //     }

    //     if (IS_ZEBRA_IF_VXLAN(ifp)) {
    //         if (!dst_present)
    //             return 0;

    //         if (vni_mcast_grp)
    //             return zebra_vxlan_if_vni_mcast_group_add_update(
    //                 ifp, vni, &vtep_ip);

    //         return zebra_vxlan_dp_network_mac_add(
    //             ifp, br_if, &mac, vid, vni, nhg_id, sticky,
    //             !!(ndm->ndm_flags & NTF_EXT_LEARNED));
    //     }

    //     return zebra_vxlan_local_mac_add_update(ifp, br_if, &mac, vid,
    //                                             sticky, local_inactive, dp_static);
    // }

    // /* This is a delete notification.
    //  * Ignore the notification with IP dest as it may just signify that the
    //  * MAC has moved from remote to local. The exception is the special
    //  * all-zeros MAC that represents the BUM flooding entry; we may have
    //  * to readd it. Otherwise,
    //  *  1. For a MAC over VxLan, check if it needs to be refreshed(readded)
    //  *  2. For a MAC over "local" interface, delete the mac
    //  * Note: We will get notifications from both bridge driver and VxLAN
    //  * driver.
    //  */
    // if (nhg_id)
    //     return 0;

    // if (dst_present) {
    //     if (vni_mcast_grp)
    //         return zebra_vxlan_if_vni_mcast_group_del(ifp, vni,
    //                                                   &vtep_ip);

    //     if (is_zero_mac(&mac) && vni)
    //         return zebra_vxlan_check_readd_vtep(ifp, vni, vtep_ip);

    //     return 0;
    // }

    // if (IS_ZEBRA_IF_VXLAN(ifp))
    //     return 0;

    // return zebra_vxlan_local_mac_del(ifp, br_if, &mac, vid);
}

#define NEIGH_START

static int netlink_neigh_table(struct nlmsghdr* h) {
    int len;
    struct ndmsg* ndm;

    if (h->nlmsg_type != RTM_NEWNEIGH)
        return 0;

    /* Length validity. */
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ndmsg));
    if (len < 0)
        return -1;

    /* We are interested only in AF_INET or AF_INET6 notifications. */
    ndm = NLMSG_DATA(h);
    if (ndm->ndm_family != AF_INET && ndm->ndm_family != AF_INET6)
        return 0;

    // return netlink_neigh_change(h, len);
}

/* Request for IP neighbor information from the kernel */
static int netlink_request_neigh(struct nlsock* nl, int family,
                                 int type, ifindex_t ifindex) {
    struct {
        struct nlmsghdr n;
        struct ndmsg ndm;
        char buf[256];
    } req;

    /* Form the request, specifying filter (rtattr) if needed. */
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_type = type;
    req.n.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
    req.ndm.ndm_family = family;
    // if (ifindex)
    //     nl_attr_put32(&req.n, sizeof(req), NDA_IFINDEX, ifindex);

    return netlink_request(nl, &req);
}

/**
 * IP Neighbor table read using netlink interface.
 * ==> ip neigh show nud all
 */
int netlink_neigh_read(struct nlsock* nl) {
    int ret;

    /* Get IP neighbor table. */
    ret = netlink_request_neigh(nl, AF_UNSPEC, RTM_GETNEIGH, 0);
    if (ret < 0)
        return ret;
    ret = netlink_parse_info(nl, netlink_neigh_table);

    return ret;
}

int netlink_neigh_update_internal(struct nlsock* nl, int cmd, int ifindex, void* addr, char* lla,
                                  int llalen, ns_id_t ns_id, uint8_t family,
                                  bool permanent, uint8_t protocol) {
    struct {
        struct nlmsghdr n;
        struct ndmsg ndm;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ndmsg));
    req.n.nlmsg_flags = NLM_F_CREATE | NLM_F_REQUEST;
    req.n.nlmsg_type = cmd; // RTM_NEWNEIGH or RTM_DELNEIGH
    req.n.nlmsg_pid = nl->snl.nl_pid;

    req.ndm.ndm_family = family;
    req.ndm.ndm_ifindex = ifindex;
    req.ndm.ndm_type = RTN_UNICAST;
    if (cmd == RTM_NEWNEIGH) {
        if (permanent)
            req.ndm.ndm_state = NUD_PERMANENT;
        else
            req.ndm.ndm_state = NUD_REACHABLE;
    } else
        req.ndm.ndm_state = NUD_FAILED;

    nl_attr_put(&req.n, sizeof(req), NDA_PROTOCOL, &protocol, sizeof(protocol));
    req.ndm.ndm_type = RTN_UNICAST;
    nl_attr_put(&req.n, sizeof(req), NDA_DST, addr, ipaddr_length(family));
    if (lla)
        nl_attr_put(&req.n, sizeof(req), NDA_LLADDR, lla, llalen);

    // if (IS_ZEBRA_DEBUG_KERNEL) {
    // 	char ip_str[INET6_ADDRSTRLEN + 8];
    // 	struct interface *ifp = if_lookup_by_index_per_ns(
    // 		zebra_ns_lookup(ns_id), ifindex);
    // 	if (ifp) {
    // 		if (family == AF_INET6)
    // 			snprintfrr(ip_str, sizeof(ip_str), "ipv6 %pI6",
    // 				   (struct in6_addr *)addr);
    // 		else
    // 			snprintfrr(ip_str, sizeof(ip_str), "ipv4 %pI4",
    // 				   (in_addr_t *)addr);
    // 		log_debug(
    // 			"%s: %s ifname %s ifindex %u addr %s mac %pEA vrf %s(%u)",
    // 			__func__, nl_msg_type_to_str(cmd), ifp->name,
    // 			ifindex, ip_str, (struct ethaddr *)lla,
    // 			vrf_id_to_name(ifp->vrf->vrf_id),
    // 			ifp->vrf->vrf_id);
    // 	}
    // }

    char ip_str[INET6_ADDRSTRLEN + 8] = {0};
    if (family == AF_INET) {
        snprintf(ip_str, sizeof(ip_str), "ipv4 " NIP4_FMT, NIP4(*(struct in_addr*)addr));
    } else {
        snprintf(ip_str, sizeof(ip_str), "ipv6 " NIP6_FMT, NIP6(*(struct in6_addr*)addr));
    }
    char if_name[IF_NAMESIZE];
    if_indextoname(ifindex, if_name);
    log_debug("%s: %s dev %s ifindex %u addr %s lladdr %s vrf %s(%u)", __func__, nl_msg_type_to_str(cmd),
              if_name, ifindex, ip_str, ether_mac2str((struct ethaddr*)lla, NULL, 0), "null", 0);

    return netlink_talk(nl, &req.n, NULL);
}

// TODO: update ifindex_t
int netlink_neigh_update(struct nlsock* nl, int cmd, ipaddr_t ip, ethaddr_t mac, const char* ifname,
                         bool permanent, uint8_t protocol, ns_id_t ns_id) {
    // if index
    int ifindex = if_nametoindex(ifname);
    if (ip.address_family == AF_INET) {
        netlink_neigh_update_internal(nl, cmd, ifindex, &(ip.ip.v4), mac.octet, ETH_ALEN,
                                      ns_id, ip.address_family, permanent, protocol);
    } else {
        netlink_neigh_update_internal(nl, cmd, ifindex, &(ip.ip.v6), mac.octet, ETH_ALEN,
                                      ns_id, ip.address_family, permanent, protocol);
    }
}

int netlink_ipneigh_change(struct nlmsghdr* h, int len, ns_id_t ns_id) {
    // 	struct ndmsg *ndm;
    // 	struct interface *ifp;
    // 	struct zebra_if *zif;
    // 	struct rtattr *tb[NDA_MAX + 1];
    // 	struct interface *link_if;
    // 	struct ethaddr mac;
    // 	struct ipaddr ip;
    // 	char buf[ETHER_ADDR_STRLEN];
    // 	int mac_present = 0;
    // 	bool is_ext;
    // 	bool is_router;
    // 	bool local_inactive;
    // 	uint32_t ext_flags = 0;
    // 	bool dp_static = false;
    // 	int l2_len = 0;
    // 	int cmd;

    // 	ndm = NLMSG_DATA(h);

    // 	/* The interface should exist. */
    // 	// ifp = if_lookup_by_index_per_ns(zebra_ns_lookup(ns_id),
    // 	// 				ndm->ndm_ifindex);
    // 	// if (!ifp || !ifp->info)
    // 	// 	return 0;

    // 	// zif = (struct zebra_if *)ifp->info;

    // 	/* Parse attributes and extract fields of interest. */
    // 	netlink_parse_rtattr(tb, NDA_MAX, NDA_RTA(ndm), len);

    // 	if (!tb[NDA_DST]) {
    // 		log_debug("%s family %s IF %s(%u) vrf %s(%u) - no DST",
    // 			   nl_msg_type_to_str(h->nlmsg_type),
    // 			   nl_family_to_str(ndm->ndm_family), ifp->name,
    // 			   ndm->ndm_ifindex, ifp->vrf->name, ifp->vrf->vrf_id);
    // 		return 0;
    // 	}

    // 	memset(&ip, 0, sizeof(ip));
    // 	ip.ipa_type = (ndm->ndm_family == AF_INET) ? IPADDR_V4 : IPADDR_V6;
    // 	memcpy(&ip.ip.addr, RTA_DATA(tb[NDA_DST]), RTA_PAYLOAD(tb[NDA_DST]));

    // 	/* if kernel deletes our rfc5549 neighbor entry, re-install it */
    // 	if (h->nlmsg_type == RTM_DELNEIGH && (ndm->ndm_state & NUD_PERMANENT)) {
    // 		netlink_handle_5549(ndm, zif, ifp, &ip, false);
    // 		if (IS_ZEBRA_DEBUG_KERNEL)
    // 			log_debug(
    // 				"    Neighbor Entry Received is a 5549 entry, finished");
    // 		return 0;
    // 	}

    // 	/* if kernel marks our rfc5549 neighbor entry invalid, re-install it */
    // 	if (h->nlmsg_type == RTM_NEWNEIGH && !(ndm->ndm_state & NUD_VALID))
    // 		netlink_handle_5549(ndm, zif, ifp, &ip, true);

    // 	/* we send link layer information to client:
    // 	 * - nlmsg_type = RTM_DELNEIGH|NEWNEIGH|GETNEIGH
    // 	 * - struct ipaddr ( for DEL and GET)
    // 	 * - struct ethaddr mac; (for NEW)
    // 	 */
    // 	if (h->nlmsg_type == RTM_NEWNEIGH)
    // 		cmd = ZEBRA_NHRP_NEIGH_ADDED;
    // 	else if (h->nlmsg_type == RTM_GETNEIGH)
    // 		cmd = ZEBRA_NHRP_NEIGH_GET;
    // 	else if (h->nlmsg_type == RTM_DELNEIGH)
    // 		cmd = ZEBRA_NHRP_NEIGH_REMOVED;
    // 	else {
    // 		log_debug("%s(): unknown nlmsg type %u", __func__,
    // 			   h->nlmsg_type);
    // 		return 0;
    // 	}
    // 	if (tb[NDA_LLADDR]) {
    // 		/* copy LLADDR information */
    // 		l2_len = RTA_PAYLOAD(tb[NDA_LLADDR]);
    // 	}
    // 	if (l2_len == IPV4_MAX_BYTELEN || l2_len == 0) {
    // 		union sockunion link_layer_ipv4;

    // 		if (l2_len) {
    // 			sockunion_family(&link_layer_ipv4) = AF_INET;
    // 			memcpy((void *)sockunion_get_addr(&link_layer_ipv4),
    // 			       RTA_DATA(tb[NDA_LLADDR]), l2_len);
    // 		} else
    // 			sockunion_family(&link_layer_ipv4) = AF_UNSPEC;
    // 		zsend_nhrp_neighbor_notify(
    // 			cmd, ifp, &ip,
    // 			netlink_nbr_entry_state_to_zclient(ndm->ndm_state),
    // 			&link_layer_ipv4);
    // 	}

    // 	if (h->nlmsg_type == RTM_GETNEIGH)
    // 		return 0;

    // 	/* The neighbor is present on an SVI. From this, we locate the
    // 	 * underlying
    // 	 * bridge because we're only interested in neighbors on a VxLAN bridge.
    // 	 * The bridge is located based on the nature of the SVI:
    // 	 * (a) In the case of a VLAN-aware bridge, the SVI is a L3 VLAN
    // 	 * interface
    // 	 * and is linked to the bridge
    // 	 * (b) In the case of a VLAN-unaware bridge, the SVI is the bridge
    // 	 * interface
    // 	 * itself
    // 	 */
    // 	if (IS_ZEBRA_IF_VLAN(ifp)) {
    // 		link_if = if_lookup_by_index_per_ns(zebra_ns_lookup(ns_id),
    // 						    zif->link_ifindex);
    // 		if (!link_if)
    // 			return 0;
    // 	} else if (IS_ZEBRA_IF_BRIDGE(ifp))
    // 		link_if = ifp;
    // 	else {
    // 		link_if = NULL;
    // 		if (IS_ZEBRA_DEBUG_KERNEL)
    // 			log_debug(
    // 				"    Neighbor Entry received is not on a VLAN or a BRIDGE, ignoring");
    // 	}

    // 	memset(&mac, 0, sizeof(mac));
    // 	if (h->nlmsg_type == RTM_NEWNEIGH) {
    // 		if (tb[NDA_LLADDR]) {
    // 			if (RTA_PAYLOAD(tb[NDA_LLADDR]) != ETH_ALEN) {
    // 				if (IS_ZEBRA_DEBUG_KERNEL)
    // 					log_debug(
    // 						"%s family %s IF %s(%u) vrf %s(%u) - LLADDR is not MAC, len %lu",
    // 						nl_msg_type_to_str(
    // 							h->nlmsg_type),
    // 						nl_family_to_str(
    // 							ndm->ndm_family),
    // 						ifp->name, ndm->ndm_ifindex,
    // 						ifp->vrf->name,
    // 						ifp->vrf->vrf_id,
    // 						(unsigned long)RTA_PAYLOAD(
    // 							tb[NDA_LLADDR]));
    // 				return 0;
    // 			}

    // 			mac_present = 1;
    // 			memcpy(&mac, RTA_DATA(tb[NDA_LLADDR]), ETH_ALEN);
    // 		}

    // 		is_ext = !!(ndm->ndm_flags & NTF_EXT_LEARNED);
    // 		is_router = !!(ndm->ndm_flags & NTF_ROUTER);

    // 		if (tb[NDA_EXT_FLAGS]) {
    // 			ext_flags = *(uint32_t *)RTA_DATA(tb[NDA_EXT_FLAGS]);
    // 			if (ext_flags & NTF_E_MH_PEER_SYNC)
    // 				dp_static = true;
    // 		}

    // 		if (IS_ZEBRA_DEBUG_KERNEL)
    // 			log_debug(
    // 				"Rx %s family %s IF %s(%u) vrf %s(%u) IP %pIA MAC %s state 0x%x flags 0x%x ext_flags 0x%x",
    // 				nl_msg_type_to_str(h->nlmsg_type),
    // 				nl_family_to_str(ndm->ndm_family), ifp->name,
    // 				ndm->ndm_ifindex, ifp->vrf->name,
    // 				ifp->vrf->vrf_id, &ip,
    // 				mac_present
    // 					? prefix_mac2str(&mac, buf, sizeof(buf))
    // 					: "",
    // 				ndm->ndm_state, ndm->ndm_flags, ext_flags);

    // 		/* If the neighbor state is valid for use, process as an add or
    // 		 * update
    // 		 * else process as a delete. Note that the delete handling may
    // 		 * result
    // 		 * in re-adding the neighbor if it is a valid "remote" neighbor.
    // 		 */
    // 		if (ndm->ndm_state & NUD_VALID) {
    // 			if (zebra_evpn_mh_do_adv_reachable_neigh_only())
    // 				local_inactive =
    // 					!(ndm->ndm_state & NUD_LOCAL_ACTIVE);
    // 			else
    // 				/* If EVPN-MH is not enabled we treat STALE
    // 				 * neighbors as locally-active and advertise
    // 				 * them
    // 				 */
    // 				local_inactive = false;

    // 			/* Add local neighbors to the l3 interface database */
    // 			if (is_ext)
    // 				zebra_neigh_del(ifp, &ip);
    // 			else
    // 				zebra_neigh_add(ifp, &ip, &mac);

    // 			if (link_if)
    // 				zebra_vxlan_handle_kernel_neigh_update(
    // 					ifp, link_if, &ip, &mac, ndm->ndm_state,
    // 					is_ext, is_router, local_inactive,
    // 					dp_static);
    // 			return 0;
    // 		}

    // 		zebra_neigh_del(ifp, &ip);
    // 		if (link_if)
    // 			zebra_vxlan_handle_kernel_neigh_del(ifp, link_if, &ip);
    // 		return 0;
    // 	}

    // 	if (IS_ZEBRA_DEBUG_KERNEL)
    // 		log_debug("Rx %s family %s IF %s(%u) vrf %s(%u) IP %pIA",
    // 			   nl_msg_type_to_str(h->nlmsg_type),
    // 			   nl_family_to_str(ndm->ndm_family), ifp->name,
    // 			   ndm->ndm_ifindex, ifp->vrf->name, ifp->vrf->vrf_id,
    // 			   &ip);

    // 	/* Process the delete - it may result in re-adding the neighbor if it is
    // 	 * a valid "remote" neighbor.
    // 	 */
    // 	zebra_neigh_del(ifp, &ip);
    // 	if (link_if)
    // 		zebra_vxlan_handle_kernel_neigh_del(ifp, link_if, &ip);

    // 	return 0;
}

int netlink_neigh_change(struct nlmsghdr* h, ns_id_t ns_id) {
    int len;
    struct ndmsg* ndm;

    if (!(h->nlmsg_type == RTM_NEWNEIGH || h->nlmsg_type == RTM_DELNEIGH || h->nlmsg_type == RTM_GETNEIGH))
        return 0;

    /* Length validity. */
    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(struct ndmsg));
    if (len < 0) {
        log_error("%s: Message received from netlink is of a broken size %d %zu",
                  __func__, h->nlmsg_len, (size_t)NLMSG_LENGTH(sizeof(struct ndmsg)));
        return -1;
    }

    /* Is this a notification for the MAC FDB or IP neighbor table? */
    ndm = NLMSG_DATA(h);
    if (ndm->ndm_family == AF_BRIDGE)
        return netlink_macfdb_change(h, len, ns_id);

    if (ndm->ndm_type != RTN_UNICAST)
        return 0;

    if (ndm->ndm_family == AF_INET || ndm->ndm_family == AF_INET6)
        return netlink_ipneigh_change(h, len, ns_id);
    else {
        log_warn("Invalid address family: %u received from kernel neighbor change: %s",
                 ndm->ndm_family, nl_msg_type_to_str(h->nlmsg_type));
        return 0;
    }

    return 0;
}