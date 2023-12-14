#include "ethernet.h"
#include "ip_address.h"
#include "log.h"
#include "net_utils.h"
#include "packet_parser.h"
#include "packet_pcap.h"
#include "packet_socket.h"
#include "packet_stringify.h"
#include "test.h"

void test_packet_socket() {
    int num_packets = 0;
    char* error = NULL;
    char* dump = NULL;
    struct packet_socket* psock = packet_socket_new("ens33");
    enum direction_t direction = DIRECTION_ALL;
    enum packet_layer_t layer = PACKET_LAYER_2_ETHERNET;
    s32 timeout_secs = -1;
    struct packet* packet;

    // packet_socket_set_filter_str(psock, "not tcp");
    FILE* pf = NULL;
    // printf("pf: %p\n", pf);
    pf = pcap_open("build/a.pcap");
    // printf("pf: %p\n", pf);
    int i = 0;

    while (1) {
        error = NULL;
        int in_bytes = 0;
        enum packet_parse_result_t result;
        packet = packet_new(PACKET_READ_BYTES);
        int rcv_status = packet_socket_receive(psock, direction, timeout_secs,
                                               packet, &in_bytes);
        // printf("pf: %p\n", pf);

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

        if (result == PACKET_OK) {
            packet_stringify(packet, DUMP_VERBOSE, &dump, &error);
            // printf("dump = '%s'\n", dump);
        }
        packet_add_pcap(packet, pf);
        i++;

        if (i == 10) {
            pcap_close(pf);
            return;
        }

        packet_free(packet);
        packet = NULL;

        log_debug("parse_result: %d", result);
        printf("%s", dump);
    }
}