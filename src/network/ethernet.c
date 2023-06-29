#include "ethernet.h"

#include <stdlib.h>

static uint8_t hex_char_to_int(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    return 0;
}

void ether_from_string(const char *str, ether_addr_t *ether) {
    char *p;
    char i, j;
    unsigned char h = 0, l = 0;
    for (i = 0, j = 0, p = str; *p; p++, i++) {
        if (*p == ':' || *p == '-') p++;
        if (i % 2 == 0) {
            h = hex_char_to_int(*p);
        } else {
            l = hex_char_to_int(*p);
            ether->ether_addr_octet[j] = h << 4 | l;
            j++;
        }
    }
}

char *ether_to_string(ether_addr_t *ether, const char *str) {
    if (!str) str = calloc(1, ETH_ASLEN);

    snprintf(str, ETH_ASLEN, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
             ether->ether_addr_octet[0], ether->ether_addr_octet[1],
             ether->ether_addr_octet[2], ether->ether_addr_octet[3],
             ether->ether_addr_octet[4], ether->ether_addr_octet[5]);

    return str;
}

const char *ether_type2str(uint16_t type) {
    switch (type) {
        case ETHERTYPE_IP:
            return "IPv4";
        case ETHERTYPE_IPV6:
            return "IPv6";
        case ETHERTYPE_ARP:
            return "ARP";
        case ETHERTYPE_VLAN:
            return "802.1Q VLAN";
        default:
            return "UNKNOWN";
    }
}