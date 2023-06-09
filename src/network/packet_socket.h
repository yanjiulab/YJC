#ifndef __PACKET_SOCKET_H__
#define __PACKET_SOCKET_H__



/* Number of bytes to buffer in the packet socket we use for sniffing. */
static const int PACKET_SOCKET_RCVBUF_BYTES = 2 * 1024 * 1024;

typedef struct packet_socket {
    int packet_fd; /* socket for sending, sniffing timestamped packets */
    char *name;    /* malloc-allocated copy of interface name */
    int index;     /* interface index from if_nametoindex */
} packet_socket_t;

/* The directions in which a packet may flow. */
enum direction_t {
    DIRECTION_INVALID,
    DIRECTION_INBOUND,  /* packet coming into the kernel under test */
    DIRECTION_OUTBOUND, /* packet leaving the kernel under test */
};

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
extern int packet_socket_receive(struct packet_socket *psock,
                                 enum direction_t direction,
                                 signed int timeout_secs, struct packet *packet,
                                 int *in_bytes);
#endif