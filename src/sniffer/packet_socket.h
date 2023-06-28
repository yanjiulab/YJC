#ifndef __PACKET_SOCKET_H__
#define __PACKET_SOCKET_H__

#include <assert.h>
#include <errno.h>
#include <linux/filter.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <unistd.h>

#include "packet.h"

#define TIMEOUT_NONE -1

typedef struct packet_socket {
    int packet_fd; /* socket for sending, sniffing timestamped packets */
    char *name;    /* malloc-allocated copy of interface name */
    int index;     /* interface index from if_nametoindex */
} packet_socket_t;

/* Allocate and initialize a packet socket. */
extern packet_socket_t *packet_socket_new(const char *device_name);

/* Free all the memory used by the packet socket. */
extern void packet_socket_free(struct packet_socket *packet_socket);

/* Add a filter so we only sniff packets we want. */
extern void packet_socket_set_filter(struct packet_socket *psock,
                                     struct sock_filter *filter, int len);

/* Do a blocking sniff (until timeout) of the next packet going over the given
 * device in the given direction, fill in the given packet with the sniffed
 * packet info, and return the number of bytes in the packet in
 * *in_bytes.
 *
 * If we successfully read a matching packet, return
 * STATUS_OK; If we timed out, return STATUS_TIMEOUT;
 * else return STATUS_ERR (in which case the caller can
 * retry).
 */
extern int packet_socket_receive(struct packet_socket *psock,
                                 enum direction_t direction,
                                 signed int timeout_secs, struct packet *packet,
                                 int *in_bytes);

extern int packet_socket_writev(struct packet_socket *psock,
                                const struct iovec *iov, int iovcnt);
extern int packet_socket_send(struct packet_socket *psock, unsigned char *data,
                              int datalen);

#endif