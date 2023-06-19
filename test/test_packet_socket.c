#include "netproto/ethernet.h"
#include "inet.h"
#include "ip_address.h"
#include "packet_socket.h"
#include "test.h"

void test_packet_socket() {
    // Ethernet test
    ether_addr_t ea;
    ether_addr_from_string("00:01-02-03-04-05", &ea);
    unsigned a;
    print_data(&ea, sizeof(ea));

    char sbuf[ETH_ASLEN];
    ether_addr_to_string(&ea, sbuf);
    printf("%s", sbuf);

    exit(0);
    packet_socket_t *psock = packet_socket_new("ens33");
    struct packet *p = packet_new(64 * 1024);
    int len;
    packet_socket_receive(psock, DIRECTION_OUTBOUND, -1, &p, &len);

    char *buf;
    int status;
    // status = packet_to_string(p, DUMP_FULL, &buf, NULL);
    // packet_buffer_to_string()
    printf("%d", status);
    printf("dump = '%s'", buf);
}