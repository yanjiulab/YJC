#ifndef _SOCKOPT_H_
#define _SOCKOPT_H_

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // TCP
#include <linux/filter.h>
#include <errno.h> // errno

typedef signed int ifindex_t;

#define SO_ATTACH_FILTER 26 // for remove warning

#define blocking(s)      fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define nonblocking(s)   fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)

// When enabled, a close(2) or shutdown(2) will not return
// until all queued messages for the socket have been
// successfully sent or the linger timeout has been reached.
// Otherwise, the call returns immediately and the closing is
// done in the background.
int so_linger(int sockfd, int timeout);

// Don't send via a gateway, send only to directly connected hosts.
// The same effect can be achieved by setting the MSG_DONTROUTE flag
// on a socket send(2) operation. Expects an integer boolean flag.
int so_noroute(int sockfd, int on);

// recv buffer size
int so_rcvbuf(int sockfd, int bufsize);
// send buffer size
int so_sndbuf(int sockfd, int bufsize);
// send timeout
int so_sndtimeo(int sockfd, int ms);
// recv timeout
int so_rcvtimeo(int sockfd, int ms);

int so_reuseaddr(int sockfd, int on);

int so_reuseport(int sockfd, int on);

int so_setfilter(int sockfd, struct sock_fprog fprog);

// Set or receive the Type-Of-Service (TOS) field that is
// sent with every IP packet originating from this socket.
int so_sendtos(int sockfd, int tos);
int go_sendtos(int sockfd);

// Set or retrieve the current time-to-live field that is
// used in every packet sent from this socket.
int so_sendttl(int sockfd, int ttl);
int go_sendttl(int sockfd);

/**
 * When this flag is set, pass a IP_TTL control message with
 * the time-to-live field of the received packet as a 32 bit integer.
 * Not supported for SOCK_STREAM sockets.
 *
 * Usage:
 *
 * // create a udp server
 * int sockfd = udp_socket();
 *
 * // set recvttl option
 * so_recvttl(sockfd, 1);
 *
 * // receive data with control message
 * num_bytes = recvmsg(sockfd, &msg, 0);
 *
 * // extract ttl in control message
 * for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
 *   if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL)
 *     ttl = (int)*CMSG_DATA(cmsg);
 *   if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_TTL)
 *     tos = (int)*CMSG_DATA(cmsg);
 * }
 *
 */
int so_recvttl(int sockfd, int on);

// If enabled, the IP_TOS ancillary message is passed with
// incoming packets.  It contains a byte which specifies the
// Type of Service/Precedence field of the packet header.
// Expects a boolean integer flag.
int so_recvtos(int sockfd, int on);

int so_pktinfo(int sockfd, int on);

int so_recvopts(int sockfd, int on);
int so_options(int sockfd, void* opt, int optlen);
int so_bindtodev(int sock, char* if_name);

// Set the broadcast flag.  When enabled, datagram sockets
// are allowed to send packets to a broadcast address.
// This option has no effect on stream-oriented sockets.
int udp_broadcast(int sockfd, int on);

int tcp_nodelay(int sockfd, int on);

int tcp_nopush(int sockfd, int on);

// Enable sending of keep-alive messages on connection- oriented sockets.
// Expects an integer boolean flag.
int tcp_keepalive(int sockfd, int on, int delay);

// send
// if = auto choose
// ttl = 1
// loop = true
int so_ipv4_multicast(int sock, int optname, struct in_addr if_addr, unsigned int mcast_addr, ifindex_t ifindex);
#endif
