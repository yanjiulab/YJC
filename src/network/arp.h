/* arp.h */

#ifndef __ARP_H__
#define __ARP_H__

// #include <linux/if_arp.h>

/* ARP IP-Ethernet protocol header. */
struct arphdr_s {
    unsigned short ar_hrd;          /* format of hardware address	*/
    unsigned short ar_pro;          /* format of protocol address	*/
    unsigned char ar_hln;           /* length of hardware address	*/
    unsigned char ar_pln;           /* length of protocol address	*/
    unsigned short ar_op;           /* ARP opcode (command)		*/
    unsigned char ar_sha[ETH_ALEN]; /* sender hardware address	*/
    unsigned char ar_sip[4];        /* sender IP address		*/
    unsigned char ar_tha[ETH_ALEN]; /* target hardware address	*/
    unsigned char ar_tip[4];        /* target IP address		*/
} __attribute__((packed));

typedef struct arphdr_s arp_t;

/* ARP protocol opcodes. */
#define ARPOP_REQUEST 1   /* ARP request.  */
#define ARPOP_REPLY 2     /* ARP reply.  */
#define ARPOP_RREQUEST 3  /* RARP request.  */
#define ARPOP_RREPLY 4    /* RARP reply.  */
#define ARPOP_InREQUEST 8 /* InARP request.  */
#define ARPOP_InREPLY 9   /* InARP reply.  */
#define ARPOP_NAK 10      /* (ATM)ARP NAK.  */

/* ARP protocol HARDWARE identifiers. */
#define ARPHRD_ETHER 1 /* Ethernet */

// void arp_neigh_table_get();
// void arp_neigh_table_put();

#endif /* __ARP_H__ */