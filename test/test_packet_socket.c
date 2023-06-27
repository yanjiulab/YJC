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
    struct packet_socket *psock = packet_socket_new("ens38");
    enum direction_t direction = DIRECTION_INVALID;
    enum packet_layer_t layer = PACKET_LAYER_2_ETHERNET;
    s32 timeout_secs = -1;
    struct packet *packet;

    while (1) {
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
            packet_stringify(packet, DUMP_FULL, &dump, &error);
            printf("dump = '%s'\n", dump);
        // }

        packet_free(packet);
        packet = NULL;

        if (result == PACKET_BAD) continue;

        log_debug("parse_result:%d; error parsing packet: %s", result, error);
    }
}