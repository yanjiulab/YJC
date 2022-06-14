#include "sock.h"

#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <string.h>

#include "debug.h"
#include "ifi.h"
#include "name.h"
#include "wrapper.h"

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
const char *str_fam(int family) {
    if (family == AF_INET) return ("AF_INET");
    if (family == AF_INET6) return ("AF_INET6");
    if (family == AF_LOCAL) return ("AF_LOCAL");
    return ("<unknown family>");
}

const char *str_sock(int socktype) {
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