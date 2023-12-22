/* nl_kernel.h */

#ifndef __NL_SOCKET_H__
#define __NL_SOCKET_H__

/* Headers */
#include <linux/neighbour.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include "log.h"
#include "network.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NDA_RTA(r) \
    ((struct rtattr *)(((char *)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))

#define NLMSG_TAIL(nmsg) \
    ((struct rtattr *)(((void *)(nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

/*
 * nl_attr_put - add an attribute to the Netlink message.
 *
 * Returns true if the attribute could be added to the message (fits into the
 * buffer), otherwise false is returned.
 */
extern bool nl_attr_put(struct nlmsghdr *n, unsigned int maxlen, int type,
                        const void *data, unsigned int alen);
extern bool nl_attr_put8(struct nlmsghdr *n, unsigned int maxlen, int type,
                         uint8_t data);
extern bool nl_attr_put16(struct nlmsghdr *n, unsigned int maxlen, int type,
                          uint16_t data);
extern bool nl_attr_put32(struct nlmsghdr *n, unsigned int maxlen, int type,
                          uint32_t data);
extern bool nl_attr_put64(struct nlmsghdr *n, unsigned int maxlen, int type,
                          uint64_t data);

/*
 * nl_attr_nest - start an attribute nest.
 *
 * Returns a valid pointer to the beginning of the nest if the attribute
 * describing the nest could be added to the message (fits into the buffer),
 * otherwise NULL is returned.
 */
extern struct rtattr *nl_attr_nest(struct nlmsghdr *n, unsigned int maxlen,
                                   int type);

/*
 * nl_attr_nest_end - finalize nesting of attributes.
 *
 * Updates the length field of the attribute header to include the appeneded
 * attributes. Returns a total length of the Netlink message.
 */
extern int nl_attr_nest_end(struct nlmsghdr *n, struct rtattr *nest);

/*
 * nl_attr_rtnh - append a rtnexthop record to the Netlink message.
 *
 * Returns a valid pointer to the rtnexthop struct if it could be added to
 * the message (fits into the buffer), otherwise NULL is returned.
 */
extern struct rtnexthop *nl_attr_rtnh(struct nlmsghdr *n, unsigned int maxlen);

/*
 * nl_attr_rtnh_end - finalize adding a rtnexthop record.
 *
 * Updates the length field of the rtnexthop to include the appeneded
 * attributes.
 */
extern void nl_attr_rtnh_end(struct nlmsghdr *n, struct rtnexthop *rtnh);

extern void netlink_parse_rtattr(struct rtattr **tb, int max,
                                 struct rtattr *rta, int len);
extern void netlink_parse_rtattr_flags(struct rtattr **tb, int max,
                                       struct rtattr *rta, int len,
                                       unsigned short flags);
extern void netlink_parse_rtattr_nested(struct rtattr **tb, int max,
                                        struct rtattr *rta);
/*
 * nl_addraw_l copies raw form the netlink message buffer into netlink
 * message header pointer. It ensures the aligned data buffer does not
 * override past max length.
 * return value is 0 if its successful
 */
extern bool nl_addraw_l(struct nlmsghdr *n, unsigned int maxlen,
                        const void *data, unsigned int len);
/*
 * nl_rta_put - add an additional optional attribute(rtattr) to the
 * Netlink message buffer.
 *
 * Returns true if the attribute could be added to the message (fits into the
 * buffer), otherwise false is returned.
 */
extern bool nl_rta_put(struct rtattr *rta, unsigned int maxlen, int type,
                       const void *data, int alen);
extern bool nl_rta_put16(struct rtattr *rta, unsigned int maxlen, int type,
                         uint16_t data);
extern bool nl_rta_put64(struct rtattr *rta, unsigned int maxlen, int type,
                         uint64_t data);
/*
 * nl_rta_nest - start an additional optional attribute (rtattr) nest.
 *
 * Returns a valid pointer to the beginning of the nest if the attribute
 * describing the nest could be added to the message (fits into the buffer),
 * otherwise NULL is returned.
 */
extern struct rtattr *nl_rta_nest(struct rtattr *rta, unsigned int maxlen,
                                  int type);
/*
 * nl_rta_nest_end - finalize nesting of an aditionl optionl attributes.
 *
 * Updates the length field of the attribute header to include the appeneded
 * attributes. Returns a total length of the Netlink message.
 */
extern int nl_rta_nest_end(struct rtattr *rta, struct rtattr *nest);
extern const char *nl_msg_type_to_str(uint16_t msg_type);
extern const char *nl_rtproto_to_str(uint8_t rtproto);
extern const char *nl_family_to_str(uint8_t family);
extern const char *nl_rttype_to_str(uint8_t rttype);

typedef struct nl_socket {
    int nl_fd;  /* socket for sending, sniffing timestamped netlinks */
    char *name; /* malloc-allocated copy of netlink name */
    struct sockaddr_nl snl;
    int seq;
} nl_socket_t;

/**
 * @brief Allocate and initialize a netlink socket.
 *
 * @param nl_type selects the kernel module or netlink group to communicate with.
 * @param nl_name user-friendly name of the socket.
 * @param groups set netlink socket address groups.
 * @return nl_socket_t* the pointer of the allocated netlink socket.
 */
extern nl_socket_t *nl_socket_new(int nl_type, const char *nl_name, int groups);

/**
 * @brief Free all the memory used by the netlink socket.
 *
 * @param nl_socket
 */
extern void nl_socket_free(struct nl_socket *nl_socket);

extern int nl_socket_set_groups(struct nl_socket *nlsock, int group);

/*
 * netlink_sendmsg - send a netlink message of a certain size.
 * Returns -1 on error. Otherwise, it returns the number of bytes sent.
 */
ssize_t netlink_sendmsg(struct nl_socket *nl, void *buf, size_t buflen);

/*
 * netlink_recvmsg - receive a netlink message.
 * Returns -1 on error, 0 if read would block or the number of bytes received.
 */
ssize_t netlink_recvmsg(int fd, struct msghdr *msg, char **answer);

/* Issue request message to kernel via netlink socket. GET messages
 * are issued through this interface.
 */
extern int netlink_request(struct nl_socket *nlsock, void *req);

/*
 * Receive message from netlink interface and pass those information
 *  to the given function.
 *
 * nlsock  -> netlink socket information
 * filter  -> Function to call to read the results
 */
extern int netlink_parse_info(struct nl_socket *nlsock,
                              int (*filter)(struct nlmsghdr *));

/*
 * Parse a netlink error message
 *
 * Returns 1 if this message is acknowledgement, 0 if this error should be
 * ignored, -1 otherwise.
 */
extern int netlink_parse_error(const struct nl_socket *nlsock, struct nlmsghdr *h);

#ifdef __cplusplus
}
#endif

#endif /* __NL_SOCKET_H__ */