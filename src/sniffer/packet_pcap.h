#ifndef __PACKET_PCAP_H__
#define __PACKET_PCAP_H__

#include "packet.h"

/* PCAP related */
#define PCAP_MAGIC 0xa1b2c3d4
#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4
#define PCAP_SNAPLEN_MAX 0x40000

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

/**
 * @brief Truncate pcap file to zero length or create pcap file for writing.
 *
 * @param filename
 * @return FILE*
 */
extern FILE* pcap_open(char* filename);

/**
 * @brief Close pcap file.
 * 
 * @param pf 
 */
extern void pcap_close(FILE* pf);

/**
 * @brief Append a packet to an opened pcap file.
 * 
 * @param p
 * @param pf 
 * @return int 
 */
extern int packet_add_pcap(struct packet* p, FILE* pf);

/**
 * @brief Append a packet to a pcap file if it exists, otherwise create pcap file and writing.
 * 
 * @param p 
 * @param filename 
 * @return int 
 */
extern int packet_to_pcap(struct packet* p, char* filename);

#endif