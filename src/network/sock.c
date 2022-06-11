

#include "sock.h"

#include <linux/if_packet.h>
#include <netinet/ether.h>

#include "ifi.h"

int sock_packet(int type, int proto, const char *ifname) {
    int sockfd;

    sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_ll));
    saddr.sll_family = PF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    if (ifname)
        saddr.sll_ifindex = if_nametoindex(ifname);

    bind(sockfd, (struct sockaddr *)&saddr, sizeof(saddr));
}