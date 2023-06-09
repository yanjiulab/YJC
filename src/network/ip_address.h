#ifndef __IP_ADDRESS_H__
#define __IP_ADDRESS_H__
#include <netinet/in.h>
#include <string.h>
/* IPv4 or IPv6 address. */
struct ip_address {
    int address_family; /* AF_INET or AF_INET6 */
    union {
        struct in_addr v4;
        struct in6_addr v6;
        uint8_t bytes[16];
    } ip; /* IP address (network order) */
};

static inline void ip_reset(struct ip_address *ip) {
    memset(ip, 0, sizeof(*ip));
}

/* Fill in an ip_address using the given family-specific struct. */
extern void ip_from_ipv4(const struct in_addr *ipv4, struct ip_address *ip);
extern void ip_from_ipv6(const struct in6_addr *ipv6, struct ip_address *ip);

/* Fill in the given family-specific struct using the given ip_address. */
extern void ip_to_ipv4(const struct ip_address *ip, struct in_addr *ipv4);
extern void ip_to_ipv6(const struct ip_address *ip, struct in6_addr *ipv6);

/* Return the number of bytes in the on-the-wire representation of
 * addresses of the given family.
 */
extern int ip_address_length(int address_family);

/* Return the number of bytes in sockaddr of the given family. */
extern int sockaddr_length(int address_family);

/* Return true if the two addresses are the same. */
static inline bool is_equal_ip(const struct ip_address *a,
                               const struct ip_address *b) {
    return ((a->address_family == b->address_family) &&
            !memcmp(&a->ip, &b->ip, ip_address_length(a->address_family)));
}

/* Parse a human-readable IPv4 address and return it. Print an error
 * to stderr and exit if there is an error parsing the address.
 */
extern struct ip_address ipv4_parse(const char *ip_string);

/* Parse a human-readable IPv6 address and return it. Print an error
 * to stderr and exit if there is an error parsing the address.
 */
extern struct ip_address ipv6_parse(const char *ip_string);

/* Attempt to parse a human-readable IPv4/IPv6 address and return it. Return
 * STATUS_OK on success, or STATUS_ERR on failure (meaning the input string was
 * not actually a valid IPv4 or IPv6 address).
 */
extern int ip_parse(const char *ip_string, struct ip_address *ip);

/* Print a human-readable representation of the given IP address in the
 * given buffer, which must be at least ADDR_STR_LEN bytes long.
 * Returns a pointer to the given buffer.
 */
extern const char *ip_to_string(const struct ip_address *ip, char *buffer);

/* Convert a string containing a human-readable DNS name or IPv/IPv6 address
 * into an IP address struct. Return STATUS_OK on success. Upon failure (i.e.,
 * the input host string was not actually a valid DNS name, IPv4 address, or
 * IPv6 address) returns STATUS_ERR and fills in *error with a malloc-allocated
 * error message.
 */
extern int string_to_ip(const char *host, struct ip_address *ip, char **error);

/* Create an IPv4-mapped IPv6 address. */
extern struct ip_address ipv6_map_from_ipv4(const struct ip_address ipv4);

/* Deconstruct an IPv4-mapped IPv6 address and fill in *ipv4 with the
 * IPv4 address that was mapped into IPv6 space. Return STATUS_OK on
 * success, or STATUS_ERR on failure (meaning the input ipv6 was not
 * actually an IPv4-mapped IPv6 address).
 */
extern int ipv6_map_to_ipv4(const struct ip_address ipv6,
                            struct ip_address *ipv4);

/* Fill in a sockaddr struct and socklen_t using the given IP and port.
 * The IP address may be IPv4 or IPv6.
 */
extern void ip_to_sockaddr(const struct ip_address *ip, uint16_t port,
                           struct sockaddr *address, socklen_t *length);

/* Fill in an IP address and port by parsing a sockaddr struct and
 * socklen_t using the given IP and port. The IP address may be IPv4
 * or IPv6. Exits with an error message if the address family is other
 * than AF_INET or AF_INET6.
 */
extern void ip_from_sockaddr(const struct sockaddr *address, socklen_t length,
                             struct ip_address *ip, uint16_t *port);

/* Return true iff the address is that of a local interface. */
/* Note: this should return bool, but that doesn't compile on NetBSD. */
extern int is_ip_local(const struct ip_address *ip);

/* Fill in the name of the device configured with the given IP, if
 * any. The dev_name buffer should be at least IFNAMSIZ bytes.
 * Return true iff the IP is found on a local device.
 */
/* Note: this should return bool, but that doesn't compile on NetBSD. */
extern int get_ip_device(const struct ip_address *ip, char *dev_name);

/* Convert dotted decimal netmask to equivalent CIDR prefix length */
extern int netmask_to_prefix(const char *netmask);

void generate_random_ipv4_addr(char *result, const char *base,
                               const char *netmask);

void generate_random_ipv6_addr(char *result, const char *base, int prefixlen);

#endif