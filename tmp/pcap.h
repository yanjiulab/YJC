#ifndef PCAP_H
#define PCAP_H

#include <linux/filter.h>
#include <libpcap.h>
typedef struct pcap {
    int fd;
    unsigned int bufsize;
    void *buffer;
} pcap_t;

struct pcap_stat {};

char *pcap_lookupdev();

int pcap_setfilter(pcap_t *p /*, struct bpf*/);

int pcap_stats(pcap_t *p, struct pcap_stat *ps);

#endif  // !PCAP_H