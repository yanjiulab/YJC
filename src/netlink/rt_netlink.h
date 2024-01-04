/* rt_netlink.h */

#ifndef __RT_NETLINK_H__
#define __RT_NETLINK_H__

#include "nl_kernel.h"

#define RT_METRIC_DEFAULT 100 // specious ?
#define RT_METRIC_HIGHEST 0

#define RTM_NHA(h) \
    ((struct rtattr*)(((char*)(h)) + NLMSG_ALIGN(sizeof(struct nhmsg))))

// High-level API
int netlink_route_change(struct nlmsghdr* h, ns_id_t ns_id);

int netlink_route_read(struct nlsock* nl); // ==> ip route list table all
#define netlink_route_get netlink_route_read

int netlink_route_add(struct nlsock* nl, struct ipaddr* dst, int dst_mlen,
                      ipaddr_t* gw, int ifidx, int default_gw);

int netlink_route_del(struct nlsock* nl, struct ipaddr* dst, int dst_mlen,
                      ipaddr_t* gw, int ifidx, int default_gw);

int netlink_route_update(struct nlsock* nl, int cmd,
                         struct ipaddr* dst, int dst_mlen, ipaddr_t* gw,
                         int ifidx, int metric, int default_gw,
                         int table, int proto, int scope, int type,
                         vrf_id_t vrf, ns_id_t ns_id);

int netlink_neigh_add();
int netlink_neigh_del();
int netlink_neigh_get();
int netlink_neigh_change();
int netlink_neigh_read(struct nlsock* nl);
int netlink_neigh_update(struct nlsock* nl, int cmd, ipaddr_t ip, ethaddr_t mac, const char* ifname,
                         bool permanent, uint8_t protocol, ns_id_t ns_id);

// TODO
int netlink_macfdb_table(struct nlmsghdr* nl_header_answer);
// #define netlink_macfdb_change netlink_macfdb_table
int netlink_macfdb_read(struct nlsock* nl);
int netlink_macfdb_add();
int netlink_macfdb_del();
int netlink_macfdb_update();

// Low-level API
int netlink_request_route(struct nlsock* nl, int family, int type);
int netlink_request_macs(struct nlsock* nl, int family, int type, ifindex_t master_ifindex);

#endif