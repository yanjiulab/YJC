/*
 * Copyright 2013-2015 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Author: ncardwell@google.com (Neal Cardwell)
 *
 * Implementation for operations for IPv4 and IPv6 addresses.
 */

#include "ipaddr.h"

#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "defs.h"
#include "log.h"
#include "packet.h"

/* IPv6 prefix for IPv4-mapped addresses. These are in the
 * ::FFFF:0:0/96 space, i.e. 10 bytes of 0x00 and 2 bytes of 0xFF. See
 * RFC 4291 ("IPv6 Addressing Architecture") section 2.5.5.2
 * ("IPv4-Mapped IPv6 Address").
 */
const uint8_t ipv4_mapped_prefix[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF};

int ipaddr_length(int address_family) {
    switch (address_family) {
    case AF_INET:
        return sizeof(struct in_addr);
    case AF_INET6:
        return sizeof(struct in6_addr);
    default:
        log_error("ipaddr_length: bad address family: %d\n",
                  address_family);
        break;
    }
    return 0; /* not reached */
}

int sockaddr_length(int address_family) {
    switch (address_family) {
    case AF_INET:
        return sizeof(struct sockaddr_in);
    case AF_INET6:
        return sizeof(struct sockaddr_in6);
    default:
        log_error("sockaddr_length: bad address family: %d\n",
                  address_family);
        break;
    }
    return 0; /* not reached */
}

static void ipv4_init(struct ipaddr* ipv4) {
    memset(ipv4, 0, sizeof(*ipv4));
    ipv4->address_family = AF_INET;
}

static void ipv6_init(struct ipaddr* ipv6) {
    memset(ipv6, 0, sizeof(*ipv6));
    ipv6->address_family = AF_INET6;
}

void ip_from_ipv4(const struct in_addr* ipv4, struct ipaddr* ip) {
    ipv4_init(ip);
    ip->ip.v4 = *ipv4;
}

void ip_from_ipv6(const struct in6_addr* ipv6, struct ipaddr* ip) {
    ipv6_init(ip);
    ip->ip.v6 = *ipv6;
}

void ip_to_ipv4(const struct ipaddr* ip, struct in_addr* ipv4) {
    *ipv4 = ip->ip.v4;
}

void ip_to_ipv6(const struct ipaddr* ip, struct in6_addr* ipv6) {
    *ipv6 = ip->ip.v6;
}

struct ipaddr ipv4_parse(const char* ip_string) {
    struct ipaddr ipv4;
    ipv4_init(&ipv4);

    if (inet_pton(AF_INET, ip_string, &ipv4.ip.v4) != 1)
        log_error("bad IPv4 address: %s\n", ip_string);

    return ipv4;
}

struct ipaddr ipv6_parse(const char* ip_string) {
    struct ipaddr ipv6;
    ipv6_init(&ipv6);

    if (inet_pton(AF_INET6, ip_string, &ipv6.ip.v6) != 1)
        log_error("bad IPv6 address: %s\n", ip_string);

    return ipv6;
}

int ip_parse(const char* ip_string, struct ipaddr* ip) {
    ipv6_init(ip);
    if (inet_pton(AF_INET6, ip_string, &ip->ip.v6) == 1)
        return STATUS_OK;

    ipv4_init(ip);
    if (inet_pton(AF_INET, ip_string, &ip->ip.v4) == 1)
        return STATUS_OK;

    return STATUS_ERR;
}

const char* ip_to_string(const struct ipaddr* ip, char* buffer) {
    if (!buffer) {
        buffer = malloc(ADDR_STR_LEN);
    }

    if (!inet_ntop(ip->address_family, &ip->ip, buffer, ADDR_STR_LEN))
        log_error("inet_ntop");

    return buffer;
}

extern int string_to_ip(const char* host, struct ipaddr* ip, char** error) {
    int status;
    uint16_t port = 0;
    struct addrinfo *results = NULL, *result = NULL;

    status = getaddrinfo(host, NULL, NULL, &results);
    if (status) {
        asprintf(error, "getaddrinfo: %s\n", gai_strerror(status));
        return STATUS_ERR;
    }

    status = STATUS_ERR;
    for (result = results; result != NULL; result = result->ai_next) {
        if (result->ai_family == AF_INET || result->ai_family == AF_INET6) {
            ip_from_sockaddr(result->ai_addr, result->ai_addrlen, ip, &port);
            status = STATUS_OK;
            break;
        }
    }
    freeaddrinfo(results);
    return status;
}

struct ipaddr ipv6_map_from_ipv4(const struct ipaddr ipv4) {
    struct ipaddr ipv6;
    ipv6_init(&ipv6);

    assert(sizeof(ipv4.ip.v4) + sizeof(ipv4_mapped_prefix) ==
           sizeof(ipv6.ip.v6));
    memcpy(ipv6.ip.v6.s6_addr, ipv4_mapped_prefix, sizeof(ipv4_mapped_prefix));
    memcpy(ipv6.ip.v6.s6_addr + sizeof(ipv4_mapped_prefix), &ipv4.ip.v4,
           sizeof(ipv4.ip.v4));
    return ipv6;
}

int ipv6_map_to_ipv4(const struct ipaddr ipv6, struct ipaddr* ipv4) {
    if (memcmp(&ipv6.ip.v6.s6_addr, ipv4_mapped_prefix,
               sizeof(ipv4_mapped_prefix)) == 0) {
        ipv4_init(ipv4);
        memcpy(&ipv4->ip.v4, ipv6.ip.v6.s6_addr + sizeof(ipv4_mapped_prefix),
               sizeof(ipv4->ip.v4));
        return STATUS_OK;
    } else {
        return STATUS_ERR;
    }
}

/* Fill in a sockaddr struct and socklen_t using the given IPv4
 * address and port.
 */
static void ipv4_to_sockaddr(const struct ipaddr* ipv4, uint16_t port,
                             struct sockaddr* address, socklen_t* length) {
    struct sockaddr_in sa_v4;
    memset(&sa_v4, 0, sizeof(sa_v4));
    sa_v4.sin_family = AF_INET;
    sa_v4.sin_port = htons(port);
    memcpy(&sa_v4.sin_addr, &ipv4->ip.v4, sizeof(sa_v4.sin_addr));
    *length = sizeof(sa_v4);
    memcpy(address, &sa_v4, *length);
}

/* Fill in a sockaddr struct and socklen_t using the given IPv6
 * address and port.
 */
static void ipv6_to_sockaddr(const struct ipaddr* ipv6, uint16_t port,
                             struct sockaddr* address, socklen_t* length) {
    struct sockaddr_in6 sa_v6;
    memset(&sa_v6, 0, sizeof(sa_v6));
#ifndef linux
    sa_v6.sin6_len = sizeof(sa_v6);
#endif
    sa_v6.sin6_family = AF_INET6;
    sa_v6.sin6_port = htons(port);
    memcpy(&sa_v6.sin6_addr, &ipv6->ip.v6, sizeof(sa_v6.sin6_addr));
    *length = sizeof(sa_v6);
    memcpy(address, &sa_v6, *length);
}

void ip_to_sockaddr(const struct ipaddr* ip, uint16_t port,
                    struct sockaddr* address, socklen_t* length) {
    switch (ip->address_family) {
    case AF_INET:
        ipv4_to_sockaddr(ip, port, address, length);
        break;
    case AF_INET6:
        ipv6_to_sockaddr(ip, port, address, length);
        break;
    default:
        log_error("ip_to_sockaddr: bad address family: %d\n",
                  ip->address_family);
        break;
    }
}

/* Extract and return the IPv4 address and port from the given sockaddr. */
static void ipv4_from_sockaddr(const struct sockaddr* address, socklen_t length,
                               struct ipaddr* ipv4, uint16_t* port) {
    assert(address->sa_family == AF_INET);
    ipv4_init(ipv4);

    struct sockaddr_in sa_v4;
    assert(length == sizeof(sa_v4));
    memcpy(&sa_v4, address, length); /* to avoid aliasing issues */
    ipv4->ip.v4 = sa_v4.sin_addr;
    *port = ntohs(sa_v4.sin_port);
}

/* Extract and return the IPv6 address and port from the given sockaddr. */
static void ipv6_from_sockaddr(const struct sockaddr* address, socklen_t length,
                               struct ipaddr* ipv4, uint16_t* port) {
    assert(address->sa_family == AF_INET6);
    ipv6_init(ipv4);

    struct sockaddr_in6 sa_v6;
    assert(length == sizeof(sa_v6));
    memcpy(&sa_v6, address, length); /* to avoid aliasing issues */
    ipv4->ip.v6 = sa_v6.sin6_addr;
    *port = ntohs(sa_v6.sin6_port);
}

void ip_from_sockaddr(const struct sockaddr* address, socklen_t length,
                      struct ipaddr* ip, uint16_t* port) {
    switch (address->sa_family) {
    case AF_INET:
        ipv4_from_sockaddr(address, length, ip, port);
        break;
    case AF_INET6:
        ipv6_from_sockaddr(address, length, ip, port);
        break;
    default:
        log_error("ip_from_sockaddr: bad address family: %d\n",
                  address->sa_family);
        break;
    }
}

int get_ip_device(const struct ipaddr* ip, char* dev_name) {
    struct ifaddrs *ifaddr_list, *ifaddr;
    bool is_local = false;

    if (getifaddrs(&ifaddr_list))
        log_error("getifaddrs");

    for (ifaddr = ifaddr_list; ifaddr != NULL; ifaddr = ifaddr->ifa_next) {
        int family;
        struct ipaddr interface_ip;
        uint16_t port;

        if (ifaddr->ifa_addr == NULL)
            continue;

        family = ifaddr->ifa_addr->sa_family;
        if (family != ip->address_family)
            continue;

        ip_from_sockaddr(ifaddr->ifa_addr, sockaddr_length(family),
                         &interface_ip, &port);
        if (is_equal_ip(ip, &interface_ip)) {
            assert(ifaddr->ifa_name);
            assert(strlen(ifaddr->ifa_name) < IFNAMSIZ);
            strcpy(dev_name, ifaddr->ifa_name);
            is_local = true;
            break;
        }
    }

    freeifaddrs(ifaddr_list);

    return is_local;
}

int is_ip_local(const struct ipaddr* ip) {
    char dev_name[IFNAMSIZ];

    return get_ip_device(ip, dev_name);
}

bool is_ipv4(const char* host) {
    struct sockaddr_in sin;
    return inet_pton(AF_INET, host, &sin) == 1;
}

bool is_ipv6(const char* host) {
    struct sockaddr_in6 sin6;
    return inet_pton(AF_INET6, host, &sin6) == 1;
}

bool is_ipaddr(const char* host) { return is_ipv4(host) || is_ipv6(host); }

int netmask_to_prefix(const char* netmask) {
    int pos;
    struct ipaddr mask = ipv4_parse(netmask);
    uint32_t mask_addr = ntohl(mask.ip.v4.s_addr);
    int prefix_len = 0;

    for (pos = 31; pos >= 0; --pos) {
        if (!(mask_addr & (1 << pos)))
            break;
        ++prefix_len;
    }
    return prefix_len;
}

static int urandom_read(void* buffer, int sz) {
    static int fd_urandom = -1;

    if (fd_urandom == -1)
        fd_urandom = open("/dev/urandom", O_RDONLY);
    return read(fd_urandom, buffer, sz);
}

void generate_random_ipv4_addr(char* result, const char* base,
                               const char* netmask) {
    int prefix_len = netmask_to_prefix(netmask);
    struct ipaddr addr = ipv4_parse(base);

    if (prefix_len < 31) {
        unsigned int rnd;

        if (urandom_read(&rnd, sizeof(rnd)) == sizeof(rnd)) {
            if (prefix_len) {
                uint32_t mask = (1U << (32 - prefix_len)) - 1;

                rnd &= mask;
                /* .0 is reserved for network address.
                 * .1 is reserved for the gateway
                 */
                if (rnd < 2)
                    rnd = 2;
                /* .255.255 is reserved for net broadcast */
                if (rnd == mask)
                    rnd--;
            }
            addr.ip.v4.s_addr |= htonl(rnd);
        }
    }
    ip_to_string(&addr, result);
}

/* In this version, we randomize last 32bits (or less) of the address.
 * There is no need to fully use RFC 4193 range.
 * ( fd3d:fa7b:d17d::/48 in unique local address space )
 */
void generate_random_ipv6_addr(char* result, const char* base, int prefixlen) {
    struct ipaddr addr = ipv6_parse(base);
    unsigned int mask = ~0U, rnd = 0;

    urandom_read(&rnd, sizeof(rnd));
    if (prefixlen > 128 - 32) {
        mask = (1U << (128 - prefixlen)) - 1;
        rnd &= mask;
    }
    if (!rnd)
        rnd++;
    if (rnd == mask)
        rnd--;
    addr.ip.v6.s6_addr32[3] |= htonl(rnd);
    ip_to_string(&addr, result);
}
