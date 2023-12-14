#ifndef __PACKET_PARSER_H__
#define __PACKET_PARSER_H__

#include "packet.h"

enum packet_parse_result_t {
    PACKET_OK,         /* no errors detected */
    PACKET_BAD,        /* illegal header */
    PACKET_UNKNOWN_L3, /* not IPv4 or IPv6 */
    PACKET_UNKNOWN_L4, /* not TCP or UDP */
};

typedef int (*parse_func_t)(struct packet*, uint8_t*, uint8_t*, char**);
extern parse_func_t parse_handlers[PACKET_LAYER_MAX];
#define parse_handler_register(l, f) parse_handlers[l] = f

/* Given an input packet of length 'in_bytes' stored in the buffer
 * whose location is given by the packet's 'buffer' field and whose
 * full size is given by the 'buffer_bytes' field, parses the packets
 * and fills in packet fields 'ip_bytes', 'ip', and 'tcp'.
 *
 * On success, returns PACKET_OK; on error, returns a enum packet_parse_result_t
 * error code and fills in *error with a human-readable, malloc-allocated error
 * message.
 *
 * Example:
 *   char* error = NULL;
 *   parse_packet(packet, in_bytes, PACKET_LAYER_2_ETHERNET, &error);
 */
int parse_packet(struct packet* packet, int in_bytes, enum packet_layer_t layer, char** error);

#endif /* __PACKET_PARSER_H__ */