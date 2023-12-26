#ifndef __IP_ADDRESS_H__
#define __IP_ADDRESS_H__

#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>

/* Length of output buffer for inet_ntop, plus prefix length (e.g. "/128"). */
#define ADDR_STR_LEN ((INET_ADDRSTRLEN + INET6_ADDRSTRLEN) + 5)

/* IPv4 or IPv6 address. */
struct ipaddr {
    int address_family; /* AF_INET or AF_INET6 */
    union {
        struct in_addr v4;
        struct in6_addr v6;
        uint8_t addr[16];
    } ip; /* IP address (network order) */
#define ipaddr_v4 ip.v4
#define ipaddr_v6 ip.v6
};
typedef struct ipaddr ipaddr_t;

#define IS_IPADDR_NONE(p) ((p)->address_family == AF_UNSPEC)
#define IS_IPADDR_V4(p) ((p)->address_family == AF_INET)
#define IS_IPADDR_V6(p) ((p)->address_family == AF_INET6)
#define SET_IPADDR_V4(p) (p)->address_family = AF_INET
#define SET_IPADDR_V6(p) (p)->address_family = AF_INET6

#define IPADDRSZ(p) \
    (IS_IPADDR_V4((p)) ? sizeof(struct in_addr) : sizeof(struct in6_addr))

#define IPADDR_STRING_SIZE 46

static inline void ip_reset(struct ipaddr* ip) {
    memset(ip, 0, sizeof(*ip));
}

/* Fill in an ipaddr using the given family-specific struct. */
extern void ip_from_ipv4(const struct in_addr* ipv4, struct ipaddr* ip);
extern void ip_from_ipv6(const struct in6_addr* ipv6, struct ipaddr* ip);

/* Fill in the given family-specific struct using the given ipaddr. */
extern void ip_to_ipv4(const struct ipaddr* ip, struct in_addr* ipv4);
extern void ip_to_ipv6(const struct ipaddr* ip, struct in6_addr* ipv6);

/* Return the number of bytes in the on-the-wire representation of
 * addresses of the given family.
 */
extern int ipaddr_length(int address_family);

/* Return the number of bytes in sockaddr of the given family. */
extern int sockaddr_length(int address_family);

/* Return true if the two addresses are the same. */
static inline bool is_equal_ip(const struct ipaddr* a,
                               const struct ipaddr* b) {
    return ((a->address_family == b->address_family) &&
            !memcmp(&a->ip, &b->ip, ipaddr_length(a->address_family)));
}

/* Parse a human-readable IPv4 address and return it. Print an error
 * to stderr and exit if there is an error parsing the address.
 */
extern struct ipaddr ipv4_parse(const char* ip_string);

/* Parse a human-readable IPv6 address and return it. Print an error
 * to stderr and exit if there is an error parsing the address.
 */
extern struct ipaddr ipv6_parse(const char* ip_string);

/* Attempt to parse a human-readable IPv4/IPv6 address and return it. Return
 * STATUS_OK on success, or STATUS_ERR on failure (meaning the input string was
 * not actually a valid IPv4 or IPv6 address).
 */
extern int ip_parse(const char* ip_string, struct ipaddr* ip);

/* Print a human-readable representation of the given IP address in the
 * given buffer, which must be at least ADDR_STR_LEN bytes long.
 * Returns a pointer to the given buffer.
 */
extern const char* ip_to_string(const struct ipaddr* ip, char* buffer);

/* Convert a string containing a human-readable DNS name or IPv/IPv6 address
 * into an IP address struct. Return STATUS_OK on success. Upon failure (i.e.,
 * the input host string was not actually a valid DNS name, IPv4 address, or
 * IPv6 address) returns STATUS_ERR and fills in *error with a malloc-allocated
 * error message.
 */
extern int string_to_ip(const char* host, struct ipaddr* ip, char** error);

/* Create an IPv4-mapped IPv6 address. */
extern struct ipaddr ipv6_map_from_ipv4(const struct ipaddr ipv4);

/* Deconstruct an IPv4-mapped IPv6 address and fill in *ipv4 with the
 * IPv4 address that was mapped into IPv6 space. Return STATUS_OK on
 * success, or STATUS_ERR on failure (meaning the input ipv6 was not
 * actually an IPv4-mapped IPv6 address).
 */
extern int ipv6_map_to_ipv4(const struct ipaddr ipv6,
                            struct ipaddr* ipv4);

/* Fill in a sockaddr struct and socklen_t using the given IP and port.
 * The IP address may be IPv4 or IPv6.
 */
extern void ip_to_sockaddr(const struct ipaddr* ip, uint16_t port,
                           struct sockaddr* address, socklen_t* length);

/* Fill in an IP address and port by parsing a sockaddr struct and
 * socklen_t using the given IP and port. The IP address may be IPv4
 * or IPv6. Exits with an error message if the address family is other
 * than AF_INET or AF_INET6.
 */
extern void ip_from_sockaddr(const struct sockaddr* address, socklen_t length,
                             struct ipaddr* ip, uint16_t* port);

/* Return true if the address is that of a local interface. */
/* Note: this should return bool, but that doesn't compile on NetBSD. */
extern int is_ip_local(const struct ipaddr* ip);

/* Fill in the name of the device configured with the given IP, if
 * any. The dev_name buffer should be at least IFNAMSIZ bytes.
 * Return true if the IP is found on a local device.
 */
/* Note: this should return bool, but that doesn't compile on NetBSD. */
extern int get_ip_device(const struct ipaddr* ip, char* dev_name);

/* Convert dotted decimal netmask to equivalent CIDR prefix length */
extern int netmask_to_prefix(const char* netmask);

bool is_ipv4(const char* host);
bool is_ipv6(const char* host);
bool is_ipaddr(const char* host);

void generate_random_ipv4_addr(char* result, const char* base,
                               const char* netmask);

void generate_random_ipv6_addr(char* result, const char* base, int prefixlen);

#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001 /* Internet address 127.0.0.1.  */
#endif

/* Address family numbers from RFC1700. */
typedef enum {
    AFI_UNSPEC = 0,
    AFI_IP = 1,
    AFI_IP6 = 2,
    AFI_L2VPN = 3,
    AFI_MAX = 4
} afi_t;

#define IS_VALID_AFI(a) ((a) > AFI_UNSPEC && (a) < AFI_MAX)

/* Subsequent Address Family Identifier. */
typedef enum {
    SAFI_UNSPEC = 0,
    SAFI_UNICAST = 1,
    SAFI_MULTICAST = 2,
    SAFI_MPLS_VPN = 3,
    SAFI_ENCAP = 4,
    SAFI_EVPN = 5,
    SAFI_LABELED_UNICAST = 6,
    SAFI_FLOWSPEC = 7,
    SAFI_MAX = 8
} safi_t;

#define FOREACH_AFI_SAFI(afi, safi)          \
    for (afi = AFI_IP; afi < AFI_MAX; afi++) \
        for (safi = SAFI_UNICAST; safi < SAFI_MAX; safi++)

#define FOREACH_AFI_SAFI_NSF(afi, safi)      \
    for (afi = AFI_IP; afi < AFI_MAX; afi++) \
        for (safi = SAFI_UNICAST; safi <= SAFI_MPLS_VPN; safi++)
#endif