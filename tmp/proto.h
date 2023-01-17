#ifndef PROTO_H
#define PROTO_H

#include <linux/if_packet.h>
#include <net/if_arp.h>       // ARP
#include <netinet/ether.h>    // Ethernet protocol, including net/ethernet
#include <netinet/igmp.h>     // IGMP
#include <netinet/ip.h>       // IPv4
#include <netinet/ip6.h>      // IPv6
#include <netinet/ip_icmp.h>  // ICMP
#include <netinet/tcp.h>      // TCP
#include <netinet/udp.h>      // UDP

// #define VERBOSE 1
#define HDRBUFLEN 4096

enum { FMT_V, FMT_VV };

struct packet_ {
} packet;

struct packet_stats {
    int recv_eth;
    int recv_arp;
    int recv_ip;
    int recv_udp;
    int recv_tcp;
    int send_eth;
    int send_arp;
    int send_ip;
    int send_udp;
    int send_tcp;
} pkt_stats;

void print_frame(int len);
void print_ethhdr(struct ethhdr *eth, int mode);      // layer 2
void print_arphdr(struct arphdr *arph, int mode);     // layer 3 (2.5)
void print_iphdr(struct iphdr *iph, int mode);        // layer 3
void print_icmphdr(struct icmphdr *icmph, int mode);  // layer 4
void print_igmp(struct igmp *igmp, int mode);         // layer 4
// void print_pimhdr(struct pimhdr *pim,int mode);      // layer 4
void print_udphdr(struct udphdr *udph, int mode);  // layer 4
void print_tcphdr(struct tcphdr *tcph, int mode);  // layer 4
// void print_httphdr(struct httphdr *httph,int mode);  // layer 5
void print_payload(unsigned char *recvbuf, int len, int mode);
void print_data(unsigned char *recvbuf, int len);

struct ethhdr *parse_ethhdr(unsigned char *recvbuf, int len);
struct arphdr *parse_arp(unsigned char *recvbuf, int len);
struct iphdr *parse_iphdr(unsigned char *recvbuf, int len);
struct icmphdr *parse_icmphdr(unsigned char *recvbuf, int len);
struct igmp *parse_igmp(unsigned char *recvbuf, int len);
struct pimhdr *parse_pim(unsigned char *recvbuf, int len);  // TODO
struct udphdr *parse_udphdr(unsigned char *recvbuf, int len);
struct tcphdr *parse_tcphdr(unsigned char *recvbuf, int len);
struct httphdr *parse_httphdr(unsigned char *recvbuf, int len);

unsigned char *wrap_ethhdr(unsigned char *sendbuf, int len);

#endif  // !PROTO_H