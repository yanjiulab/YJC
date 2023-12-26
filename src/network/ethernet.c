#include "ethernet.h"

#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

const char* ether_type2str(uint16_t type) {
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

bool is_zero_mac(const struct ethaddr* mac) {
    int i = 0;

    for (i = 0; i < ETH_ALEN; i++) {
        if (mac->octet[i])
            return false;
    }

    return true;
}

bool is_bcast_mac(const struct ethaddr* mac) {
    int i = 0;

    for (i = 0; i < ETH_ALEN; i++)
        if (mac->octet[i] != 0xFF)
            return false;

    return true;
}

bool is_mcast_mac(const struct ethaddr* mac) {
    if ((mac->octet[0] & 0x01) == 0x01)
        return true;

    return false;
}

/* converts to internal representation of mac address
 * returns 1 on success, 0 otherwise
 * format accepted: AA:BB:CC:DD:EE:FF
 * if mac parameter is null, then check only
 */
int ether_str2mac(const char* str, struct ethaddr* mac) {
    unsigned int a[6];
    int i;

    if (!str)
        return 0;

    if (sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", a + 0, a + 1, a + 2, a + 3,
               a + 4, a + 5) != 6) {
        /* error in incoming str length */
        return 0;
    }
    /* valid mac address */
    if (!mac)
        return 1;
    for (i = 0; i < 6; ++i)
        mac->octet[i] = a[i] & 0xff;
    return 1;
}

char* ether_mac2str(const struct ethaddr* mac, char* buf, int size) {
    char* ptr;

    if (!mac)
        return NULL;
    if (!buf) {
        ptr = calloc(1, ETH_ASLEN);
    } else {
        assert(size >= ETH_ASLEN);
        ptr = buf;
    }
    snprintf(ptr, ETH_ASLEN, "%02x:%02x:%02x:%02x:%02x:%02x",
             (uint8_t)mac->octet[0], (uint8_t)mac->octet[1],
             (uint8_t)mac->octet[2], (uint8_t)mac->octet[3],
             (uint8_t)mac->octet[4], (uint8_t)mac->octet[5]);
    return ptr;
}
