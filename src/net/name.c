#include "name.h"

#include "inet.h"
#include "socket.h"

struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype) {
    int n;
    struct addrinfo hints, *res;

    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_flags = AI_CANONNAME; /* always return canonical name */
    hints.ai_family = family;      /* AF_UNSPEC, AF_INET, AF_INET6, etc. */
    hints.ai_socktype = socktype;  /* 0, SOCK_STREAM, SOCK_DGRAM, etc. */

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) return (NULL);

    return (res); /* return pointer to first on linked list */
}

void print_addrinfo(struct addrinfo *res) {
    while (res) {
        printf("socket(%s, %s, %d)\n", str_fam(res->ai_family), str_sock(res->ai_socktype), res->ai_protocol);

        if (res->ai_flags & AI_CANONNAME) {
            if (res->ai_canonname) {
                printf("\tai_canonname = %s\n", res->ai_canonname);
            }
        }

        printf("\taddress: %s\n", inet_fmt(res->ai_addr));

        res = res->ai_next;
    }

    // do {
    //     printf("\nsocket(%s, %s, %d)", str_fam(res->ai_family),
    // 				str_sock(res->ai_socktype), res->ai_protocol);
    // } while ((res = res->ai_next) != NULL);
}