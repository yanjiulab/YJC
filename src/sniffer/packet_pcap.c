#include "packet_pcap.h"
#include <stdbool.h>

// TODO: make strict check
static bool is_valid_pcap(char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("open pcap file failed");
        return false;
    }
    uint32_t magic;
    if (fread(&magic, 4, 1, f) != 1) {
        perror("read pcap file failed");
        return false;
    }
    fclose(f);

    return magic == PCAP_MAGIC ? true : false;
}

FILE* pcap_open(char* filename) {
    pcap_file_header_t pfh = {0};
    pfh.magic = PCAP_MAGIC;
    pfh.version_major = PCAP_VERSION_MAJOR;
    pfh.version_minor = PCAP_VERSION_MINOR;
    pfh.thiszone = 0;
    pfh.sigfigs = 0;
    pfh.snaplen = PCAP_SNAPLEN_MAX;
    pfh.linktype = 1;

    FILE* pf = fopen(filename, "wb");
    if (!pf) {
        perror("open pcap file failed");
        return NULL;
    }

    if (fwrite(&pfh, sizeof(pfh), 1, pf) != 1) {
        perror("write pcap file header failed");
        fclose(pf);
        return NULL;
    }

    return pf;
}

void pcap_close(FILE* pf) {
    if (pf) {
        fclose(pf);
        pf = NULL;
    }
}

int packet_add_pcap(struct packet* p, FILE* pf) {
    pcap_pkthdr_t pph = {0};
    pph.ts.tv_sec = p->tv.tv_sec;
    pph.ts.tv_usec = p->tv.tv_usec;
    pph.caplen = p->buffer_active;
    pph.len = pph.caplen;

    if (fwrite(&pph, sizeof(pph), 1, pf) != 1) {
        perror("write pcap file packet header failed");
        return -1;
    }

    if (fwrite(p->buffer, p->buffer_active, 1, pf) != 1) {
        perror("write pcap file packet data failed");
        return -1;
    }

    return 0;
}

int packet_to_pcap(struct packet* p, char* filename) {

    FILE* pf = NULL;
    if (is_valid_pcap(filename)) {
        pf = fopen(filename, "ab");
    } else {
        pf = pcap_open(filename);
    }
    packet_add_pcap(p, pf);
    pcap_close(pf);
}
