/* udp.h */

#ifndef __UDP_H__
#define __UDP_H__

// #include <netinet/udp.h>

/* UDP header. See RFC 768. */
struct udp {
    __be16 src_port;
    __be16 dst_port;
    __be16 len;    /* UDP length in bytes, includes UDP header */
    __sum16 check; /* UDP checksum */
};

typedef struct udp udp_t;

#endif /* __UDP_H__ */