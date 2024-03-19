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

// set socket to blocking
#define blocking(s)      fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)

// set socket to non-blocking
#define nonblocking(s)   fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)

/* SOL_SOCKET */

/* IPPROTO_IP */

// Set or receive the Type-Of-Service (TOS) field that is
// sent with every IP packet originating from this socket.
int setsockopt_tos(int sockfd, int tos);
int getsockopt_tos(int sockfd);

// Set or retrieve the current time-to-live field that is
// used in every packet sent from this socket.
int setsockopt_ttl(int sockfd, int ttl);
int getsockopt_ttl(int sockfd);

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
 * setsockopt_recvttl(sockfd, 1);
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
int setsockopt_recvttl(int sockfd, int on);

// If enabled, the IP_TOS ancillary message is passed with
// incoming packets.  It contains a byte which specifies the
// Type of Service/Precedence field of the packet header.
// Expects a boolean integer flag.
int setsockopt_recvtos(int sockfd, int on);

#define TODO_BELOW

static inline int tcp_nodelay(int sockfd, int on) {
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on,
                      sizeof(int));
}

static inline int tcp_nopush(int sockfd, int on) {
#ifdef TCP_NOPUSH
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NOPUSH, (const char*)&on,
                      sizeof(int));
#elif defined(TCP_CORK)
    return setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, (const char*)&on,
                      sizeof(int));
#else
    return 0;
#endif
}

/* IPPROTO_TCP */
static inline int tcp_keepalive(int sockfd, int on, int delay) {
    if (setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (const char*)&on, sizeof(int)) != 0) {
        return errno;
    }

#ifdef TCP_KEEPALIVE
    return setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPALIVE, (const char*)&delay, sizeof(int));
#elif defined(TCP_KEEPIDLE)
    // TCP_KEEPIDLE     => tcp_keepalive_time
    // TCP_KEEPCNT      => tcp_keepalive_probes
    // TCP_KEEPINTVL    => tcp_keepalive_intvl
    return setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, (const char*)&delay,
                      sizeof(int));
#else
    return 0;
#endif
}

static inline int udp_broadcast(int sockfd, int on) {
    return setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (const char*)&on,
                      sizeof(int));
}

// send timeout
static inline int so_sndtimeo(int sockfd, int timeout) {
#ifdef OS_WIN
    return setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(int));
#else
    struct timeval tv = {timeout / 1000, (timeout % 1000) * 1000};
    return setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

// recv timeout
static inline int so_rcvtimeo(int sockfd, int timeout) {
#ifdef OS_WIN
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(int));
#else
    struct timeval tv = {timeout / 1000, (timeout % 1000) * 1000};
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
}

// send buffer size
static inline int so_sndbuf(int sockfd, int len) {
    return setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (const char*)&len, sizeof(int));
}

// recv buffer size
static inline int so_rcvbuf(int sockfd, int len) {
    return setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (const char*)&len, sizeof(int));
}

static inline int so_reuseaddr(int sockfd, int on) {
#ifdef SO_REUSEADDR
    // NOTE: SO_REUSEADDR allow to reuse sockaddr of TIME_WAIT status
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on,
                      sizeof(int));
#else
    return 0;
#endif
}

static inline int so_reuseport(int sockfd, int on) {
#ifdef SO_REUSEPORT
    // NOTE: SO_REUSEPORT allow multiple sockets to bind same port
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on,
                      sizeof(int));
#else
    return 0;
#endif
}

static inline int so_linger(int sockfd, int timeout) {
#ifdef SO_LINGER
    struct linger linger;
    if (timeout >= 0) {
        linger.l_onoff = 1;
        linger.l_linger = timeout;
    } else {
        linger.l_onoff = 0;
        linger.l_linger = 0;
    }
    // NOTE: SO_LINGER change the default behavior of close, send RST, avoid
    // TIME_WAIT
    return setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (const char*)&linger,
                      sizeof(linger));
#else
    return 0;
#endif
}

static inline int so_setfilter(int sockfd, struct sock_fprog fprog) {
    return setsockopt(sockfd, SOL_SOCKET, SO_ATTACH_FILTER, &fprog, sizeof(fprog));
}

// send
// if = auto choose
// ttl = 1
// loop = true
int setsockopt_ipv4_multicast(int sock, int optname, struct in_addr if_addr, unsigned int mcast_addr, ifindex_t ifindex);
int so_bindtodev(int sock, char* if_name);
#endif
