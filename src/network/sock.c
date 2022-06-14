#include "sock.h"

#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <string.h>

#include "ifi.h"
#include "wrapper.h"

int sock_packet(int type, int proto, const char *ifname) {
    int sockfd;

    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_ll));
    saddr.sll_family = PF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    if (ifname) saddr.sll_ifindex = if_nametoindex(ifname);

    bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
}

const char *str_fam(int family) {
    if (family == AF_INET) return ("AF_INET");
    if (family == AF_INET6) return ("AF_INET6");
    if (family == AF_LOCAL) return ("AF_LOCAL");
    return ("<unknown family>");
}

const char *str_sock(int socktype) {
    switch (socktype) {
        case SOCK_STREAM:
            return "SOCK_STREAM";
        case SOCK_DGRAM:
            return "SOCK_DGRAM";
        case SOCK_RAW:
            return "SOCK_RAW";
#ifdef SOCK_RDM
        case SOCK_RDM:
            return "SOCK_RDM";
#endif
#ifdef SOCK_SEQPACKET
        case SOCK_SEQPACKET:
            return "SOCK_SEQPACKET";
#endif
        default:
            return "<unknown socktype>";
    }
}