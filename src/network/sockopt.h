#ifndef _SOCKOPT_H_
#define _SOCKOPT_H_

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // TCP
#include <linux/filter.h>
#include <errno.h> // errno

#define blocking(s)    fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define nonblocking(s) fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)

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

#endif