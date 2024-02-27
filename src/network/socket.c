#include "socket.h"
#include "sockopt.h"

const char* socket_strerror(int err) {
    return strerror(ABS(err));
}

static inline int socket_errno_negative() {
    int err = socket_errno();
    return err > 0 ? -err : -1;
}

// dup with string_to_ip
// int ResolveAddr(const char* host, sockaddr_u* addr) {
//     if (inet_pton(AF_INET, host, &addr->sin.sin_addr) == 1) {
//         addr->sa.sa_family = AF_INET; // host is ipv4, so easy ;)
//         return 0;
//     }

//     if (inet_pton(AF_INET6, host, &addr->sin6.sin6_addr) == 1) {
//         addr->sa.sa_family = AF_INET6; // host is ipv6
//     }

//     struct addrinfo* ais = NULL;
//     int ret = getaddrinfo(host, NULL, NULL, &ais);
//     if (ret != 0 || ais == NULL || ais->ai_addr == NULL || ais->ai_addrlen == 0) {
//         printf("unknown host: %s err:%d:%s\n", host, ret, gai_strerror(ret));
//         return ret;
//     }
//     struct addrinfo* pai = ais;
//     while (pai != NULL) {
//         if (pai->ai_family == AF_INET)
//             break;
//         pai = pai->ai_next;
//     }
//     if (pai == NULL)
//         pai = ais;
//     memcpy(addr, pai->ai_addr, pai->ai_addrlen);
//     freeaddrinfo(ais);
//     return 0;
// }

// const char* sockaddr_ip(sockaddr_u* addr, char* ip, int len) {
//     if (addr->sa.sa_family == AF_INET) {
//         return inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
//     } else if (addr->sa.sa_family == AF_INET6) {
//         return inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
//     }
//     return ip;
// }

// uint16_t sockaddr_port(sockaddr_u* addr) {
//     uint16_t port = 0;
//     if (addr->sa.sa_family == AF_INET) {
//         port = ntohs(addr->sin.sin_port);
//     } else if (addr->sa.sa_family == AF_INET6) {
//         port = ntohs(addr->sin6.sin6_port);
//     }
//     return port;
// }

// int sockaddr_set_ip(sockaddr_u* addr, const char* host) {
//     if (!host || *host == '\0') {
//         addr->sin.sin_family = AF_INET;
//         addr->sin.sin_addr.s_addr = htonl(INADDR_ANY);
//         return 0;
//     }
//     return ResolveAddr(host, addr);
// }

// void sockaddr_set_port(sockaddr_u* addr, int port) {
//     if (addr->sa.sa_family == AF_INET) {
//         addr->sin.sin_port = htons(port);
//     } else if (addr->sa.sa_family == AF_INET6) {
//         addr->sin6.sin6_port = htons(port);
//     }
// }

// int sockaddr_set_ipport(sockaddr_u* addr, const char* host, int port) {
// #ifdef ENABLE_UDS
//     if (port < 0) {
//         sockaddr_set_path(addr, host);
//         return 0;
//     }
// #endif
//     int ret = sockaddr_set_ip(addr, host);
//     if (ret != 0)
//         return ret;
//     sockaddr_set_port(addr, port);
//     // SOCKADDR_PRINT(addr);
//     return 0;
// }

// socklen_t sockaddr_len(sockaddr_u* addr) {
//     if (addr->sa.sa_family == AF_INET) {
//         return sizeof(struct sockaddr_in);
//     } else if (addr->sa.sa_family == AF_INET6) {
//         return sizeof(struct sockaddr_in6);
//     }
// #ifdef ENABLE_UDS
//     else if (addr->sa.sa_family == AF_UNIX) {
//         return sizeof(struct sockaddr_un);
//     }
// #endif
//     return sizeof(sockaddr_u);
// }

// char* inet_itoa_n(uint32_t i) {
//     struct in_addr ia = {0};
//     char* r;
//     ia.s_addr = i;
//     r = inet_ntoa(ia);
//     return strdup(r);
// }

// char* inet_itoa_h(uint32_t i) {
//     struct in_addr ia = {0};
//     char* r;
//     ia.s_addr = htonl(i);
//     r = inet_ntoa(ia);
//     return strdup(r);
// }

// const char* sockaddr_str(sockaddr_u* addr, char* buf, int len) {
//     char ip[SOCKADDR_STRLEN] = {0};
//     uint16_t port = 0;
//     if (addr->sa.sa_family == AF_INET) {
//         inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
//         port = ntohs(addr->sin.sin_port);
//         snprintf(buf, len, "%s:%d", ip, port);
//     } else if (addr->sa.sa_family == AF_INET6) {
//         inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
//         port = ntohs(addr->sin6.sin6_port);
//         snprintf(buf, len, "[%s]:%d", ip, port);
//     }
// #ifdef ENABLE_UDS
//     else if (addr->sa.sa_family == AF_UNIX) {
//         snprintf(buf, len, "%s", addr->sun.sun_path);
//     }
// #endif
//     return buf;
// }

// static int sockaddr_bind(sockaddr_u* localaddr, int type) {
//     // socket -> setsockopt -> bind
// #ifdef SOCK_CLOEXEC
//     type |= SOCK_CLOEXEC;
// #endif
//     int sockfd = socket(localaddr->sa.sa_family, type, 0);
//     if (sockfd < 0) {
//         perror("socket");
//         return socket_errno_negative();
//     }

//     so_reuseaddr(sockfd, 1);
//     // so_reuseport(sockfd, 1);

//     if (bind(sockfd, &localaddr->sa, sockaddr_len(localaddr)) < 0) {
//         perror("bind");
//         goto error;
//     }

//     return sockfd;
// error:
//     closesocket(sockfd);
//     return socket_errno_negative();
// }

// static int sockaddr_connect(sockaddr_u* peeraddr, int nonblock) {
//     // socket -> nonblocking -> connect
//     int connfd = socket(peeraddr->sa.sa_family, SOCK_STREAM, 0);
//     if (connfd < 0) {
//         perror("socket");
//         return socket_errno_negative();
//     }

//     if (nonblock) {
//         nonblocking(connfd);
//     }

//     int ret = connect(connfd, &peeraddr->sa, sockaddr_len(peeraddr));

//     if (ret < 0 && socket_errno() != EINPROGRESS) {
//         // perror("connect");
//         closesocket(connfd);
//         return socket_errno_negative();
//     }
//     return connfd;
// }

// static int ListenFD(int sockfd) {
//     if (sockfd < 0)
//         return sockfd;
//     if (listen(sockfd, SOMAXCONN) < 0) {
//         perror("listen");
//         closesocket(sockfd);
//         return socket_errno_negative();
//     }
//     return sockfd;
// }

// static int ConnectFDTimeout(int connfd, int ms) {
//     int err;
//     socklen_t optlen = sizeof(err);
//     struct timeval tv = {ms / 1000, (ms % 1000) * 1000};
//     fd_set writefds;
//     FD_ZERO(&writefds);
//     FD_SET(connfd, &writefds);
//     int ret = select(connfd + 1, 0, &writefds, 0, &tv);
//     if (ret < 0) {
//         perror("select");
//         goto error;
//     }
//     if (ret == 0) {
//         errno = ETIMEDOUT;
//         goto error;
//     }
//     if (getsockopt(connfd, SOL_SOCKET, SO_ERROR, (char*)&err, &optlen) < 0 || err != 0) {
//         goto error;
//     }
//     blocking(connfd);
//     return connfd;
// error:
//     closesocket(connfd);
//     return socket_errno_negative();
// }

// int Bind(int port, const char* host, int type) {
//     sockaddr_u localaddr;
//     memset(&localaddr, 0, sizeof(localaddr));
//     int ret = sockaddr_set_ipport(&localaddr, host, port);
//     if (ret != 0) {
//         return NABS(ret);
//     }
//     return sockaddr_bind(&localaddr, type);
// }

// int Listen(int port, const char* host) {
//     int sockfd = Bind(port, host, SOCK_STREAM);
//     if (sockfd < 0)
//         return sockfd;
//     return ListenFD(sockfd);
// }

// int Connect(const char* host, int port, int nonblock) {
//     sockaddr_u peeraddr;
//     memset(&peeraddr, 0, sizeof(peeraddr));
//     int ret = sockaddr_set_ipport(&peeraddr, host, port);
//     if (ret != 0) {
//         return NABS(ret);
//     }
//     return sockaddr_connect(&peeraddr, nonblock);
// }

// int ConnectNonblock(const char* host, int port) {
//     return Connect(host, port, 1);
// }

// int ConnectTimeout(const char* host, int port, int ms) {
//     int connfd = Connect(host, port, 1);
//     if (connfd < 0)
//         return connfd;
//     return ConnectFDTimeout(connfd, ms);
// }

// #ifdef ENABLE_UDS
// int BindUnix(const char* path, int type) {
//     sockaddr_u localaddr;
//     memset(&localaddr, 0, sizeof(localaddr));
//     sockaddr_set_path(&localaddr, path);
//     return sockaddr_bind(&localaddr, type);
// }

// int ListenUnix(const char* path) {
//     int sockfd = BindUnix(path, SOCK_STREAM);
//     if (sockfd < 0)
//         return sockfd;
//     return ListenFD(sockfd);
// }

// int ConnectUnix(const char* path, int nonblock) {
//     sockaddr_u peeraddr;
//     memset(&peeraddr, 0, sizeof(peeraddr));
//     sockaddr_set_path(&peeraddr, path);
//     return sockaddr_connect(&peeraddr, nonblock);
// }

// int ConnectUnixNonblock(const char* path) {
//     return ConnectUnix(path, 1);
// }

// int ConnectUnixTimeout(const char* path, int ms) {
//     int connfd = ConnectUnix(path, 1);
//     if (connfd < 0)
//         return connfd;
//     return ConnectFDTimeout(connfd, ms);
// }
// #endif

// int Socketpair(int family, int type, int protocol, int sv[2]) {
//     return socketpair(AF_LOCAL, type, protocol, sv);
// }

/************************** <unistd.h> ***************************************/
/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void* vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char* ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0; /* and call read() again */
            else
                return (-1);
        } else if (nread == 0)
            break; /* EOF */

        nleft -= nread;
        ptr += nread;
    }
    return (n - nleft); /* return >= 0 */
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void* vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char* ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ((nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return (-1); /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return (n);
}

ssize_t Readline(int fd, void* vptr, size_t maxlen) {
    ssize_t n, rc;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
    again:
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break; /* newline is stored, like fgets() */
        } else if (rc == 0) {
            *ptr = 0;
            return (n - 1); /* EOF, n - 1 bytes were read */
        } else {
            if (errno == EINTR)
                goto again;
            return (-1); /* error, errno set by read() */
        }
    }

    *ptr = 0; /* null terminate like fgets() */
    return (n);
}

/************************** High-level API ***************************************/
int tcp_listen(const char* host, const char* serv, socklen_t* addrlenp) {
    int listenfd, n;
    const int on = 1;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("tcp_listen error for %s, %s: %s", host, serv, gai_strerror(n));
    ressave = res;

    do {
        listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (listenfd < 0)
            continue; /* error, try next one */

        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        close(listenfd); /* bind error, close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno from final socket() or bind() */
        err_sys("tcp_listen error for %s, %s", host, serv);

    listen(listenfd, LISTENQ);

    if (addrlenp)
        *addrlenp = res->ai_addrlen; /* return size of protocol address */

    freeaddrinfo(ressave);

    return (listenfd);
}

int tcp_connect(const char* host, const char* serv) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
            continue; /* ignore this one */

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        close(sockfd); /* ignore this one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno set from final connect() */
        err_sys("tcp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);

    return (sockfd);
}

int udp_server(const char* host, const char* serv, socklen_t* addrlenp) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("udp_server error for %s, %s: %s",
                 host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
            continue; /* error - try next one */

        so_reuseaddr(sockfd, 1);
        so_reuseport(sockfd, 1);
        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        close(sockfd); /* bind error - close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno from final socket() or bind() */
        err_sys("udp_server error for %s, %s", host, serv);

    if (addrlenp)
        *addrlenp = res->ai_addrlen; /* return size of protocol address */

    freeaddrinfo(ressave);

    return (sockfd);
}

int udp_client(const char* host, const char* serv, struct sockaddr** saptr, socklen_t* lenp) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("udp_client error for %s, %s: %s",
                 host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd >= 0)
            break; /* success */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno set from final socket() */
        err_sys("udp_client error for %s, %s", host, serv);

    *saptr = malloc(res->ai_addrlen);
    memcpy(*saptr, res->ai_addr, res->ai_addrlen);
    *lenp = res->ai_addrlen;

    freeaddrinfo(ressave);

    return (sockfd);
}

int udp_connect(const char* host, const char* serv) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("udp_connect error for %s, %s: %s",
                 host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
            continue; /* ignore this one */

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
            break; /* success */

        close(sockfd); /* ignore this one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno set from final connect() */
        err_sys("udp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);

    return (sockfd);
}

int sockfd_to_family(int sockfd) {
    struct sockaddr_storage ss;
    socklen_t len;

    len = sizeof(ss);
    if (getsockname(sockfd, (SA*)&ss, &len) < 0)
        return (-1);
    return (ss.ss_family);
}

int family_to_level(int family) {
    switch (family) {
    case AF_INET:
        return IPPROTO_IP;
#ifdef IPV6
    case AF_INET6:
        return IPPROTO_IPV6;
#endif
    default:
        return -1;
    }
}