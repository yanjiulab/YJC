/* rt_netlink.h */

#ifndef __RT_NETLINK_H__
#define __RT_NETLINK_H__

#include "nl_kernel.h"

typedef signed int ifindex_t;

#define RTM_NHA(h) \
    ((struct rtattr*)(((char*)(h)) + NLMSG_ALIGN(sizeof(struct nhmsg))))

// High-level API

int netlink_route_read(struct nlsock* nl); // ==> ip route list table all
int netlink_route_change(struct nlsock* nl);

int netlink_macfdb_read(struct nlsock* nl);

int netlink_neigh_read(struct nlsock* nl);
int netlink_neigh_update(struct nlsock* nl, int cmd, int ifindex, void* addr, char* lla,
                         int llalen, ns_id_t ns_id, uint8_t family,
                         bool permanent, uint8_t protocol);

// Low-level API

int netlink_rtm_parse_route(struct nlmsghdr* nl_header_answer);
int netlink_macfdb_table(struct nlmsghdr* nl_header_answer);

int netlink_request_route(struct nlsock* nl, int family, int type);
int netlink_request_route_add(struct nlsock* nl, int type,
                              ip_address_t* dst, ip_address_t* gw,
                              int default_gw, int ifidx);
int netlink_request_macs(struct nlsock* nl, int family, int type, ifindex_t master_ifindex);

#endif