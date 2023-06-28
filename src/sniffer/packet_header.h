#ifndef __PACKET_HEADER_H__
#define __PACKET_HEADER_H__

// #include <sys/time.h>
#include <stdint.h>

#include "assert.h"
#include "ethernet.h"
// #include "gre.h"
// #include "icmp.h"
// #include "icmpv6.h"
#include "ip.h"
// #include "ipv6.h"
// #include "mpls.h"
#include "defs.h"
#include "tcp.h"
#include "udp.h"

struct packet;

/* The type of a header in a packet. */
enum header_t {
    HEADER_NONE,
    HEADER_ETH,
    HEADER_IPV4,
    HEADER_IPV6,
    HEADER_GRE,
    HEADER_MPLS,
    HEADER_TCP,
    HEADER_UDP,
    HEADER_ICMPV4,
    HEADER_ICMPV6,
    HEADER_NUM_TYPES
};

/* Metadata about a header in a packet. We support multi-layer encapsulation. */
struct header {
    enum header_t type;    /* type of this header */
    uint32_t header_bytes; /* length of this header */
    uint32_t total_bytes;  /* length of header plus data inside */
    union {
        uint8_t *ptr; /* a pointer to the header bits */
        struct ipv4 *ipv4;
        struct ipv6 *ipv6;
        struct gre *gre;
        struct mpls *mpls;
        struct tcp *tcp;
        struct udp *udp;
        struct icmpv4 *icmpv4;
        struct icmpv6 *icmpv6;
    } h;
};

// /* Info for a particular type of header. */
// struct header_type_info {
//     const char *name; /* human-readable protocol name */
//     u8 ip_proto;      /* IP protocol code */
//     u16 eth_proto;    /* Ethernet protocol code */

//     /* Call this to finalize the header once we know what's inside... */
//     int (*finish)(struct packet *packet, struct header *header,
//                   struct header *next_inner);
// };

// /* Return the info for the given type of header. */
// extern struct header_type_info *header_type_info(enum header_t header_type);

#endif /* __PACKET_HEADER_H__ */
