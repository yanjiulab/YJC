#ifndef __PACKET_DUMP_H__
#define __PACKET_DUMP_H__

#include "packet.h"

typedef enum dump_format_e {
    DUMP_PCAP,
    DUMP_TXT_HEX,
    DUMP_STR_SHORT,   /* brief format used in scripts */
    DUMP_STR_FULL,    /* add local and remote address and port */
    DUMP_STR_VERBOSE, /* add hex dump */
} dump_format_tt;     // TODO: rename to _t

/* PCAP related */
#define PCAP_MAGIC 0xa1b2c3d4
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4

typedef struct pcap_file_header_s {
    uint32_t magic; // 0xa1b2c3d4
    uint16_t version_major;
    uint16_t version_minor;
    int32_t thiszone;
    uint32_t sigfigs;
    uint32_t snaplen; // max len saved portion of each packet
    uint32_t linktype;
} pcap_file_header_t;

// struct timeval is uint64
struct pcap_timeval {
    uint32_t tv_sec;
    uint32_t tv_usec;
};

typedef struct pcap_pkthdr_s {
    struct pcap_timeval ts;
    uint32_t caplen; // length of portion present
    uint32_t len;    // length of this packet (off wire)
} pcap_pkthdr_t;

extern int packet_dump_pcap_from_hex(uint8_t* hex, dump_format_tt format, char* filename);

extern int packet_dump_pcap(struct packet* p, char* filename);
int pcap_file_write(FILE* pf, struct packet* p);
void pcap_file_free(FILE* pf);
FILE* pcap_file_new(char* filename);
bool is_valid_pcap(char* filename);

#endif