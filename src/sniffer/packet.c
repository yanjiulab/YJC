#include "packet.h"

#include "defs.h"
#include "ethernet.h"
#include "ip.h"
/* ---------------------------- packet ---------------------------- */

struct packet *packet_new(uint32_t buffer_bytes) {
    struct packet *packet = calloc(1, sizeof(struct packet));
    packet->buffer = calloc(1, buffer_bytes);
    packet->buffer_bytes = buffer_bytes;
    return packet;
}

void packet_free(struct packet *packet) {
    free(packet->buffer);
    memset(packet, 0, sizeof(*packet)); /* paranoia to help catch bugs */
    free(packet);
}

struct packet_list *packet_list_new(void) {
    struct packet_list *list = calloc(1, sizeof(struct packet_list));
    list->packet = NULL;
    list->next = NULL;
    return list;
}

void packet_list_free(struct packet_list *list) {
    while (list != NULL) {
        struct packet_list *dead_list = list;
        if (list->packet) packet_free(list->packet);
        list = list->next;
        free(dead_list);
    }
}

int packet_header_count(const struct packet *packet) {
    int i;

    for (i = 0; i < ARRAY_SIZE(packet->headers); ++i) {
        if (packet->headers[i].type == HEADER_NONE) break;
    }
    return i;
}


struct header *packet_append_header(struct packet *packet,
                                    enum header_t header_type,
                                    int header_bytes) {
    struct header *header = NULL;
    int num_headers;
    int bytes_headers = 0;

    for (num_headers = 0; num_headers < ARRAY_SIZE(packet->headers);
         ++num_headers) {
        if (packet->headers[num_headers].type == HEADER_NONE) break;
        bytes_headers += packet->headers[num_headers].header_bytes;
    }

    assert(num_headers <= PACKET_MAX_HEADERS);
    if (num_headers == PACKET_MAX_HEADERS) return NULL;

    header = &packet->headers[num_headers];
    header->h.ptr = packet->buffer + bytes_headers;
    header->type = header_type;
    header->header_bytes = header_bytes;
    header->total_bytes = 0;
    return header;
}