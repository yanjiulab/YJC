/* ip.h */

#ifndef __IP_H__
#define __IP_H__

#include <netinet/ip.h>  // IPv4

struct ipv4 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    __uint8_t ihl : 4, version : 4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    __uint8_t version : 4, ihl : 4;
#else
#error "Please fix endianness defines"
#endif
    __uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    __uint8_t ttl;
    __uint8_t protocol;
    uint16_t check;
    struct in_addr src_ip;
    struct in_addr dst_ip;
};

typedef struct ipv4 ipv4_t;

#define IPPROTO_OSPF 89

/* ----------------------- IP socket option values -------------------- */

/* Oddly enough, Linux distributions are typically missing even some
 * of the older and more common IP socket options, such as IP_MTU.
 */
#define IP_TOS 1
#define IP_TTL 2
#define IP_HDRINCL 3
#define IP_OPTIONS 4
#define IP_ROUTER_ALERT 5
#define IP_RECVOPTS 6
#define IP_RETOPTS 7
#define IP_PKTINFO 8
#define IP_PKTOPTIONS 9
#define IP_MTU_DISCOVER 10
#define IP_RECVERR 11
#define IP_RECVTTL 12
#define IP_RECVTOS 13
#define IP_MTU 14
#define IP_FREEBIND 15
#define IP_IPSEC_POLICY 16
#define IP_XFRM_POLICY 17
#define IP_PASSSEC 18
#define IP_TRANSPARENT 19

/* ECN: RFC 3168: http://tools.ietf.org/html/rfc3168 */
#define IP_ECN_MASK 3
#define IP_ECN_NONE 0
#define IP_ECN_ECT1 1
#define IP_ECN_ECT0 2
#define IP_ECN_CE 3

static inline uint8_t ipv4_tos_byte(const struct ipv4 *ipv4) {
    return ipv4->tos;
}

static inline uint8_t ipv4_ttl_byte(const struct ipv4 *ipv4) {
    return ipv4->ttl;
}

static inline int ipv4_header_len(const struct ipv4 *ipv4) {
    return ipv4->ihl * sizeof(uint32_t);
}

/* IP fragmentation bit flags */
#define IP_RF 0x8000      /* reserved fragment flag */
#define IP_DF 0x4000      /* don't fragment flag */
#define IP_MF 0x2000      /* more fragments flag */
#define IP_OFFMASK 0x1FFF /* mask for fragmenting bits */

static inline char *ip_proto2str(uint8_t proto) {
    switch (proto) {
        case IPPROTO_ICMP:
            return "ICMP";
        case IPPROTO_IGMP:
            return "IGMP";
        case IPPROTO_IPIP:
            return "IPIP";
        case IPPROTO_TCP:
            return "TCP";
        case IPPROTO_UDP:
            return "UDP";
        case IPPROTO_IPV6:
            return "IPv6";
        case IPPROTO_RSVP:
            return "RSVP";
        case IPPROTO_GRE:
            return "GRE";
        case IPPROTO_ESP:
            return "ESP";
        case IPPROTO_OSPF:
            return "OSPF";
        case IPPROTO_MTP:
            return "MTP";
        case IPPROTO_PIM:
            return "PIM";
        case IPPROTO_SCTP:
            return "SCTP";
        case IPPROTO_MPLS:
            return "MPLS";
        case IPPROTO_RAW:
            return "RAW";
        default:
            return "UNKNOWN";
    }
}

/* IPv6 */
struct ipv6 {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    __uint8_t traffic_class_hi : 4, version : 4;
    __uint8_t flow_label_hi : 4, traffic_class_lo : 4;
    uint16_t flow_label_lo;
#elif __BYTE_ORDER == __BIG_ENDIAN
    __uint8_t version : 4, traffic_class_hi : 4;
    __uint8_t traffic_class_lo : 4, flow_label_hi : 4;
    uint16_t flow_label_lo;
#else
#error "Please fix endianness defines"
#endif

    uint16_t payload_len;
    __uint8_t next_header;
    __uint8_t hop_limit;

    struct in6_addr src_ip;
    struct in6_addr dst_ip;
};

typedef struct ipv6 ipv6_t;

#define IPV6_HOPLIMIT 52
#define IPV6_TCLASS 67

static inline uint8_t ipv6_tos_byte(const struct ipv6 *ipv6) {
    return (ipv6->traffic_class_hi << 4) | ipv6->traffic_class_lo;
}

static inline uint32_t ipv6_flow_label(const struct ipv6 *ipv6) {
    return (ntohs(ipv6->flow_label_lo)) | (ipv6->flow_label_hi << 16);
}

static inline uint8_t ipv6_hoplimit_byte(const struct ipv6 *ipv6) {
    return ipv6->hop_limit;
}

/* The following struct declaration is needed for the IPv6 ioctls
 * SIOCSIFADDR and SIOCDIFADDR that add and delete IPv6 addresses from
 * a network interface. We have to declare our own version here
 * because this struct is only available in /usr/include/linux/ipv6.h,
 * but that .h file has kernel IPv6 declarations that conflict with
 * standard user-space IPv6 declarations.
 */
struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __uint32_t ifr6_prefixlen;
    int ifr6_ifindex;
};

#endif /* __IP_H__ */