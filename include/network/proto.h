#ifndef PROTO_H
#define PROTO_H

#define VERBOSE 1

#include <netinet/ether.h>  // ether protocol, including net/ethernet
#include <netinet/ip.h>     // iphdr

void print_ethhdr(struct ethhdr *eth, int mode);
void print_iphdr();


#endif // !PROTO_H