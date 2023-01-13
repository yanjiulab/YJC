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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "sock.h"
//-----------------------------headers----------------------------------------------
typedef struct ethhdr ethhdr_t;
typedef struct arphdr arphdr_t;
typedef struct ether_arp arp_t;
typedef struct iphdr iphdr_t;
typedef struct icmphdr icmphdr_t;
typedef struct icmp icmp_t;
typedef struct udphdr udphdr_t;
typedef struct tcphdr tcphdr_t;

#define HDRSTRLEN 4096

char* ethhdr_str(ethhdr_t* eth);
char* arp_str(arp_t* arp);
char* iphdr_str(iphdr_t* ip);
char* udphdr_str(udphdr_t* udp);
char* tcphdr_str(tcphdr_t* tcp);
void print_data(unsigned char* data, int size);

//-----------------------------utils----------------------------------------------
uint16_t checksum(uint8_t* buf, int len);

#endif