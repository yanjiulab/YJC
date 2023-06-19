/* ip.h */

#ifndef __IP_H__
#define __IP_H__

#include <netinet/ip.h>   // IPv4
#include <netinet/ip6.h>  // IPv6

typedef struct iphdr ipv4_t;
typedef struct ip6_hdr ipv6_t;

#endif /* __IP_H__ */