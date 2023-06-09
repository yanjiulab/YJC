#include "packet_socket.h"

#include <errno.h>
#include <linux/if_ether.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

/* Set the receive buffer for a socket to the given size in bytes. */
static void set_receive_buffer_size(int fd, int bytes) {
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bytes, sizeof(bytes)) < 0)
        die_perror("setsockopt SOL_SOCKET SO_RCVBUF");
}

/* Bind the packet socket with the given fd to the given interface. */
static void bind_to_interface(int fd, int interface_index) {
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = interface_index;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0)
        die_perror("bind packet socket");
}