#ifndef IPA_H
#define IPA_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

// structure -> string
char *sock_ntop(const struct sockaddr *sockaddr, socklen_t addrlen);
char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

// int <-> string
char *sock_itop(in_addr_t);
in_addr_t sock_ptoi(const char *);

// wrapper function
char *Sock_ntop(const struct sockaddr *sockaddr, socklen_t addrlen);

#endif