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

#define TIMEOUT_NONE -1

/* Most functions in this codebase return one of these two values to let the
 * caller know whether there was a problem.
 */
enum status_t {
    STATUS_OK = 0,
    STATUS_ERR = -1,
    STATUS_WARN = -2,   /* a non-fatal error or warning */
    STATUS_TIMEOUT = -3 /* timeout sniffing outbound packet */
};

/* The directions in which a packet may flow. */
enum direction_t {
    DIRECTION_INVALID,
    DIRECTION_INBOUND,  /* packet coming into the kernel under test */
    DIRECTION_OUTBOUND, /* packet leaving the kernel under test */
};

struct packet {
    uint8_t *buffer;            /* data buffer: full contents of packet */
    uint32_t buffer_bytes;      /* bytes of space in data buffer */
    uint32_t l2_header_bytes;   /* bytes in outer hardware/layer-2 header */
    uint32_t ip_bytes;          /* bytes in outermost IP hdrs/payload */
    enum direction_t direction; /* direction packet is traveling */
    int64_t time_usecs;         /* wall time of receive/send if non-zero */
};

typedef struct packet_socket {
    int packet_fd; /* socket for sending, sniffing timestamped packets */
    char *name;    /* malloc-allocated copy of interface name */
    int index;     /* interface index from if_nametoindex */
} packet_socket_t;

/* Allocate and initialize a packet socket. */
extern packet_socket_t *packet_socket_new(const char *device_name);

/* Free all the memory used by the packet socket. */
extern void packet_socket_free(struct packet_socket *packet_socket);

/* Do a blocking sniff (until timeout) of the next packet going over the given
 * device in the given direction, fill in the given packet with the sniffed
 * packet info, and return the number of bytes in the packet in
 * *in_bytes. If we successfully read a matching packet, return
 * STATUS_OK; If we timed out, return STATUS_TIMEOUT;
 * else return STATUS_ERR (in which case the caller can
 * retry).
 */
// extern int packet_socket_receive(struct packet_socket *psock,
//                                  enum direction_t direction,
//                                  signed int timeout_secs, struct packet
//                                  *packet, int *in_bytes);

#endif