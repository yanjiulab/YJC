#include "packet.h"

#include "defs.h"
#include "ethernet.h"
#include "ip.h"
/* ---------------------------- packet ---------------------------- */

/* Map a pointer to a packet offset from an old base to a new base. */
static void* offset_ptr(uint8_t* old_base, uint8_t* new_base, void* old_ptr) {
    uint8_t* old = (uint8_t*)old_ptr;

    return (old == NULL) ? NULL : (new_base + (old - old_base));
}

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

static void packet_duplicate_info(struct packet* packet,
                                  struct packet* old_packet,
                                  int bytes_headroom,
                                  int extra_payload) {
    uint8_t* old_base = old_packet->buffer;
    uint8_t* new_base = packet->buffer + bytes_headroom;

    packet->direction = old_packet->direction;
    packet->dev_ifindex = old_packet->dev_ifindex;
    packet->tv = old_packet->tv;

    /* Set up header pointer. */
    packet->eth = offset_ptr(old_base, new_base, old_packet->eth);
    packet->ipv4 = offset_ptr(old_base, new_base, old_packet->ipv4);
    packet->ipv6 = offset_ptr(old_base, new_base, old_packet->ipv6);
    packet->tcp = offset_ptr(old_base, new_base, old_packet->tcp);
    packet->udp = offset_ptr(old_base, new_base, old_packet->udp);
}

/* Make a copy of the given old packet, but in the new copy reserve the
 * given number of bytes of headroom at the start of the packet->buffer.
 * This empty headroom can later be filled with outer packet headers.
 * A slow but simple model.
 */
static struct packet* packet_copy_with_headroom(struct packet* old_packet,
                                                int bytes_headroom) {
    /* Allocate a new packet and copy link layer header and IP datagram. */
    const int bytes_used = packet_end(old_packet) - old_packet->buffer;
    assert(bytes_used >= 0);
    assert(bytes_used <= 128 * 1024);
    struct packet* packet = packet_new(bytes_headroom + bytes_used);
    uint8_t* old_base = old_packet->buffer;
    uint8_t* new_base = packet->buffer + bytes_headroom;

    memcpy(new_base, old_base, bytes_used);

    packet_duplicate_info(packet, old_packet, bytes_headroom, 0);

    return packet;
}

struct packet* packet_copy(struct packet* old_packet) {
    return packet_copy_with_headroom(old_packet, 0);
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