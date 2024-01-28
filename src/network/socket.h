#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <arpa/inet.h> // inet_*, address
#include <errno.h>     // errno
#include <fcntl.h>
#include <netdb.h> // dns
#include <netinet/in.h>
#include <netinet/tcp.h> // TCP
#include <stdbool.h>     // bool
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h> // sockaddr_un
#include <unistd.h>

#include "defs.h"
#include "export.h"

//-----------------------------socket----------------------------------------------
#define LOCALHOST "127.0.0.1"
#define ANYADDR "0.0.0.0"
#define INVALID_SOCKET -1

static inline int socket_errno() { return errno; }
EXPORT const char* socket_strerror(int err);
static inline int closesocket(int sockfd) { return close(sockfd); }

#ifndef SAFE_CLOSESOCKET
#define SAFE_CLOSESOCKET(fd) \
    do {                     \
        if ((fd) >= 0) {     \
            closesocket(fd); \
            (fd) = -1;       \
        }                    \
    } while (0)
#endif

//-----------------------------sockaddr_u----------------------------------------------
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
#ifdef ENABLE_UDS
    struct sockaddr_un sun;
#endif
} sockaddr_u;

// @param host: domain or ip
// @retval 0:succeed
int ResolveAddr(const char* host, sockaddr_u* addr);

const char* sockaddr_ip(sockaddr_u* addr, char* ip, int len);
uint16_t sockaddr_port(sockaddr_u* addr);
int sockaddr_set_ip(sockaddr_u* addr, const char* host);
void sockaddr_set_port(sockaddr_u* addr, int port);
int sockaddr_set_ipport(sockaddr_u* addr, const char* host, int port);
socklen_t sockaddr_len(sockaddr_u* addr);
const char* sockaddr_str(sockaddr_u* addr, char* buf, int len);

#define inet_atoi_n(cp) inet_addr(cp)
#define inet_atoi_h(cp) ntohl(inet_addr(cp))
char* inet_itoa_h(uint32_t i);
char* inet_itoa_n(uint32_t i);

#ifdef ENABLE_UDS
#define SOCKADDR_STRLEN sizeof(((struct sockaddr_un*)(NULL))->sun_path)
static inline void sockaddr_set_path(sockaddr_u* addr, const char* path) {
    addr->sa.sa_family = AF_UNIX;
    strncpy(addr->sun.sun_path, path, sizeof(addr->sun.sun_path));
}
#else
#define SOCKADDR_STRLEN 64 // ipv4:port | [ipv6]:port
#endif

static inline void sockaddr_print(sockaddr_u* addr) {
    char buf[SOCKADDR_STRLEN] = {0};
    sockaddr_str(addr, buf, sizeof(buf));
    puts(buf);
}

#define SOCKADDR_LEN(addr) sockaddr_len((sockaddr_u*)addr)
#define SOCKADDR_STR(addr, buf) \
    sockaddr_str((sockaddr_u*)addr, buf, sizeof(buf))
#define SOCKADDR_PRINT(addr) sockaddr_print((sockaddr_u*)addr)

//-----------------------------base
// functions----------------------------------------------

// socket -> setsockopt -> bind
// @param type: SOCK_STREAM(tcp) SOCK_DGRAM(udp)
// @return sockfd
EXPORT int Bind(int port, const char* host DEFAULT(ANYADDR),
                int type DEFAULT(SOCK_STREAM));

// Bind -> listen
// @return listenfd
EXPORT int Listen(int port, const char* host DEFAULT(ANYADDR));

// @return connfd
// ResolveAddr -> socket -> nonblocking -> connect
EXPORT int Connect(const char* host, int port, int nonblock DEFAULT(0));
// Connect(host, port, 1)
EXPORT int ConnectNonblock(const char* host, int port);
// Connect(host, port, 1) -> select -> blocking
#define DEFAULT_CONNECT_TIMEOUT 10000 // ms
EXPORT int ConnectTimeout(const char* host, int port,
                          int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));

#ifdef ENABLE_UDS
EXPORT int BindUnix(const char* path, int type DEFAULT(SOCK_STREAM));
EXPORT int ListenUnix(const char* path);
EXPORT int ConnectUnix(const char* path, int nonblock DEFAULT(0));
EXPORT int ConnectUnixNonblock(const char* path);
EXPORT int ConnectUnixTimeout(const char* path,
                              int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));
#endif

EXPORT int Socketpair(int family, int type, int protocol, int sv[2]);

/************************** Socket Option API ***************************************/

#define blocking(s) fcntl(s, F_SETFL, fcntl(s, F_GETFL) & ~O_NONBLOCK)
#define nonblocking(s) fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)

static inline int tcp_nodelay(int sockfd, int on DEFAULT(1)) {
    return setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&on,
                      sizeof(int));
}

static inline int tcp_nopush(int sockfd, int on DEFAULT(1)) {
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

static inline int tcp_keepalive(int sockfd, int on DEFAULT(1), int delay DEFAULT(60)) {
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

static inline int udp_broadcast(int sockfd, int on DEFAULT(1)) {
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

static inline int so_reuseaddr(int sockfd, int on DEFAULT(1)) {
#ifdef SO_REUSEADDR
    // NOTE: SO_REUSEADDR allow to reuse sockaddr of TIME_WAIT status
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on,
                      sizeof(int));
#else
    return 0;
#endif
}

static inline int so_reuseport(int sockfd, int on DEFAULT(1)) {
#ifdef SO_REUSEPORT
    // NOTE: SO_REUSEPORT allow multiple sockets to bind same port
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&on,
                      sizeof(int));
#else
    return 0;
#endif
}

static inline int so_linger(int sockfd, int timeout DEFAULT(1)) {
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

#endif // !__SOCKET_H__
