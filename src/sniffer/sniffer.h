#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include "packet_socket.h"
#include "packet.h"

struct sniffer {
    packet_socket_t *psock;
};

typedef struct sniffer sniffer_t;

sniffer_t *sniffer_new();
sniffer_t *sniffer_free();

#define sniffer_ifnam(snif) (snif->psock->name)
#define sniffer_ifidx(snif) (snif->psock->name)

#endif // _SNIFFER_H_