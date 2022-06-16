#include "sock.h"

#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <string.h>

#include "debug.h"
#include "ifi.h"
#include "name.h"

/* Unix wrapper functions */
#define UNIX_WRAPPER
void *Calloc(size_t n, size_t size) {
    void *ptr;

    if ((ptr = calloc(n, size)) == NULL) err_sys("calloc error");
    return (ptr);
}

void Close(int fd) {
    if (close(fd) == -1) err_sys("close error");
}

void Dup2(int fd1, int fd2) {
    if (dup2(fd1, fd2) == -1) err_sys("dup2 error");
}

int Fcntl(int fd, int cmd, int arg) {
    int n;

    if ((n = fcntl(fd, cmd, arg)) == -1) err_sys("fcntl error");
    return (n);
}

void Gettimeofday(struct timeval *tv, void *foo) {
    if (gettimeofday(tv, foo) == -1) err_sys("gettimeofday error");
    return;
}

int Ioctl(int fd, int request, void *arg) {
    int n;

    if ((n = ioctl(fd, request, arg)) == -1) err_sys("ioctl error");
    return (n); /* streamio of I_LIST returns value */
}

pid_t Fork(void) {
    pid_t pid;

    if ((pid = fork()) == -1) err_sys("fork error");
    return (pid);
}

void *Malloc(size_t size) {
    void *ptr;

    if ((ptr = malloc(size)) == NULL) err_sys("malloc error");
    return (ptr);
}

int Mkstemp(char *template) {
    int i;

#ifdef HAVE_MKSTEMP
    if ((i = mkstemp(template)) < 0) err_quit("mkstemp error");
#else
    if (mktemp(template) == NULL || template[0] == 0) err_quit("mktemp error");
    i = Open(template, O_CREAT | O_WRONLY, FILE_MODE);
#endif

    return i;
}

#include <sys/mman.h>

void *Mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset) {
    void *ptr;

    if ((ptr = mmap(addr, len, prot, flags, fd, offset)) == ((void *)-1)) err_sys("mmap error");
    return (ptr);
}

int Open(const char *pathname, int oflag, mode_t mode) {
    int fd;

    if ((fd = open(pathname, oflag, mode)) == -1) err_sys("open error for %s", pathname);
    return (fd);
}

void Pipe(int *fds) {
    if (pipe(fds) < 0) err_sys("pipe error");
}

ssize_t Read(int fd, void *ptr, size_t nbytes) {
    ssize_t n;

    if ((n = read(fd, ptr, nbytes)) == -1) err_sys("read error");
    return (n);
}

void Sigaddset(sigset_t *set, int signo) {
    if (sigaddset(set, signo) == -1) err_sys("sigaddset error");
}

void Sigdelset(sigset_t *set, int signo) {
    if (sigdelset(set, signo) == -1) err_sys("sigdelset error");
}

void Sigemptyset(sigset_t *set) {
    if (sigemptyset(set) == -1) err_sys("sigemptyset error");
}

void Sigfillset(sigset_t *set) {
    if (sigfillset(set) == -1) err_sys("sigfillset error");
}

int Sigismember(const sigset_t *set, int signo) {
    int n;

    if ((n = sigismember(set, signo)) == -1) err_sys("sigismember error");
    return (n);
}

void Sigpending(sigset_t *set) {
    if (sigpending(set) == -1) err_sys("sigpending error");
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oset) {
    if (sigprocmask(how, set, oset) == -1) err_sys("sigprocmask error");
}

char *Strdup(const char *str) {
    char *ptr;

    if ((ptr = strdup(str)) == NULL) err_sys("strdup error");
    return (ptr);
}

long Sysconf(int name) {
    long val;

    errno = 0; /* in case sysconf() does not change this */
    if ((val = sysconf(name)) == -1) err_sys("sysconf error");
    return (val);
}

#ifdef HAVE_SYS_SYSCTL_H
void Sysctl(int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp, size_t newlen) {
    if (sysctl(name, namelen, oldp, oldlenp, newp, newlen) == -1) err_sys("sysctl error");
}
#endif

void Unlink(const char *pathname) {
    if (unlink(pathname) == -1) err_sys("unlink error for %s", pathname);
}

pid_t Wait(int *iptr) {
    pid_t pid;

    if ((pid = wait(iptr)) == -1) err_sys("wait error");
    return (pid);
}

pid_t Waitpid(pid_t pid, int *iptr, int options) {
    pid_t retpid;

    if ((retpid = waitpid(pid, iptr, options)) == -1) err_sys("waitpid error");
    return (retpid);
}

void Write(int fd, void *ptr, size_t nbytes) {
    if (write(fd, ptr, nbytes) != nbytes) err_sys("write error");
}

/* stdio wrapper functions */
#define STDIO_WRAPPER
void Fclose(FILE *fp) {
    if (fclose(fp) != 0) err_sys("fclose error");
}

FILE *Fdopen(int fd, const char *type) {
    FILE *fp;

    if ((fp = fdopen(fd, type)) == NULL) err_sys("fdopen error");

    return (fp);
}

char *Fgets(char *ptr, int n, FILE *stream) {
    char *rptr;

    if ((rptr = fgets(ptr, n, stream)) == NULL && ferror(stream)) err_sys("fgets error");

    return (rptr);
}

FILE *Fopen(const char *filename, const char *mode) {
    FILE *fp;

    if ((fp = fopen(filename, mode)) == NULL) err_sys("fopen error");

    return (fp);
}

void Fputs(const char *ptr, FILE *stream) {
    if (fputs(ptr, stream) == EOF) err_sys("fputs error");
}

/* socket wrapper functions */
#define SOCKET_WRAPPER
int Socket(int family, int type, int protocol) {
    int n;

    if ((n = socket(family, type, protocol)) < 0) err_sys("socket error");
    return (n);
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen) {
    if (connect(fd, sa, salen) < 0) err_sys("connect error");
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen) {
    if (bind(fd, sa, salen) < 0) err_sys("bind error");
}

void Listen(int fd, int backlog) {
    char *ptr;

    /*4can override 2nd argument with environment variable */
    if ((ptr = getenv("LISTENQ")) != NULL) backlog = atoi(ptr);

    if (listen(fd, backlog) < 0) err_sys("listen error");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    int n;

again:
    if ((n = accept(fd, sa, salenptr)) < 0) {
#ifdef EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)
#else
        if (errno == ECONNABORTED)
#endif
            goto again;
        else
            err_sys("accept error");
    }
    return (n);
}

void Shutdown(int fd, int how) {
    if (shutdown(fd, how) < 0) err_sys("shutdown error");
}

// get remote peer address
void Getpeername(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    if (getpeername(fd, sa, salenptr) < 0) err_sys("getpeername error");
}

// get local assigned address
void Getsockname(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    if (getsockname(fd, sa, salenptr) < 0) err_sys("getsockname error");
}

void Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr) {
    if (getsockopt(fd, level, optname, optval, optlenptr) < 0) err_sys("getsockopt error");
}

void Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen) {
    if (setsockopt(fd, level, optname, optval, optlen) < 0) err_sys("setsockopt error");
}

ssize_t Recv(int fd, void *ptr, size_t nbytes, int flags) {
    ssize_t n;

    if ((n = recv(fd, ptr, nbytes, flags)) < 0) err_sys("recv error");
    return (n);
}

ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr) {
    ssize_t n;

    if ((n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0) err_sys("recvfrom error");
    return (n);
}

ssize_t Recvmsg(int fd, struct msghdr *msg, int flags) {
    ssize_t n;

    if ((n = recvmsg(fd, msg, flags)) < 0) err_sys("recvmsg error");
    return (n);
}

void Send(int fd, const void *ptr, size_t nbytes, int flags) {
    if (send(fd, ptr, nbytes, flags) != (ssize_t)nbytes) err_sys("send error");
}

void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen) {
    if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes) err_sys("sendto error");
}

void Sendmsg(int fd, const struct msghdr *msg, int flags) {
    unsigned int i;
    ssize_t nbytes;

    nbytes = 0; /* must first figure out what return value should be */
    for (i = 0; i < msg->msg_iovlen; i++) nbytes += msg->msg_iov[i].iov_len;

    if (sendmsg(fd, msg, flags) != nbytes) err_sys("sendmsg error");
}

#ifdef HAVE_INET6_RTH_INIT
int Inet6_rth_space(int type, int segments) {
    int ret;

    ret = inet6_rth_space(type, segments);
    if (ret < 0) err_quit("inet6_rth_space error");

    return ret;
}

void *Inet6_rth_init(void *rthbuf, socklen_t rthlen, int type, int segments) {
    void *ret;

    ret = inet6_rth_init(rthbuf, rthlen, type, segments);
    if (ret == NULL) err_quit("inet6_rth_init error");

    return ret;
}

void Inet6_rth_add(void *rthbuf, const struct in6_addr *addr) {
    if (inet6_rth_add(rthbuf, addr) < 0) err_quit("inet6_rth_add error");
}

void Inet6_rth_reverse(const void *in, void *out) {
    if (inet6_rth_reverse(in, out) < 0) err_quit("inet6_rth_reverse error");
}

int Inet6_rth_segments(const void *rthbuf) {
    int ret;

    ret = inet6_rth_segments(rthbuf);
    if (ret < 0) err_quit("inet6_rth_segments error");

    return ret;
}

struct in6_addr *Inet6_rth_getaddr(const void *rthbuf, int idx) {
    struct in6_addr *ret;

    ret = inet6_rth_getaddr(rthbuf, idx);
    if (ret == NULL) err_quit("inet6_rth_getaddr error");

    return ret;
}
#endif

#ifdef HAVE_KQUEUE
int Kqueue(void) {
    int ret;

    if ((ret = kqueue()) < 0) err_sys("kqueue error");
    return ret;
}

int Kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents,
           const struct timespec *timeout) {
    int ret;

    if ((ret = kevent(kq, changelist, nchanges, eventlist, nevents, timeout)) < 0) err_sys("kevent error");
    return ret;
}
#endif

#ifdef HAVE_POLL
int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout) {
    int n;

    if ((n = poll(fdarray, nfds, timeout)) < 0) err_sys("poll error");

    return (n);
}
#endif

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
    int n;

    if ((n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0) err_sys("select error");
    return (n); /* can return 0 on timeout */
}

int Sockatmark(int fd) {
    int n;

    if ((n = sockatmark(fd)) < 0) err_sys("sockatmark error");
    return (n);
}

void Socketpair(int family, int type, int protocol, int *fd) {
    int n;

    if ((n = socketpair(family, type, protocol, fd)) < 0) err_sys("socketpair error");
}

/* unp library functions */
#define UNP_FUNC

int tcp_listen(const char *host, const char *serv, socklen_t *addrlenp) {
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
        if (listenfd < 0) continue; /* error, try next one */

        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0) break; /* success */

        Close(listenfd); /* bind error, close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno from final socket() or bind() */
        err_sys("tcp_listen error for %s, %s", host, serv);

    Listen(listenfd, LISTENQ);

    if (addrlenp) *addrlenp = res->ai_addrlen; /* return size of protocol address */

    freeaddrinfo(ressave);

    return (listenfd);
}

int tcp_connect(const char *host, const char *serv) {
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
        if (sockfd < 0) continue; /* ignore this one */

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) break; /* success */

        Close(sockfd); /* ignore this one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno set from final connect() */
        err_sys("tcp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);

    return (sockfd);
}

int udp_server(const char *host, const char *serv, socklen_t *addrlenp) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("udp_server error for %s, %s: %s", host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) continue; /* error - try next one */

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0) break; /* success */

        Close(sockfd); /* bind error - close and try next one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno from final socket() or bind() */
        err_sys("udp_server error for %s, %s", host, serv);

    if (addrlenp) *addrlenp = res->ai_addrlen; /* return size of protocol address */

    freeaddrinfo(ressave);

    return (sockfd);
}

int udp_connect(const char *host, const char *serv) {
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0)
        err_quit("udp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) continue; /* ignore this one */

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) break; /* success */

        Close(sockfd); /* ignore this one */
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) /* errno set from final connect() */
        err_sys("udp_connect error for %s, %s", host, serv);

    freeaddrinfo(ressave);

    return (sockfd);
}

/* Utility */

/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

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
ssize_t writen(int fd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

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

/* our own library functions */
#define YJ_FUNC
int sock_packet(int type, int proto, const char *ifname) {
    int sockfd;

    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_ll));
    saddr.sll_family = PF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    if (ifname) saddr.sll_ifindex = if_nametoindex(ifname);

    bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
}

/* util functions */
#define UTIL_FUNC
char *str_fam(int family) {
    if (family == AF_INET) return ("AF_INET");
    if (family == AF_INET6) return ("AF_INET6");
    if (family == AF_LOCAL) return ("AF_LOCAL");
    return ("<unknown family>");
}

char *str_sock(int socktype) {
    switch (socktype) {
        case SOCK_STREAM:
            return "SOCK_STREAM";
        case SOCK_DGRAM:
            return "SOCK_DGRAM";
        case SOCK_RAW:
            return "SOCK_RAW";
#ifdef SOCK_RDM
        case SOCK_RDM:
            return "SOCK_RDM";
#endif
#ifdef SOCK_SEQPACKET
        case SOCK_SEQPACKET:
            return "SOCK_SEQPACKET";
#endif
        default:
            return "<unknown socktype>";
    }
}