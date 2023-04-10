#include "inet.h"
#define HDRSTR_VERBOSE 1
char buf[HDRSTRLEN];

char* ethhdr_str(ethhdr_t* eth) {
#ifdef HDRSTR_VERBOSE
    sprintf(buf,
            "> Ethernet II\n"
            "    |-Destination : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n"
            "    |-Source      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n"
            "    |-Type        : %s (0x%.4x)\n",
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
            eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
            "IPv4", ntohs((unsigned short)eth->h_proto));
#else
    sprintf(buf, "+ Ethernet II, %.2X-%.2X-%.2X-%.2X-%.2X-%.2X -> %.2X-%.2X-%.2X-%.2X-%.2X-%.2X (0x%.4x)\n",
            eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5],
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5],
            ntohs((unsigned short)eth->h_proto));
#endif
    return buf;
}

char* arp_str(arp_t* arp) {
#ifdef HDRSTR_VERBOSE

#else
#endif
    return buf;
}

char* iphdr_str(struct iphdr* iph) {
#ifdef HDRSTR_VERBOSE
    sprintf(buf,
            "> Internet Protocol Version 4\n"
            "    |-Version          : %d\n"
            "    |-Header Length    : %d Bytes\n"
            "    |-Type Of Service  : %d\n"
            "    |-Total Length     : %d Bytes\n"
            "    |-Identification   : %d\n"
            "    |-Fragment offset  : %d\n"
            "    |-TTL              : %d\n"
            "    |-Protocol         : %d\n"
            "    |-Checksum         : %d\n"
            "    |-Source IP        : %s\n"
            "    |-Destination IP   : %s\n",
            (unsigned int)iph->version, (unsigned int)((iph->ihl) * 4), (unsigned int)iph->tos, ntohs(iph->tot_len),
            ntohs(iph->id), ntohs(iph->frag_off), (unsigned int)iph->ttl, (unsigned int)iph->protocol,
            ntohs(iph->check), inet_itoa_n(iph->saddr), inet_itoa_n(iph->daddr));
#else
    sprintf(buf, "+ IPv4, %s -> %s (0x%.2x)\n", inet_itoa_n(iph->saddr), inet_itoa_n(iph->daddr),
            (unsigned int)iph->protocol);
#endif
    return buf;
}

char* udphdr_str(udphdr_t* udph) {
#ifdef HDRSTR_VERBOSE
    sprintf(buf,
            "> User Datagram Protocol\n"
            "    |-Source Port      : %d\n"
            "    |-Destination Port : %d\n"
            "    |-Length           : %d\n"
            "    |-Checksum         : %d\n", ntohs(udph->source), ntohs(udph->dest), ntohs(udph->len),
            ntohs(udph->check));
#else
    sprintf(buf, "+ UDP, %d -> %d\n", ntohs(udph->source), ntohs(udph->dest));
#endif
    return buf;
}

char* tcphdr_str(tcphdr_t* tcph) {
#ifdef HDRSTR_VERBOSE

#else
#endif
    return buf;
}

void print_data(unsigned char* data, int size) {
#if HDRSTR_VERBOSE
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
#else
    int i, j;
    for (i = 0; i < size; i++) {
        if (i != 0 && i % 20 == 0) printf("\n");
        if (i % 20 == 0) printf("   ");
        printf(" %02X", (unsigned int)data[i]);
        if (i == size - 1) printf("\n");
    }
#endif
}

uint16_t checksum(uint8_t* buf, int len) {
    unsigned int sum = 0;
    uint16_t* ptr = (uint16_t*)buf;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len) {
        sum += *(uint8_t*)ptr;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
};
