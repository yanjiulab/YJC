#ifndef INET_H_
#define INET_H_

#include <arpa/inet.h>
#include <net/if_arp.h>       // ARP
#include <netinet/ether.h>    // Ethernet
#include <netinet/igmp.h>     // IGMP
#include <netinet/in.h>       // IP address
#include <netinet/ip.h>       // IPv4
#include <netinet/ip6.h>      // IPv6
#include <netinet/ip_icmp.h>  // ICMP
#include <netinet/tcp.h>      // TCP
#include <netinet/udp.h>      // UDP
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

typedef struct ethhdr ethhdr_t;
typedef struct iphdr iphdr_t;
typedef struct udphdr udphdr_t;
typedef struct tcphdr tcphdr_t;
typedef struct icmphdr icmphdr_t;
typedef struct icmp icmp_t;

/* Convert a Internet address in binary network format for interface
   type AF in buffer starting at CP to presentation form and return
   result.  */
char *inet_fmt(const struct sockaddr *sockaddr);

char *sock_ntop_host(const struct sockaddr *sa, socklen_t salen);

// int <-> string
char *sock_itop(in_addr_t);
in_addr_t sock_ptoi(const char *);

// wrapper function
char *Sock_ntop(const struct sockaddr *sockaddr, socklen_t addrlen);

// sockopt

#endif