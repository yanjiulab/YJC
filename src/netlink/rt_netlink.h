/* rt_netlink.h */

#ifndef __RT_NETLINK_H__
#define __RT_NETLINK_H__

#include "netlink_socket.h"

int netlink_rtm_parse_route(struct nlmsghdr *nl_header_answer);
int netlink_macfdb_table(struct nlmsghdr *nl_header_answer);

int netlink_request_route(struct netlink_socket *nlsock, int family, int type);
int netlink_request_route_add(struct netlink_socket *nlsock, int type,
                              ip_address_t *dst, ip_address_t *gw,
                              int default_gw, int ifidx);
int netlink_request_macs(struct netlink_socket *nlsock, int family, int type);

#endif 