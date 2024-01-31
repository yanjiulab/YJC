#include "sniffer.h"
#include "base.h"
#include "eventloop.h"
#include "log.h"
#include "packet.h"
#include "packet_parser.h"
#include "packet_pcap.h"
#include "packet_socket.h"
#include "packet_stringify.h"
#include "str.h"

struct packet_socket* psock = NULL;
int num_packets = 0;
FILE* pf = NULL;

sniffer_t* sniff = NULL;

void on_stdin(eio_t* io, void* buf, int readbytes) {
    if (strmatch((char*)buf, "q")) {
        eloop_stop(event_loop(io));
        pcap_close(pf);
        printf("sniffer %d packets.\n", num_packets);
    }
}

void on_packet_socket(eio_t* io) {
    char* error = NULL;
    char* dump = NULL;
    enum packet_parse_result_t result;

    int rcv_status = sniffer_recv(sniff);

    result = parse_packet(sniff->packet, sniff->packet_len, PACKET_LAYER_2_ETHERNET, &error);
    if (result != PACKET_OK) {
        printf("%s", error);
    }

    result = packet_stringify(sniff->packet, DUMP_FULL, &dump, &error);
    if (result != STATUS_OK) {
        printf("%s", error);
    }

    printf("%s", dump);
}

int main(int argc, char* argv[]) {

    eloop_t* loop = eloop_new(0);

    // set stdin
    printf("input 'q' to quit loop\n");
    char buf[64];
    hread(loop, 0, buf, sizeof(buf), on_stdin);

    // t
    sniff = sniffer_new(NULL);
    sniffer_set_record(sniff, SNIFFER_RECORD_PCAP, 100, NULL);
    sniffer_set_direction(sniff, DIRECTION_ALL);
    sniffer_set_filter_str(sniff, "arp");
    sniffer_start(sniff);
    // sniffer_set_filer(sniff, );
    hread(loop, sniff->psock->packet_fd, sniff->packet->buffer, PACKET_READ_BYTES, on_packet_socket);

    // psock = packet_socket_new(NULL);
    // pf = pcap_open("build/a.pcap");
    // char rbuf[PACKET_READ_BYTES];
    // hread(loop, psock->packet_fd, rbuf, sizeof(rbuf), on_packet_socket);

    eloop_run(loop);
    eloop_free(&loop);
    return 0;
}