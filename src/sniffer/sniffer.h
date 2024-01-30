#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include "base.h"
#include "packet.h"
#include "packet_pcap.h"
#include "packet_socket.h"

#define SNIFFER_RECORD_DEFAULT 65536
#define SNIFFER_PCAP_PATH "./capture"

typedef enum sniffer_record_e {
    SNIFFER_RECORD_PACKET,
    SNIFFER_RECORD_LIST,
    SNIFFER_RECORD_PCAP,
    SNIFFER_RECORD_ALL
} sniffer_record_t;

#define SNIFFER_OK 1
#define SNIFFER_ERROR -1

struct sniffer {
    packet_socket_t* psock;
    enum direction_t direction;
    uint32_t flag;
    packet_t* packet;
    int packet_len;

    // record
    sniffer_record_t record_type;
    uint32_t record_max;
    packet_list_t* packet_list;
    FILE* pcap_file;

    // statistics
    uint32_t total_pkt;
    uint32_t total_byte;
};

typedef struct sniffer sniffer_t;

sniffer_t* sniffer_new(char* device_name);
int sniffer_recv(sniffer_t* sniffer);
void sniffer_free();

// int sniffer_stop();

static inline void sniffer_set_direction(sniffer_t* s, enum direction_t d) {
    s->direction = d;
}
int sniffer_set_record(sniffer_t* sniffer, sniffer_record_t type,
                       uint32_t max, const char* pcapf);
int sniffer_set_filer();
int sniffer_set_recv_cb();
int sniffer_set_parse_cb();

#define sniffer_fd(snif) (snif->psock->packet_fd)
#define sniffer_ifnam(snif) (snif->psock->name)
#define sniffer_ifidx(snif) (snif->psock->name)

#endif // _SNIFFER_H_