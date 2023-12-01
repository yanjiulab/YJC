

#include <errno.h>
#include <linux/mroute.h>
// #include <netinet/in.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

// #define MRT_BASE 200
// #define MRT_INIT (MRT_BASE)               /* Activate the kernel mroute code 	*/
// #define MRT_DONE (MRT_BASE + 1)           /* Shutdown the kernel mroute		*/
// #define MRT_ADD_VIF (MRT_BASE + 2)        /* Add a virtual interface		*/
// #define MRT_DEL_VIF (MRT_BASE + 3)        /* Delete a virtual interface		*/
// #define MRT_ADD_MFC (MRT_BASE + 4)        /* Add a multicast forwarding entry	*/
// #define MRT_DEL_MFC (MRT_BASE + 5)        /* Delete a multicast forwarding entry	*/
// #define MRT_VERSION (MRT_BASE + 6)        /* Get the kernel multicast version	*/
// #define MRT_ASSERT (MRT_BASE + 7)         /* Activate PIM assert mode		*/
// #define MRT_PIM (MRT_BASE + 8)            /* enable PIM code			*/
// #define MRT_TABLE (MRT_BASE + 9)          /* Specify mroute table ID		*/
// #define MRT_ADD_MFC_PROXY (MRT_BASE + 10) /* Add a (*,*|G) mfc entry	*/
// #define MRT_DEL_MFC_PROXY (MRT_BASE + 11) /* Del a (*,*|G) mfc entry	*/
// #define MRT_FLUSH (MRT_BASE + 12)         /* Flush all mfc entries and/or vifs	*/
// #define MRT_MAX (MRT_BASE + 12)

// #define MAXVIFS 32
// typedef unsigned long vifbitmap_t; /* User mode code depends on this lot */
// typedef unsigned short vifi_t;
// #define ALL_VIFS ((vifi_t)(-1))

// /* Cache manipulation structures for mrouted and PIMd */
// struct mfcctl {
//     struct in_addr mfcc_origin;       /* Origin of mcast	*/
//     struct in_addr mfcc_mcastgrp;     /* Group in question	*/
//     vifi_t mfcc_parent;               /* Where it arrived	*/
//     unsigned char mfcc_ttls[MAXVIFS]; /* Where it is going	*/
//     unsigned int mfcc_pkt_cnt;        /* pkt count for src-grp */
//     unsigned int mfcc_byte_cnt;
//     unsigned int mfcc_wrong_if;
//     int mfcc_expire;
// };

// /*
//  * Argument structure for MRT_ADD_VIF.
//  * (MRT_DEL_VIF takes a single vifi_t argument.)
//  */
// struct vifctl {
// 	vifi_t	vifc_vifi;	    	/* the index of the vif to be added */
// 	u_char	vifc_flags;     	/* VIFF_ flags defined below */
// 	u_char	vifc_threshold; 	/* min ttl required to forward on vif */
// 	u_int	vifc_rate_limit;	/* max rate */
// 	struct	in_addr vifc_lcl_addr;	/* local interface address */
// 	struct	in_addr vifc_rmt_addr;	/* remote address (tunnels only) */
// };

int main() {

    struct {
        int a;
        char b;
        char c;
    } p;
    int size = sizeof(p);
    printf("sizeof req: %d\n", size);
    int len;
    len = NLMSG_SPACE(p);
    printf("NLMSG_LENGTH: %d\n", len);

    // struct mfcctl mc;
    // vifi_t vifi;
    // struct uvif* v;

    // int s;
    // uint32_t source = 0x01F4A8C0;
    // uint32_t group = 0x020202E8;
    // vifi_t iif = 1;
    // vifbitmap_t oifs;

    // int numvifs = 3;

    // mc.mfcc_origin.s_addr = source;
    // mc.mfcc_mcastgrp.s_addr = group;
    // mc.mfcc_parent = 0;
    // mc.mfcc_ttls[1] = (uint8_t)0x01;

    // // for (vifi = 0; vifi < numvifs; vifi++) {
    // //     mc.mfcc_ttls[vifi] = (uint8_t)0x01;
    // // }

    // s = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP);

    // if (setsockopt(s, IPPROTO_IP, MRT_ADD_MFC, (char*)&mc,
    //                sizeof(mc)) < 0) {
    //     printf("setsockopt MRT_ADD_MFC for source and group error (%s)\n", strerror(errno));
    //     return 1;
    // } else {
    //     printf("Install/modify a MFC entry in the kernel\n");
    // }

    // setsockopt(s, IPPROTO_IP, MRT_DEL_MFC, (char *)&mc, sizeof(mc));

    // return 0;
}

