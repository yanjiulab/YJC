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

#include "ipaddr.h"
#include "packet_header.h"

/* The directions in which a packet may flow. */
enum direction_t {
    DIRECTION_HOST,      /* To us.  */
    DIRECTION_BROADCAST, /* To all.  */
    DIRECTION_MULTICAST, /* To group.  */
    DIRECTION_OTHERHOST, /* To someone else.  */
    DIRECTION_OUTGOING,  /* Originated by us . */
    DIRECTION_LOOPBACK,
    DIRECTION_FASTROUTE,
    DIRECTION_ALL
};

/* What layer of headers is at the head of the packet? */
enum packet_layer_t {
    PACKET_LAYER_2_ETHERNET, /* layer 2 is Ethernet */
    PACKET_LAYER_3_IP,       /* no layer 2 headers */
    PACKET_LAYER_4_UDP,      /* no layer 3 headers */
    PACKET_LAYER_4_TCP,      /* no layer 3 headers */
    PACKET_LAYER_CUSTOM,     /* custom layer */
    PACKET_LAYER_MAX
};

typedef struct packet packet_t;
typedef struct packet_list packet_list_t;

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
    uint8_t* buffer;        /* data buffer: full contents of packet */
    uint32_t buffer_bytes;  /* bytes of space in data buffer */
    uint32_t buffer_active; /* bytes of active space in data buffer */

    // uint32_t l2_header_bytes;   /* bytes in outer hardware/layer-2 header */
    // uint32_t l2_data_bytes;     /* bytes in outermost IP hdrs/payload */
    uint32_t dev_ifindex;       /* from which network device */
    enum direction_t direction; /* direction packet is traveling */
    struct timeval tv;          /* wall time of receive/send if non-zero */

    /* Metadata about all the headers in the packet, including all
     * layers of encapsulation, from outer to inner, starting from
     * the outermost IP header at headers[0].
     */
    // struct header headers[PACKET_MAX_HEADERS];

    /* The following pointers point into the 'buffer' area. Each
     * pointer may be NULL if there is no header of that type
     * present in the packet. In each case these are pointers to
     * the innermost header of that kind, since that is where most
     * of the interesting TCP/UDP/IP action is.
     */
    /* Layer 2 */
    ethhdr_t* eth; /* start of Ethernet header, if present */
    /* Layer 3 */
    struct ipv4* ipv4; /* start of IPv4 header, if present */
    struct ipv6* ipv6; /* start of IPv6 header, if present */
    /* Layer 4 */
    struct tcp* tcp; /* start of TCP header, if present */
    struct udp* udp; /* start of UDP header, if present */
};

/* A simple list of packets. */
struct packet_list {
    struct packet* packet;    /* the packet content */
    struct packet_list* next; /* link to next element, or NULL if last */
};

/* Allocate a packet_list and initialize its fields to NULL. */
extern struct packet_list* packet_list_new(void);

/* Free an entire packet list. */
extern void packet_list_free(struct packet_list* list);

/* Allocate and initialize a packet. */
extern struct packet* packet_new(uint32_t buffer_length);

/* Free all the memory used by the packet. */
extern void packet_free(struct packet* packet);

/* Create a packet that is a copy of the contents of the given packet. */
extern struct packet* packet_copy(struct packet* old_packet);

/* Convenience accessors for peeking around in the packet... */

/* Return a pointer to the first byte of the outermost IP header. */
static inline uint8_t* packet_start(const struct packet* packet) {
    uint8_t* start = packet->buffer;
    assert(start != NULL);
    return start;
}

/* Return a pointer to the byte beyond the end of the packet. */
static inline uint8_t* packet_end(const struct packet* packet) {
    return packet_start(packet) + packet->buffer_active;
}

/* Return the length of the TCP header, including options. */
static inline int packet_tcp_header_len(const struct packet* packet) {
    assert(packet->tcp);
    return packet->tcp->doff * sizeof(u32);
}

/* Return the length of the UDP header. */
static inline int packet_udp_header_len(const struct packet* packet) {
    assert(packet->udp);
    return sizeof(struct udp);
}

/* Return a pointer to the TCP/UDP data payload. */
static inline uint8_t* packet_payload(const struct packet* packet) {
    if (packet->tcp)
        return ((uint8_t*)packet->tcp) + packet_tcp_header_len(packet);
    if (packet->udp)
        return ((uint8_t*)packet->udp) + packet_udp_header_len(packet);
    // if (packet->icmpv4)
    //     return ((uint8_t*)packet->icmpv4) + packet_icmpv4_header_len(packet);
    // if (packet->icmpv6)
    //     return ((uint8_t*)packet->icmpv6) + packet_icmpv6_header_len(packet);

    assert(!"no valid payload; not TCP or UDP or ICMP!?");
    return NULL;
}

/* Return the length of the TCP/UDP payload. */
static inline int packet_payload_len(const struct packet* packet) {
    return packet_end(packet) - packet_payload(packet);
}

/* A TCP/UDP/IP address for an endpoint. */
struct endpoint {
    struct ipaddr ip; /* IP address */
    __be16 port;          /* TCP/UDP port (network order) */
};

/* The 4-tuple for a TCP/UDP/IP packet. */
struct tuple {
    struct endpoint src;
    struct endpoint dst;
};

/* Return true iff the two tuples are equal. */
static inline bool is_equal_tuple(const struct tuple* a,
                                  const struct tuple* b) {
    return memcmp(a, b, sizeof(*a)) == 0;
}

static inline void reverse_tuple(const struct tuple* src_tuple,
                                 struct tuple* dst_tuple) {
    dst_tuple->src.ip = src_tuple->dst.ip;
    dst_tuple->dst.ip = src_tuple->src.ip;
    dst_tuple->src.port = src_tuple->dst.port;
    dst_tuple->dst.port = src_tuple->src.port;
}

/* Get the tuple for a packet. */
static inline void get_packet_tuple(const struct packet* packet,
                                    struct tuple* tuple) {
    memset(tuple, 0, sizeof(*tuple));
    if (packet->ipv4 != NULL) {
        ip_from_ipv4(&packet->ipv4->src_ip, &tuple->src.ip);
        ip_from_ipv4(&packet->ipv4->dst_ip, &tuple->dst.ip);
    } else if (packet->ipv6 != NULL) {
        ip_from_ipv6(&packet->ipv6->src_ip, &tuple->src.ip);
        ip_from_ipv6(&packet->ipv6->dst_ip, &tuple->dst.ip);
    } else {
        assert(!"bad IP version in packet");
    }
    if (packet->tcp != NULL) {
        tuple->src.port = packet->tcp->src_port;
        tuple->dst.port = packet->tcp->dst_port;
    } else if (packet->udp != NULL) {
        tuple->src.port = packet->udp->src_port;
        tuple->dst.port = packet->udp->dst_port;
    }
}
#endif