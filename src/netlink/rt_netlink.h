/* rt_netlink.h */

#ifndef __RT_NETLINK_H__
#define __RT_NETLINK_H__

#include "nl_kernel.h"

typedef signed int ifindex_t;

#define RTM_NHA(h) \
    ((struct rtattr*)(((char*)(h)) + NLMSG_ALIGN(sizeof(struct nhmsg))))

// High-level API
int netlink_route_read(struct nl_socket* nlsock);
int netlink_route_change(struct nl_socket* nlsock);
int netlink_macfdb_read(struct nl_socket* nlsock); 
int netlink_neigh_read(struct nl_socket* nlsock); // ==> ip neigh show nud all

// Low-level API
int netlink_rtm_parse_route(struct nlmsghdr* nl_header_answer);
int netlink_macfdb_table(struct nlmsghdr* nl_header_answer);

int netlink_request_route(struct nl_socket* nlsock, int family, int type);
int netlink_request_route_add(struct nl_socket* nlsock, int type,
                              ip_address_t* dst, ip_address_t* gw,
                              int default_gw, int ifidx);
int netlink_request_macs(struct nl_socket* nlsock, int family, int type, ifindex_t master_ifindex);

#endif