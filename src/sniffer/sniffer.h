#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include "packet.h"
#include "packet_socket.h"

typedef enum sniffer_record_e {
    SNIFFER_RECORD_PACKET,
    SNIFFER_RECORD_LIST,
    SNIFFER_RECORD_PCAP,
    SNIFFER_RECORD_ALL
} sniffer_record_t;

struct sniffer {
    packet_socket_t *psock;

    // record
    sniffer_record_t record_type;
    uint32_t record_max;
    packet_t *packet;
    packet_list_t *packet_list;
    FILE *pcap_file;

    // statistics
    uint32_t total_pkt;
    uint32_t total_byte;
};

typedef struct sniffer sniffer_t;

sniffer_t *sniffer_new(char* device_name, enum direction_t direction);
sniffer_t *sniffer_free();

#define sniffer_fd(snif) (snif->psock->packet_fd)
#define sniffer_ifnam(snif) (snif->psock->name)
#define sniffer_ifidx(snif) (snif->psock->name)

#endif // _SNIFFER_H_