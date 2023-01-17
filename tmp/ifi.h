/**
 * @file ifi.h
 * @author your name (you@domain.com)
 * @brief Enhancement of getifaddrs and ioctl
 * @version 0.1
 * @date 2022-06-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef IFI_H
#define IFI_H

#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define IFI_NAME 16 /* same as IFNAMSIZ in <net/if.h> */
#define IFI_HADDR 8 /* allow for 64-bit EUI-64 in future */
#define IFI_ALIAS 1 /* ifi_addr is an alias */

extern int ifi_fd; /* fd used for ioctl */

struct ifi_info {
    char ifi_name[IFI_NAME];       /* interface name, null-terminated */
    short ifi_index;               /* interface index */
    short ifi_mtu;                 /* interface MTU */
    u_char ifi_haddr[IFI_HADDR];   /* hardware address */
    u_short ifi_hlen;              /* # bytes in hardware address: 0, 6, 8 */
    short ifi_flags;               /* IFF_xxx constants from <net/if.h> */
    short ifi_myflags;             /* our own IFI_xxx flags */
    struct sockaddr *ifi_addr;     /* primary address */
    struct sockaddr *ifi_brdaddr;  /* broadcast address */
    struct sockaddr *ifi_maskaddr; /* destination address or netmask address */
    struct ifi_info *ifi_next;     /* next of these structures */
};

/* function prototypes */
struct ifi_info *get_ifi_by_name(const char *);
struct ifi_info *get_ifi_by_index(unsigned int);
struct ifi_info *get_ifi_info(int, int);
void free_ifi_info(struct ifi_info *);
void print_ifi_info(struct ifi_info *);

#endif