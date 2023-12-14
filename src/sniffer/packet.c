#include "packet.h"

#include "defs.h"
#include "ethernet.h"
#include "ip.h"
/* ---------------------------- packet ---------------------------- */

struct packet* packet_new(uint32_t buffer_bytes) {
    struct packet* packet = calloc(1, sizeof(struct packet));
    packet->buffer = calloc(1, buffer_bytes);
    packet->buffer_bytes = buffer_bytes;
    return packet;
}

void packet_free(struct packet* packet) {
    free(packet->buffer);
    memset(packet, 0, sizeof(*packet)); /* paranoia to help catch bugs */
    free(packet);
}

/* ---------------------------- packet list---------------------------- */
struct packet_list* packet_list_new(void) {
    struct packet_list* list = calloc(1, sizeof(struct packet_list));
    list->packet = NULL;
    list->next = NULL;
    return list;
}

void packet_list_free(struct packet_list* list) {
    while (list != NULL) {
        struct packet_list* dead_list = list;
        if (list->packet)
            packet_free(list->packet);
        list = list->next;
        free(dead_list);
    }
}