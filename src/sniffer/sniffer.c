#include "sniffer.h"

sniffer_t* sniffer_new(char* device_name) {
    sniffer_t* sniff = calloc(1, sizeof(sniffer_t));
    sniff->psock = packet_socket_new(device_name);
    sniff->packet = packet_new(PACKET_READ_BYTES);
    sniff->direction = DIRECTION_ALL;
    sniff->record = SNIFFER_RECORD_PACKET;
    // sniff->status = SNIFFER_STOP;
    // sniff->record_num = 0;
    // sniff->packet_list = NULL;
    // sniff->pcap_file = NULL;
    return sniff;
}

void sniffer_free(sniffer_t* sniffer) {
    packet_free(sniffer->packet);
    packet_list_free(sniffer->packet_list);
    pcap_close(sniffer->pcap_file);
    free(sniffer);
}

int sniffer_pause(sniffer_t* sniffer) {
    if (sniffer)
        sniffer->status = SNIFFER_PAUSE;
}

int sniffer_start(sniffer_t* sniffer) {
    if (sniffer)
        sniffer->status = SNIFFER_RUNNING;
}

int sniffer_stop(sniffer_t* sniffer) {
    if (sniffer)
        sniffer->status = SNIFFER_STOP;
}

/**************************** SET API ****************************************/

int sniffer_set_record(sniffer_t* sniffer, sniffer_record_t record,
                       uint32_t record_num, const char* pcapf) {
    if (!sniffer)
        return SNIFFER_ERROR;
    if (!record)
        return SNIFFER_OK;

    sniffer->record = record;
    sniffer->record_num = record_num ? record_num : SNIFFER_RECORD_DEFAULT;

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

// TODO: filter
// 1. ether proto
// 2. ip src dst proto
// 3. udp srcport dstport
// 4. tcp srcport dstport
int sniffer_set_filter(sniffer_t* sniffer, struct sock_filter* filter, int len) {
    return packet_socket_set_filter(sniffer->psock, filter, len);
}

int sniffer_set_filter_str(sniffer_t* sniffer, const char* fs) {
    return packet_socket_set_filter_str(sniffer->psock, fs);
}

int sniffer_set_filter_eth_proto(sniffer_t* sniffer, uint16_t proto) {
    struct sock_fprog bpfcode;
    struct sock_filter bpf_eth[] = {
        {0x28, 0, 0, 0x0000000c},
        {0x15, 0, 1, 0x00000800}, /* ETHERTYPE_IP */
        {0x6, 0, 0, 0x00040000},
        {0x6, 0, 0, 0x00000000}};

    bpf_eth[1].k = proto;
    log_debug("sniffer_set_filter_eth: eth proto: 0x%.4x\n", proto);

    /* Attach the filter. */
    bpfcode.len = array_size(bpf_eth);
    bpfcode.filter = bpf_eth;
    so_setfilter(sniffer->psock->packet_fd, bpfcode);
}

int sniffer_set_filter_eth_byte(sniffer_t* sniffer, uint32_t pos, uint8_t val) {
    struct sock_fprog bpfcode;
    struct sock_filter bpf_eth[] = {
        {0x30, 0, 0, 0x0000001e}, /* Position offset to ethernet start */
        {0x15, 0, 1, 0x000000d1}, /* Value of position */
        {0x6, 0, 0, 0x00040000},
        {0x6, 0, 0, 0x00000000}};

    bpf_eth[0].k = pos;
    bpf_eth[1].k = val;

    /* Attach the filter. */
    bpfcode.len = array_size(bpf_eth);
    bpfcode.filter = bpf_eth;
    so_setfilter(sniffer->psock->packet_fd, bpfcode);
}

int sniffer_recv(sniffer_t* sniffer) {
    if (sniffer->status == SNIFFER_STOP)
        return SNIFFER_ERROR;
    if (sniffer->status == SNIFFER_PAUSE)
        return SNIFFER_OK;

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

    if (sniffer->record == SNIFFER_RECORD_PCAP) {
        if (sniffer->total_pkt < sniffer->record_num) {
            packet_add_pcap(sniffer->packet, sniffer->pcap_file);
        } else {
            // TODO: double free bug
            pcap_close(sniffer->pcap_file);
            sniffer->pcap_file = NULL;
            log_fatal("sniffer record has been finished.");
        }
    }

    if (sniffer->record == SNIFFER_RECORD_LIST) {
        // TODO
    }

    sniffer->total_pkt++;
    sniffer->total_byte += sniffer->packet_len;

    return rcv_status;
}
