#include "sniffer.h"

// 接口名
// 方向
// 过滤器
// 是否存pcap

sniffer_t* sniffer_new(char* device_name) {
    sniffer_t* sniff = calloc(1, sizeof(sniffer_t));
    sniff->psock = packet_socket_new(device_name);
    sniff->packet = packet_new(PACKET_READ_BYTES);
    sniff->direction = DIRECTION_ALL;
    sniff->record_type = SNIFFER_RECORD_PACKET;
    // sniff->record_max = 0;
    // sniff->packet_list = NULL;
    // sniff->pcap_file = NULL;
    return sniff;
}

void sniffer_free(sniffer_t* sniffer) {
    packet_free(sniffer->packet);
    packet_list_free(sniffer->packet_list);
    pcap_close(sniffer->pcap_file);
}

int sniffer_set_record(sniffer_t* sniffer, sniffer_record_t record,
                       uint32_t max, const char* pcapf) {
    if (!sniffer)
        return SNIFFER_ERROR;
    if (!record)
        return SNIFFER_OK;

    sniffer->record_max = max ? max : SNIFFER_RECORD_DEFAULT;

    if (record == SNIFFER_RECORD_LIST) {
        sniffer->packet_list = packet_list_new();
    } else if (record == SNIFFER_RECORD_PCAP) {
        mkdir_p(SNIFFER_PCAP_PATH);
        datetime_t dt = datetime_now();
        string_t pfn = str_fmt("%s/cap-%04d-%02d-%02d-%02d-%02d-%02d.pcap", SNIFFER_PCAP_PATH,
                               dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
        sniffer->pcap_file = pcapf ? pcap_open(pcapf) : pcap_open(pfn);
        return SNIFFER_OK;
    } else {
        return SNIFFER_ERROR;
    }
}

int sniffer_recv(sniffer_t* sniffer) {
    // sniffer->direction
    int rcv_status;
    rcv_status = packet_socket_receive(sniffer->psock, sniffer->direction, TIMEOUT_NONE,
                                       sniffer->packet, &(sniffer->packet_len));

    if (rcv_status == STATUS_TIMEOUT) {
        /* Set an error message indicating what occurred. */
        fprintf(stderr, "Timed out waiting for packet");
        return STATUS_TIMEOUT;
    }

    if (rcv_status) {
        return rcv_status;
    }

    if (sniffer->record_type == SNIFFER_RECORD_PCAP) {
        if (sniffer->total_pkt >= sniffer->record_max) {
            pcap_close(sniffer->pcap_file);
        }
        packet_add_pcap(sniffer->packet, sniffer->pcap_file);
    }

    if (sniffer->record_type == SNIFFER_RECORD_LIST) {
        // TODO
    }

    sniffer->total_pkt++;
    sniffer->total_byte += sniffer->packet_len;

    return rcv_status;
}