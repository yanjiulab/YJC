/* netlink_socket.h */

#ifndef __NETLINK_SOCKET_H__
#define __NETLINK_SOCKET_H__

/* Headers */
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "socket.h"
// #include <linux/rtnetlink.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct netlink_socket {
    int nl_fd;  /* socket for sending, sniffing timestamped netlinks */
    char *name; /* malloc-allocated copy of netlink name */
    struct sockaddr_nl snl;
    int seq;
} netlink_socket_t;

/* Allocate and initialize a netlink socket. */
extern netlink_socket_t *netlink_socket_new(int nl_type, const char *nl_name);

/* Free all the memory used by the netlink socket. */
extern void netlink_socket_free(struct netlink_socket *netlink_socket);


extern int netlink_parse_info(struct netlink_socket *nlsock,
                              int (*filter)(struct nlmsghdr *));
int netlink_rtm_parse_route(struct nlmsghdr *nl_header_answer);
int netlink_macfdb_table(struct nlmsghdr *nl_header_answer);
// request
int netlink_request_route(struct netlink_socket *nlsock, int family, int type);
int netlink_request_macs(struct netlink_socket *nlsock, int family, int type);
#ifdef __cplusplus
}
#endif

#endif /* __NETLINK_SOCKET_H__ */