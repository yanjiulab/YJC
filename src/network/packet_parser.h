#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__

#include "packet.h"

/* What layer of headers is at the head of the packet? */
enum packet_layer_t {
    PACKET_LAYER_3_IP = 0,   /* no layer 2 headers */
    PACKET_LAYER_2_ETHERNET, /* layer 2 is Ethernet */
};

enum packet_parse_result_t {
    PACKET_OK,         /* no errors detected */
    PACKET_BAD,        /* illegal header */
    PACKET_UNKNOWN_L4, /* not TCP or UDP */
};

/* Given an input packet of length 'in_bytes' stored in the buffer
 * whose location is given by the packet's 'buffer' field and whose
 * full size is given by the 'buffer_bytes' field, parses the packets
 * and fills in packet fields 'ip_bytes', 'ip', and 'tcp'.
 *
 * On success, returns PACKET_OK; on error, returns a enum packet_parse_result_t error
 * code and fills in *error with a human-readable, malloc-allocated
 * error message.
 */
int parse_packet(struct packet *packet, int in_bytes, enum packet_layer_t layer, char **error);

#endif /* __PACKET_PARSER_H__ */