/* ethernet.h */

#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <net/ethernet.h>
#include <netinet/ether.h> // Ethernet
#include <stdio.h>
#include <stdbool.h>

/* IEEE 802.3 Ethernet magic constants. The frame sizes omit the preamble
 * and FCS/CRC (frame check sequence). */
#define ETH_ALEN 6         /* Octets in one ethernet addr	 */
#define ETH_ASLEN 18       /* String in one ethernet addr	 */
#define ETH_TLEN 2         /* Octets in ethernet type field */
#define ETH_HLEN 14        /* Total octets in header.	 */
#define ETH_ZLEN 60        /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN 1500  /* Max. octets in payload	 */
#define ETH_FRAME_LEN 1514 /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN 4      /* Octets in the FCS		 */

/* To tell a packet socket that you want traffic for all protocols. */
#define ETH_P_ALL 0x0003

/* Ethernet header. */
typedef struct ethhdr ethhdr_t;
const char* ether_type2str(uint16_t type);

/* Ethernet address. */
struct ethaddr {
    uint8_t octet[ETH_ALEN];
} __attribute__((packed));
typedef struct ethaddr ethaddr_t;

#define ether_copy(dst, src) memcpy(dst, src, sizeof(ethaddr_t))
int ether_str2mac(const char* str, struct ethaddr* mac);
char* ether_mac2str(const struct ethaddr* mac, char* buf, int size);
bool is_zero_mac(const struct ethaddr* mac);
bool is_bcast_mac(const struct ethaddr* mac);
bool is_mcast_mac(const struct ethaddr* mac);

#endif /* __ETHERNET_H__ */