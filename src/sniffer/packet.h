#ifndef __PACKET_H__
#define __PACKET_H__

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

#include "packet_header.h"

/* The directions in which a packet may flow. */
enum direction_t {
    DIRECTION_INVALID,
    DIRECTION_INBOUND,  /* packet coming into the kernel under test */
    DIRECTION_OUTBOUND, /* packet leaving the kernel under test */
};

/* What layer of headers is at the head of the packet? */
enum packet_layer_t {
    PACKET_LAYER_3_IP = 0,   /* no layer 2 headers */
    PACKET_LAYER_2_ETHERNET, /* layer 2 is Ethernet */
};

/* Maximum number of headers. */
#define PACKET_MAX_HEADERS 6

/* We allow reading pretty big packets, since some interface MTUs can
 * be pretty big (the Linux loopback MTU, for example, is typically
 * around 16KB).
 */
static const int PACKET_READ_BYTES = 64 * 1024;

/* TCP/UDP/IPv4 packet, including IPv4 header, TCP/UDP header, and data. There
 * may also be a link layer header between the 'buffer' and 'ip'
 * pointers, but we typically ignore that. The 'buffer_bytes' field
 * gives the total space in the buffer, which may be bigger than the
 * actual amount occupied by the packet data.
 */
struct packet {
    uint8_t *buffer;        /* data buffer: full contents of packet */
    uint32_t buffer_bytes;  /* bytes of space in data buffer */
    uint32_t buffer_active; /* bytes of active space in data buffer */
    // uint32_t l2_header_bytes;   /* bytes in outer hardware/layer-2 header */
    // uint32_t l2_data_bytes;     /* bytes in outermost IP hdrs/payload */
    enum direction_t direction; /* direction packet is traveling */

    /* Metadata about all the headers in the packet, including all
     * layers of encapsulation, from outer to inner, starting from
     * the outermost IP header at headers[0].
     */
    struct header headers[PACKET_MAX_HEADERS];

    int64_t time_usecs; /* wall time of receive/send if non-zero */
};

/* A simple list of packets. */
struct packet_list {
    struct packet *packet;    /* the packet content */
    struct packet_list *next; /* link to next element, or NULL if last */
};

/* Allocate a packet_list and initialize its fields to NULL. */
extern struct packet_list *packet_list_new(void);

/* Free an entire packet list. */
extern void packet_list_free(struct packet_list *list);

/* Allocate and initialize a packet. */
extern struct packet *packet_new(uint32_t buffer_length);

/* Free all the memory used by the packet. */
extern void packet_free(struct packet *packet);

/* Create a packet that is a copy of the contents of the given packet. */
extern struct packet *packet_copy(struct packet *old_packet);

/* Return the number of headers in the given packet. */
extern int packet_header_count(const struct packet *packet);

/* Attempt to append a new header to the given packet. Return a
 * pointer to the new header metadata, or NULL if we can't add the
 * header.
 */
extern struct header *packet_append_header(struct packet *packet,
                                           enum header_t header_type,
                                           int header_bytes);

#endif