#ifndef NAME_H
#define NAME_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>

void print_addrinfo(struct addrinfo *res);
struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype);

#endif  // !NAME_H
