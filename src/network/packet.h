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
/* ---------------------------- packet header ---------------------------- */
struct packet;

/* The type of a header in a packet. */
enum header_t {
    HEADER_NONE,
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



/* ---------------------------- packet ---------------------------- */

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

/* TCP/UDP/IPv4 packet, including IPv4 header, TCP/UDP header, and data. There
 * may also be a link layer header between the 'buffer' and 'ip'
 * pointers, but we typically ignore that. The 'buffer_bytes' field
 * gives the total space in the buffer, which may be bigger than the
 * actual amount occupied by the packet data.
 */
struct packet {
    uint8_t *buffer;            /* data buffer: full contents of packet */
    uint32_t buffer_bytes;      /* bytes of space in data buffer */
    uint32_t l2_header_bytes;   /* bytes in outer hardware/layer-2 header */
    uint32_t ip_bytes;          /* bytes in outermost IP hdrs/payload */
    enum direction_t direction; /* direction packet is traveling */
    int64_t time_usecs;         /* wall time of receive/send if non-zero */
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

/* ---------------------------- packet dump ---------------------------- */

enum dump_format_t {
    DUMP_SHORT,   /* brief format used in scripts */
    DUMP_FULL,    /* add local and remote address and port */
    DUMP_VERBOSE, /* add hex dump */
};

/* Returns in *ascii_string a human-readable representation of the
 * packet 'packet'. Returns STATUS_OK on success; on failure returns
 * STATUS_ERR and sets error message.
 */
extern int packet_to_string(struct packet *packet, enum dump_format_t format,
                            char **ascii_string, char **error);
extern char *to_printable_string(const char *in, int in_len);
extern void hex_dump(const uint8_t *buffer, int bytes, char **hex);

/* ---------------------------- packet parse ---------------------------- */

/* ---------------------------- packet generate ---------------------------- */

#endif