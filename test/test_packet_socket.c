#include "net_utils.h"
#include "ip_address.h"
#include "log.h"
#include "ethernet.h"
#include "packet_socket.h"
#include "packet_parser.h"
#include "test.h"

void test_packet_socket() {
    // Ethernet test
    // ether_addr_t ea;
    // ether_from_string("00:01-02-03-04-05", &ea);
    // unsigned a;
    // print_data(&ea, sizeof(ea));

    // char sbuf[ETH_ASLEN];
    // ether_to_string(&ea, sbuf);
    // printf("%s", sbuf);

    // exit(0);
    packet_socket_t *psock = packet_socket_new("ens38");
    struct packet *p = packet_new(64 * 1024);
    int len;
    packet_socket_receive(psock, DIRECTION_INVALID, -1, p, &len);
    char *error = NULL;
    parse_packet(p, len, PACKET_LAYER_2_ETHERNET, &error);
    log_info("%s", error);
    char *buf;
    int status;
    // status = packet_to_string(p, DUMP_FULL, &buf, NULL);
    // packet_buffer_to_string(s, packet);
    // printf("dump = '%s'\n", buf);
}