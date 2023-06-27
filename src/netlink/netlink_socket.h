/* netlink_socket.h */

#ifndef __NETLINK_SOCKET_H__
#define __NETLINK_SOCKET_H__

/* Headers */
#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
// #include <linux/nexthop.h>

#include <net/if.h>
#include "ethernet.h"
#include "net_utils.h"
#include "ip_address.h"
#include "log.h"
#include "socket.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NDA_RTA(r) \
    ((struct rtattr *)(((char *)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *)(((void *)(nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

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

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
ssize_t netlink_sendmsg(struct netlink_socket *nl, void *buf, size_t buflen);

/*
 * netlink_recvmsg - receive a netlink message.
 * Returns -1 on error, 0 if read would block or the number of bytes received.
 */
ssize_t netlink_recvmsg(int fd, struct msghdr *msg, char **answer);

/* Issue request message to kernel via netlink socket. GET messages
 * are issued through this interface.
 */
int netlink_request(struct netlink_socket *nlsock, void *req);

/* Parse netlink response */
extern int netlink_parse_info(struct netlink_socket *nlsock,
                              int (*filter)(struct nlmsghdr *));

#ifdef __cplusplus
}
#endif

#endif /* __NETLINK_SOCKET_H__ */