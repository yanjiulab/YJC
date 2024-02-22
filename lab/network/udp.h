/* udp.h */

#ifndef __UDP_H__
#define __UDP_H__

#include <netinet/udp.h>

/* UDP header. See RFC 768. */
struct udp {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t len;   /* UDP length in bytes, includes UDP header */
    uint16_t check; /* UDP checksum */
};

typedef struct udp udp_t;

#endif /* __UDP_H__ */