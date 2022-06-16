#include "proto.h"

#include <stdio.h>

#include "ipa.h"

long tot_recv_num;

void print_frame(int len) { printf("+ Frame %ld: %d Bytes captrued (%d bits)\n", ++tot_recv_num, len, len * 8); }

void print_ethhdr(struct ethhdr *eth, int mode) {
    char buf[HDRBUFLEN];
    if (mode) {
        sprintf(buf,
                "> Ethernet II\n"
                "    |-Destination : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n"
                "    |-Source      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n"
                "    |-Type        : %s (0x%.4x)\n",
                eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
                eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4],
                eth->h_source[5], "IPv4", ntohs((unsigned short)eth->h_proto));

    } else {
        sprintf(buf, "+ Ethernet II, %.2X-%.2X-%.2X-%.2X-%.2X-%.2X -> %.2X-%.2X-%.2X-%.2X-%.2X-%.2X (0x%.4x)\n",
                eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4],
                eth->h_source[5], eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4],
                eth->h_dest[5], ntohs((unsigned short)eth->h_proto));
    }

    printf(buf);
}

void print_iphdr(struct iphdr *iph, int mode) {
    char buf[HDRBUFLEN];

    if (mode == VERBOSE) {
        sprintf(buf,
                "> Internet Protocol Version 4\n"
                "    |-Version          : %d\n"
                "    |-Header Length    : %d Bytes\n"
                "    |-Type Of Service  : %d\n"
                "    |-Total Length     : %d Bytes\n"
                "    |-Identification   : %d\n "
                "    |-Fragment offset  : %d\n"
                "    |-TTL              : %d\n"
                "    |-Protocol         : %d\n"
                "    |-Checksum         : %d\n"
                "    |-Source IP        : %s\n "
                "    |-Destination IP   : %s\n",
                (unsigned int)iph->version, (unsigned int)((iph->ihl) * 4), (unsigned int)iph->tos, ntohs(iph->tot_len),
                ntohs(iph->id), ntohs(iph->frag_off), (unsigned int)iph->ttl, (unsigned int)iph->protocol,
                ntohs(iph->check), sock_itop(iph->saddr), sock_itop(iph->daddr));
    } else {
        sprintf(buf, "+ IPv4, %s -> %s (0x%.2x)\n", sock_itop(iph->saddr), sock_itop(iph->daddr));
    }

    printf(buf);
}

void print_data(unsigned char *data, int size) {
    printf("> Data Payload\n");
    int i, j;
    for (i = 0; i < size; i++) {
        if (i != 0 && i % 16 == 0) {
            printf("         ");
            for (j = i - 16; j < i; j++) {
                if (data[j] >= 32 && data[j] <= 128) {
                    printf("%c", (unsigned char)data[j]);
                } else {
                    printf(".");  // otherwise print a dot
                }
            }
            printf("\n");
        }
        if (i % 16 == 0) {
            printf("   ");
        }
        printf(" %02X", (unsigned int)data[i]);
        if (i == size - 1)  // print the last spaces
        {
            for (j = 0; j < 15 - i % 16; j++) {
                printf("   ");  // extra spaces
            }
            printf("         ");
            for (j = i - i % 16; j <= i; j++) {
                if (data[j] >= 32 && data[j] <= 128) {
                    printf("%c", (unsigned char)data[j]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
}

struct ethhdr *parse_ethhdr(unsigned char *recvbuf, int len) {
    struct ethhdr *ethh = NULL;
    if (len < sizeof(struct ethhdr)) {
        printf("ether header bad len\n");
        return recvbuf;
    }

    ethh = (struct ethhdr *)recvbuf;
}