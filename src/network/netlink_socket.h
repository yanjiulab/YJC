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
} netlink_socket_t;

/* Allocate and initialize a netlink socket. */
extern netlink_socket_t *netlink_socket_new(int nl_type, const char *nl_name);

/* Free all the memory used by the netlink socket. */
extern void netlink_socket_free(struct netlink_socket *netlink_socket);

extern int netlink_recvmsg(struct netlink_socket *nlsock);

int netlink_rt_get_req(struct netlink_socket *nlsock);


#ifdef __cplusplus
}
#endif

#endif /* __NETLINK_SOCKET_H__ */