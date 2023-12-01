#include "packet_dump.h"
#include <stdbool.h>

FILE* pcap_file_new(char* filename) {
    pcap_file_header_t pfh = {0};
    pfh.magic = PCAP_MAGIC;
    pfh.version_major = PCAP_VERSION_MAJOR;
    pfh.version_minor = PCAP_VERSION_MINOR;
    pfh.thiszone = 0;
    pfh.sigfigs = 0;
    pfh.snaplen = 0x40000;
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

void pcap_file_free(FILE* pf) {
    if (pf) {
        fclose(pf);
        pf = NULL;
    }
}

int pcap_file_write(FILE* pf, struct packet* p) {
    pcap_pkthdr_t pph = {0};
    pph.ts.tv_sec = p->tv.tv_sec;
    pph.ts.tv_usec = p->tv.tv_usec;
    pph.caplen = p->buffer_active;
    pph.len = pph.caplen;
    // printf("pf write: %p\n", pf);
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

bool is_valid_pcap(char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("open pcap file failed");
        return;
    }
    uint32_t magic;
    if (fread(&magic, 4, 1, f) != 1) {
        perror("read pcap file failed");
    }

    printf("%x", magic);

    if (magic == PCAP_MAGIC) {
        return true;
    } else {
        return false;
    }
}

int packet_dump_pcap(struct packet* p, char* filename) {

    FILE* pf = NULL;
    if (is_valid_pcap(filename)) {
        // if pcap file exsit, then append it.
        printf("valid pcap file.");
        pf = fopen(filename, "ab");
    } else {
        // new pcap file
        pf = pcap_file_new(filename);
        printf("invalid pcap file.");
    }

    pcap_file_write(pf, p);

    pcap_file_free(pf);
}

int packet_dump_pcap_from_buf(uint8_t* hex, char* filename) {
}