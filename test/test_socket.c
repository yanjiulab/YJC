#include "test.h"
#include "socket.h"
#include "sockunion.h"
#include "ipaddr.h"

void test_socket() {

    printf("usage: ./bin/t.out [ <host> ] <service or port>\n");

    int listenfd, connfd, n, flags;

    socklen_t clilen;
    listenfd = tcp_listen(ANYADDR, "6848", &clilen);

    struct sockaddr_in sa;
    union sockunion su;
    connfd = accept(listenfd, (SA*)&su, &clilen);
    printf("%x:%d\n", su.sin.sin_addr.s_addr, ntohs(su.sin.sin_port));

    ipaddr_t ip;
    uint16_t port;
    printf("%d\n", clilen);
    ip_from_sockaddr(&su, clilen, &ip, &port);
    printf("%s:%d\n", ip_to_string(&ip, NULL), port);

    // getsockname(listenfd, (SA*)&sa, &len);
    // printf("%x:%d\n", sa.sin_addr.s_addr, ntohs(sa.sin_port));
    // getpeername(listenfd, (SA*)&sa, &len);
    // printf("%x:%d\n", sa.sin_addr.s_addr, ntohs(sa.sin_port));
    // getsockname(connfd, (SA*)&sa, &len);
    // printf("%x:%d\n", sa.sin_addr.s_addr, ntohs(sa.sin_port));
    // getpeername(connfd, (SA*)&sa, &len);
    // printf("%x:%d\n", sa.sin_addr.s_addr, ntohs(sa.sin_port));
}