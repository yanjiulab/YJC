#include "ethernet.h"
#include "ip_address.h"
#include "log.h"
#include "net_utils.h"
#include "packet_parser.h"
#include "packet_socket.h"
#include "packet_stringify.h"
#include "test.h"

void test_packet_socket() {
    int num_packets = 0;
    char *error = NULL;
    char *dump = NULL;
    struct packet_socket *psock = packet_socket_new("ens33");
    enum direction_t direction = DIRECTION_INVALID;
    enum packet_layer_t layer = PACKET_LAYER_2_ETHERNET;
    s32 timeout_secs = -1;
    struct packet *packet;

    struct sock_filter filter[] = {
        {0x28, 0, 0, 0x0000000c}, {0x15, 0, 2, 0x00000800},
        {0x30, 0, 0, 0x00000017}, {0x15, 6, 7, 0x00000059},
        {0x15, 0, 6, 0x000086dd}, {0x30, 0, 0, 0x00000014},
        {0x15, 3, 0, 0x00000059}, {0x15, 0, 3, 0x0000002c},
        {0x30, 0, 0, 0x00000036}, {0x15, 0, 1, 0x00000059},
        {0x6, 0, 0, 0x00040000},  {0x6, 0, 0, 0x00000000},
    };
    packet_socket_set_filter(psock, filter, ARRAY_SIZE(filter));

    while (1) {
        error = NULL;
        int in_bytes = 0;
        enum packet_parse_result_t result;
        packet = packet_new(PACKET_READ_BYTES);
        int rcv_status = packet_socket_receive(psock, direction, timeout_secs,
                                               packet, &in_bytes);
        if (rcv_status == STATUS_TIMEOUT) {
            /* Set an error message indicating what occurred. */
            asprintf(&error, "Timed out waiting for packet");
            return STATUS_TIMEOUT;
        }
        if (rcv_status) {
            packet_free(packet);
            packet = NULL;
            continue;
        }
        ++num_packets;

        result = parse_packet(packet, in_bytes, layer, &error);
        // if (result == PACKET_OK) {
        // packet_stringify(packet, DUMP_FULL, &dump, &error);
        // printf("dump = '%s'\n", dump);
        // }

        packet_free(packet);
        packet = NULL;

        log_debug("parse_result: %d", result);
        log_debug("error: %s", error);
    }
}