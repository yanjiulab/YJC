// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Nexthop structure definition.
 * Copyright (C) 1997, 98, 99, 2001 Kunihiro Ishiguro
 * Copyright (C) 2013 Cumulus Networks, Inc.
 */

#ifndef _LIB_NEXTHOP_H
#define _LIB_NEXTHOP_H

#include "mpls.h"
#include "prefix.h"
#include "srv6.h"
#include "vxlan.h"
#include "vrf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum next hop string length - gateway + ifindex */
#define NEXTHOP_STRLEN (INET6_ADDRSTRLEN + 30)

union g_addr {
    struct in_addr ipv4;
    struct in6_addr ipv6;
};

enum nexthop_types_t {
    NEXTHOP_TYPE_IFINDEX = 1,  /* Directly connected.  */
    NEXTHOP_TYPE_IPV4,         /* IPv4 nexthop.  */
    NEXTHOP_TYPE_IPV4_IFINDEX, /* IPv4 nexthop with ifindex.  */
    NEXTHOP_TYPE_IPV6,         /* IPv6 nexthop.  */
    NEXTHOP_TYPE_IPV6_IFINDEX, /* IPv6 nexthop with ifindex.  */
    NEXTHOP_TYPE_BLACKHOLE,    /* Null0 nexthop.  */
};

enum blackhole_type {
    BLACKHOLE_UNSPEC = 0,
    BLACKHOLE_NULL,
    BLACKHOLE_REJECT,
    BLACKHOLE_ADMINPROHIB,
};

enum nh_encap_type {
    NET_VXLAN = 100, /* value copied from FPM_NH_ENCAP_VXLAN. */
};

/* Fixed limit on the number of backup nexthops per primary nexthop */
#define NEXTHOP_MAX_BACKUPS 8

/* Backup index value is limited */
#define NEXTHOP_BACKUP_IDX_MAX 255

/* Nexthop structure. */
struct nexthop {
    struct nexthop* next;
    struct nexthop* prev;

    /*
     * What vrf is this nexthop associated with?
     */
    vrf_id_t vrf_id;

    /* Interface index. */
    ifindex_t ifindex;

    enum nexthop_types_t type;

    uint16_t flags;
#define NEXTHOP_FLAG_ACTIVE (1 << 0)       /* This nexthop is alive. */
#define NEXTHOP_FLAG_FIB (1 << 1)          /* FIB nexthop. */
#define NEXTHOP_FLAG_RECURSIVE (1 << 2)    /* Recursive nexthop. */
#define NEXTHOP_FLAG_ONLINK (1 << 3)       /* Nexthop should be installed \
                                            * onlink.                     \
                                            */
#define NEXTHOP_FLAG_DUPLICATE (1 << 4)    /* nexthop duplicates another \
                                            * active one                 \
                                            */
#define NEXTHOP_FLAG_RNH_FILTERED (1 << 5) /* rmap filtered, used by rnh */
#define NEXTHOP_FLAG_HAS_BACKUP (1 << 6)   /* Backup nexthop index is set */
#define NEXTHOP_FLAG_SRTE (1 << 7)         /* SR-TE color used for BGP traffic */
#define NEXTHOP_FLAG_EVPN (1 << 8)         /* nexthop is EVPN */
#define NEXTHOP_FLAG_LINKDOWN (1 << 9)     /* is not removed on link down */

#define NEXTHOP_IS_ACTIVE(flags) \
    (CHECK_FLAG(flags, NEXTHOP_FLAG_ACTIVE) && !CHECK_FLAG(flags, NEXTHOP_FLAG_DUPLICATE))

    /* Nexthop address */
    union {
        union g_addr gate;
        enum blackhole_type bh_type;
    };
    union g_addr src;
    union g_addr rmap_src; /* Src is set via routemap */

    /* Nexthops obtained by recursive resolution.
     *
     * If the nexthop struct needs to be resolved recursively,
     * NEXTHOP_FLAG_RECURSIVE will be set in flags and the nexthops
     * obtained by recursive resolution will be added to `resolved'.
     */
    struct nexthop* resolved;
    /* Recursive parent */
    struct nexthop* rparent;

    /* Type of label(s), if any */
    enum lsp_types_t nh_label_type;

    /* Label(s) associated with this nexthop. */
    struct mpls_label_stack* nh_label;

    /* Weight of the nexthop ( for unequal cost ECMP ) */
    uint8_t weight;

    /* Count and index of corresponding backup nexthop(s) in a backup list;
     * only meaningful if the HAS_BACKUP flag is set.
     */
    uint8_t backup_num;
    uint8_t backup_idx[NEXTHOP_MAX_BACKUPS];

    /* Encapsulation information. */
    enum nh_encap_type nh_encap_type;
    union {
        vni_t vni;
    } nh_encap;

    /* EVPN router's MAC.
     * Don't support multiple RMAC from the same VTEP yet, so it's not
     * included in hash key.
     */
    struct ethaddr rmac;

    /* SR-TE color used for matching SR-TE policies */
    uint32_t srte_color;

    /* SRv6 information */
    struct nexthop_srv6* nh_srv6;
};

/* Utility to append one nexthop to another. */
#define NEXTHOP_APPEND(to, new) \
    do {                        \
        (to)->next = (new);     \
        (new)->prev = (to);     \
        (new)->next = NULL;     \
    } while (0)

struct nexthop* nexthop_new(void);

void nexthop_free(struct nexthop* nexthop);
void nexthops_free(struct nexthop* nexthop);

void nexthop_add_labels(struct nexthop* nexthop, enum lsp_types_t ltype,
                        uint8_t num_labels, const mpls_label_t* labels);
void nexthop_del_labels(struct nexthop*);
void nexthop_add_srv6_seg6local(struct nexthop* nexthop, uint32_t action,
                                const struct seg6local_context* ctx);
void nexthop_del_srv6_seg6local(struct nexthop* nexthop);
void nexthop_add_srv6_seg6(struct nexthop* nexthop,
                           const struct in6_addr* segs);
void nexthop_del_srv6_seg6(struct nexthop* nexthop);

/*
 * Allocate a new nexthop object and initialize it from various args.
 */
struct nexthop* nexthop_from_ifindex(ifindex_t ifindex, vrf_id_t vrf_id);
struct nexthop* nexthop_from_ipv4(const struct in_addr* ipv4,
                                  const struct in_addr* src,
                                  vrf_id_t vrf_id);
struct nexthop* nexthop_from_ipv4_ifindex(const struct in_addr* ipv4,
                                          const struct in_addr* src,
                                          ifindex_t ifindex, vrf_id_t vrf_id);
struct nexthop* nexthop_from_ipv6(const struct in6_addr* ipv6,
                                  vrf_id_t vrf_id);
struct nexthop* nexthop_from_ipv6_ifindex(const struct in6_addr* ipv6,
                                          ifindex_t ifindex, vrf_id_t vrf_id);
struct nexthop* nexthop_from_blackhole(enum blackhole_type bh_type,
                                       vrf_id_t nh_vrf_id);

#ifdef __cplusplus
}
#endif

#endif /*_LIB_NEXTHOP_H */