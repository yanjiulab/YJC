/* ethernet.h */

#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <net/ethernet.h>
#include <netinet/ether.h>  // Ethernet
#include <stdio.h>

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
typedef struct ethhdr ethernet_t;

/* Ethernet address. */
typedef struct ether_addr ether_addr_t;

static inline void ether_copy(void *dst, const void *src) {
    memcpy(dst, src, sizeof(ether_addr_t));
}
void ether_from_string(const char *str, ether_addr_t *ether);
char *ether_to_string(ether_addr_t *ether, const char *str);
const char *ether_type2str(uint16_t type);

#endif /* __ETHERNET_H__ */