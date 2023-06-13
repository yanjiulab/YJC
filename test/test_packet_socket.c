#include "packet_socket.h"
#include "test.h"
#include "inet.h"

void test_packet_socket() {
    packet_socket_t* psock = packet_socket_new("ens33");
    struct packet *p = packet_new(64 * 1024);
    int len;
    packet_socket_receive(psock, DIRECTION_OUTBOUND, -1, &p, &len);

    char *buf;
    int status;
    status = packet_to_string(p, DUMP_FULL, &buf, NULL);
    printf("%d", status);
    printf("dump = '%s'", buf);
}