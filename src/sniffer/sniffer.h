#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include "base.h"
#include "packet.h"
#include "packet_parser.h"
#include "packet_pcap.h"
#include "packet_socket.h"
#include "packet_stringify.h"
#include "socket.h"

#define SNIFFER_RECORD_DEFAULT 65536
#define SNIFFER_PCAP_PATH "./capture"

typedef enum sniffer_record_e {
    SNIFFER_RECORD_PACKET,
    SNIFFER_RECORD_LIST,
    SNIFFER_RECORD_PCAP,
    SNIFFER_RECORD_ALL
} sniffer_record_t;

typedef enum {
    SNIFFER_STOP,
    SNIFFER_RUNNING,
    SNIFFER_PAUSE,
} sniffer_status_t;

#define SNIFFER_OK 1
#define SNIFFER_ERROR -1

struct sniffer {
    packet_socket_t* psock;
    enum direction_t direction;
    sniffer_status_t status;
    uint32_t flag;
    packet_t* packet;
    int packet_len;

    // record
    sniffer_record_t record;
    uint32_t record_num;
    packet_list_t* packet_list;
    FILE* pcap_file;

    // statistics
    uint32_t total_pkt;
    uint32_t total_byte;
};

typedef struct sniffer sniffer_t;

sniffer_t* sniffer_new(char* device_name);
int sniffer_recv(sniffer_t* sniffer);
void sniffer_free(sniffer_t* sniffer);
int sniffer_start(sniffer_t* sniffer);
int sniffer_pause(sniffer_t* sniffer);
int sniffer_stop(sniffer_t* sniffer);

// int sniffer_stop();

static inline void sniffer_set_direction(sniffer_t* s, enum direction_t d) {
    s->direction = d;
}
int sniffer_set_record(sniffer_t* sniffer, sniffer_record_t type,
                       uint32_t max, const char* pcapf);
int sniffer_set_filter(sniffer_t* sniffer, struct sock_filter* filter, int len);
int sniffer_set_filter_str(sniffer_t* sniffer, const char* fs);
// ether proto [proto]
int sniffer_set_filter_eth_proto(sniffer_t* sniffer, uint16_t proto);
// ether[pos]=[val]
int sniffer_set_filter_eth_byte(sniffer_t* sniffer, uint32_t pos, uint8_t val);
//
int sniffer_set_recv_cb();
int sniffer_set_parse_cb();

#define sniffer_fd(snif) (snif->psock->packet_fd)
#define sniffer_ifnam(snif) (snif->psock->name)
#define sniffer_ifidx(snif) (snif->psock->name)

#endif // _SNIFFER_H_