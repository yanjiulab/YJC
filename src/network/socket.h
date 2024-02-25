#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <arpa/inet.h> // inet_*, address
#include <errno.h>     // errno
#include <fcntl.h>
#include <linux/filter.h>
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
#include "errors.h"

//-----------------------------socket----------------------------------------------
#define LOCALHOST      "127.0.0.1"
#define ANYADDR        "0.0.0.0"
#define INVALID_SOCKET -1

/* Following shortens all the typecasts of pointer arguments: */
#define LISTENQ        1024
#define SA             struct sockaddr

static inline int socket_errno() {
    return errno;
}
EXPORT const char* socket_strerror(int err);
static inline int closesocket(int sockfd) {
    return close(sockfd);
}

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
// typedef union {
//     struct sockaddr sa;
//     struct sockaddr_in sin;
//     struct sockaddr_in6 sin6;
// #ifdef ENABLE_UDS
//     struct sockaddr_un sun;
// #endif
// } sockaddr_u;

// // @param host: domain or ip
// // @retval 0:succeed
// int ResolveAddr(const char* host, sockaddr_u* addr);

// const char* sockaddr_ip(sockaddr_u* addr, char* ip, int len);
// uint16_t sockaddr_port(sockaddr_u* addr);
// int sockaddr_set_ip(sockaddr_u* addr, const char* host);
// void sockaddr_set_port(sockaddr_u* addr, int port);
// int sockaddr_set_ipport(sockaddr_u* addr, const char* host, int port);
// socklen_t sockaddr_len(sockaddr_u* addr);
// const char* sockaddr_str(sockaddr_u* addr, char* buf, int len);

// #define inet_atoi_n(cp) inet_addr(cp)
// #define inet_atoi_h(cp) ntohl(inet_addr(cp))
// char* inet_itoa_h(uint32_t i);
// char* inet_itoa_n(uint32_t i);

// #ifdef ENABLE_UDS
// #define SOCKADDR_STRLEN sizeof(((struct sockaddr_un*)(NULL))->sun_path)
// static inline void sockaddr_set_path(sockaddr_u* addr, const char* path) {
//     addr->sa.sa_family = AF_UNIX;
//     strncpy(addr->sun.sun_path, path, sizeof(addr->sun.sun_path));
// }
// #else
// #define SOCKADDR_STRLEN 64 // ipv4:port | [ipv6]:port
// #endif

// static inline void sockaddr_print(sockaddr_u* addr) {
//     char buf[SOCKADDR_STRLEN] = {0};
//     sockaddr_str(addr, buf, sizeof(buf));
//     puts(buf);
// }

// #define SOCKADDR_LEN(addr) sockaddr_len((sockaddr_u*)addr)
// #define SOCKADDR_STR(addr, buf) \
//     sockaddr_str((sockaddr_u*)addr, buf, sizeof(buf))
// #define SOCKADDR_PRINT(addr) sockaddr_print((sockaddr_u*)addr)

//-----------------------------base functions----------------------------------------------

// // socket -> setsockopt -> bind
// // @param type: SOCK_STREAM(tcp) SOCK_DGRAM(udp)
// // @return sockfd
// EXPORT int Bind(int port, const char* host DEFAULT(ANYADDR),
//                 int type DEFAULT(SOCK_STREAM));

// // Bind -> listen
// // @return listenfd
// EXPORT int Listen(int port, const char* host DEFAULT(ANYADDR));

// // @return connfd
// // ResolveAddr -> socket -> nonblocking -> connect
// EXPORT int Connect(const char* host, int port, int nonblock DEFAULT(0));
// // Connect(host, port, 1)
// EXPORT int ConnectNonblock(const char* host, int port);
// // Connect(host, port, 1) -> select -> blocking
// #define DEFAULT_CONNECT_TIMEOUT 10000 // ms
// EXPORT int ConnectTimeout(const char* host, int port,
//                           int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));

// #ifdef ENABLE_UDS
// EXPORT int BindUnix(const char* path, int type DEFAULT(SOCK_STREAM));
// EXPORT int ListenUnix(const char* path);
// EXPORT int ConnectUnix(const char* path, int nonblock DEFAULT(0));
// EXPORT int ConnectUnixNonblock(const char* path);
// EXPORT int ConnectUnixTimeout(const char* path,
//                               int ms DEFAULT(DEFAULT_CONNECT_TIMEOUT));
// #endif

// EXPORT int Socketpair(int family, int type, int protocol, int sv[2]);

/************************** <unistd.h> ***************************************/

ssize_t Readline(int, void*, size_t);
/* Read "n" bytes from a descriptor. */
ssize_t readn(int, void*, size_t);
// ssize_t read_fd(int, void *, size_t, int *);
/* Write "n" bytes to a descriptor. */
ssize_t writen(int, const void*, size_t);
// ssize_t write_fd(int, void *, size_t, int);

/************************** High-level API ***************************************/
// getsockname
// getpeername
// setsockname => bind
// setpeername => connect

// 服务端建立后，后续accept需要知道客户端addrlen长度，因此需要返回。
int tcp_listen(const char*, const char*, socklen_t*);
// 客户端建立连接后，客户端后续收发直接read/write就不需要地址了。
int tcp_connect(const char*, const char*);  
// 服务端建立后，后续收发recvfrom需要知道addrlen长度，因此需要返回。
int udp_server(const char*, const char*, socklen_t*);
// 客户端创建后，后续收发还需要客户端地址，因此需要返回地址结构。
int udp_client(const char*, const char*, struct sockaddr**, socklen_t*);
// 客户端创建后建立连接后，客户端后续收发直接read/write就不需要地址了。
int udp_connect(const char*, const char*);

int sockfd_to_family(int sockfd);

#endif // !__SOCKET_H__
