#include "packet_socket.h"

#include "defs.h"
#include "log.h"
#include "net_utils.h"
#include "str.h"

/* Number of bytes to buffer in the packet socket we use for sniffing. */
static const int PACKET_SOCKET_RCVBUF_BYTES = 2 * 1024 * 1024;

/* Set the receive buffer for a socket to the given size in bytes. */
static void set_receive_buffer_size(int fd, int bytes) {
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &bytes, sizeof(bytes)) < 0) {
        perror("setsockopt SOL_SOCKET SO_RCVBUF");
        exit(EXIT_FAILURE);
    }
}

int verbose_popen(char *cmd, char **rst) {
    int ret = -1;
    char cmd_buff[128] = {0};
    char rst_buff[128];  //= {0};
    strcpy(cmd_buff, cmd);
    FILE *ptr;
    if ((ptr = popen(cmd, "r")) != NULL) {
        while (fgets(rst_buff, sizeof(rst_buff), ptr) != NULL) {
            strcat(rst, rst_buff);
            if (strlen(rst) > sizeof(rst_buff)) {
                break;
            }
        }
        pclose(ptr);
        ptr = NULL;
        ret = 0;
    } else {
        ret = -1;
    }

    return ret;
}

/* Bind the packet socket with the given fd to the given interface. */
static void bind_to_interface(int fd, int interface_index) {
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = interface_index;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        perror("bind packet socket");
        exit(EXIT_FAILURE);
    }
}

/* Allocate and configure a packet socket just like the one tcpdump
 * uses. We do this so we can get timestamps on the outbound packets
 * the kernel sends, to verify the correct timing (tun devices do not
 * take timestamps). To reduce CPU load and filtering complexity, we
 * bind the socket to a single device so we only receive packets for
 * that device.
 */
static void packet_socket_setup(struct packet_socket *psock) {
    struct timeval tv;

    psock->packet_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (psock->packet_fd < 0) {
        perror("socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))");
        exit(EXIT_FAILURE);
    }

    psock->index = if_nametoindex(psock->name);
    if (psock->index == 0) {
        perror("if_nametoindex");
        exit(EXIT_FAILURE);
    }
    log_debug("device index: %s -> %d", psock->name, psock->index);

    bind_to_interface(psock->packet_fd, psock->index);

    set_receive_buffer_size(psock->packet_fd, PACKET_SOCKET_RCVBUF_BYTES);

    /* Pay the non-trivial latency cost to enable timestamps now, before
     * the test starts, to avoid significant delays in the middle of tests.
     */
    ioctl(psock->packet_fd, SIOCGSTAMP, &tv);
}

struct packet_socket *packet_socket_new(const char *device_name) {
    struct packet_socket *psock = calloc(1, sizeof(struct packet_socket));

    psock->name = strdup(device_name);
    psock->packet_fd = -1;

    packet_socket_setup(psock);

    return psock;
}

void packet_socket_free(struct packet_socket *psock) {
    if (psock->packet_fd >= 0) close(psock->packet_fd);

    if (psock->name != NULL) free(psock->name);

    memset(psock, 0, sizeof(*psock)); /* paranoia to catch bugs*/
    free(psock);
}

/* Add a filter so we only sniff packets we want. */
void packet_socket_set_filter(struct packet_socket *psock,
                              struct sock_filter *filter, int len) {
    struct sock_fprog bpfcode = {
        .len = len,
        .filter = filter,
    };

    log_debug("filter constants:");
    for (int i = 0; i < bpfcode.len; ++i)
        log_debug("{ 0x%02x, %3d, %3d, 0x%08x },", bpfcode.filter[i].code,
                  bpfcode.filter[i].jt, bpfcode.filter[i].jf,
                  bpfcode.filter[i].k);

    /* Attach the filter. */
    if (setsockopt(psock->packet_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bpfcode,
                   sizeof(bpfcode)) < 0) {
        err_quit("setsockopt SOL_SOCKET, SO_ATTACH_FILTER");
    }
}

void packet_socket_set_filter_str(struct packet_socket *psock, const char *fs) {

    // check filter string based on tcpdump command
    char *command = str("tcpdump -i lo %s -ddd", fs);
    char *result = NULL;
    int status = system(str("%s > /dev/null 2>&1", command));
    if (status != 0) {
        log_debug("error filter string '%s'", fs);
        return;
    }

    // parse filter bytecode based on tcpdump command
    struct sock_filter *filter;
    char line[64];
    FILE *ptr = popen(command, "r");
    if (!ptr) log_info("%p", ptr);
    fgets(line, sizeof(line), ptr);
    int len = str2int(str_rtrim(line, '\n'));
    filter = calloc(len, sizeof(*filter));
    for (int i = 0; i < len; i++) {
        sscanf(fgets(line, sizeof(line), ptr), "%d %d %d %d\n",
               &(filter + i)->code, &(filter + i)->jt, &(filter + i)->jf,
               &(filter + i)->k);
    }

    // set filter
    packet_socket_set_filter(psock, filter, len);
}

/**
 * Examples:
 *
 * struct iovec ether_frame[2];
 * ether_frame[0].iov_base = &ether; // Ether header
 * ether_frame[0].iov_len = sizeof(ether);
 * ether_frame[1].iov_base = &ip; // IP datagram.
 * ether_frame[1].iov_len = sizeof(ip);
 * result = packet_socket_writev(psock, ether_frame, ARRAY_SIZE(ether_frame));
 */
int packet_socket_writev(struct packet_socket *psock, const struct iovec *iov,
                         int iovcnt) {
    if (writev(psock->packet_fd, iov, iovcnt) < 0) {
        perror("writev");
        return STATUS_ERR;
    }
    return STATUS_OK;
}

int packet_socket_send(struct packet_socket *psock, unsigned char *data,
                       int datalen) {
    if (send(psock->packet_fd, data, datalen, 0) < 0) {
        perror("send");
        return STATUS_ERR;
    }
    return STATUS_OK;
}

int packet_socket_receive(struct packet_socket *psock,
                          enum direction_t direction, signed int timeout_secs,
                          struct packet *packet, int *in_bytes) {
    struct sockaddr_ll from;
    memset(&from, 0, sizeof(from));
    socklen_t from_len = sizeof(from);

    /* Change the socket to timeout after a certain period.
     * We set the timeout to be the maximum of the expected_usecs
     * and expected_usecs_end computed in verify_time so we wait long
     * enough regardless of the packet time type.
     */
    struct timeval sock_timeout = {.tv_sec = timeout_secs, .tv_usec = 0};
    if (timeout_secs == TIMEOUT_NONE) sock_timeout.tv_sec = 0;
    setsockopt(psock->packet_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout,
               sizeof(sock_timeout));

    /* Read the packet out of our kernel packet socket buffer. */
    *in_bytes = recvfrom(psock->packet_fd, packet->buffer, packet->buffer_bytes,
                         0, (struct sockaddr *)&from, &from_len);

    /* Set the socket back to its blocking state. */
    sock_timeout.tv_sec = 0;
    setsockopt(psock->packet_fd, SOL_SOCKET, SO_RCVTIMEO, &sock_timeout,
               sizeof(sock_timeout));
    /* Return an error if we timed out */
    if (*in_bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return STATUS_TIMEOUT;

    assert(*in_bytes <= packet->buffer_bytes);
    if (*in_bytes < 0) {
        if (errno == EINTR) {
            log_debug("EINTR\n");
            return STATUS_ERR;
        } else {
            perror("packet socket recvfrom()");
            exit(EXIT_FAILURE);
        }
    }

    if (direction == DIRECTION_OUTBOUND &&
        from.sll_pkttype != PACKET_OUTGOING) {
        log_debug("not outbound (%d)", from.sll_pkttype);
        return STATUS_ERR;
    }

    if (direction == DIRECTION_INBOUND && from.sll_pkttype != PACKET_HOST) {
        log_debug("not inbound (%d)", from.sll_pkttype);
        return STATUS_ERR;
    }

    /* We only want packets on our tun device. The kernel
     * can put packets for other devices in our receive
     * buffer before we bind the packet socket to the tun
     * device.
     */
    if (from.sll_ifindex != psock->index) {
        log_debug("not correct index (%d)", from.sll_ifindex);
        return STATUS_ERR;
    }

    /* Get the time at which the kernel sniffed the packet. */
    struct timeval tv;
    if (ioctl(psock->packet_fd, SIOCGSTAMP, &tv) < 0) {
        perror("SIOCGSTAMP");
        exit(EXIT_FAILURE);
    }

    if (from.sll_pkttype == PACKET_OUTGOING) {
        log_debug("sniffed %d bytes packet sent to ifindex %d at %u.%u ",
                  *in_bytes, from.sll_ifindex, tv.tv_sec, tv.tv_usec);
    } else {
        log_debug("sniffed %d bytes packet received from ifindex %d at %u.%u",
                  *in_bytes, from.sll_ifindex, tv.tv_sec, tv.tv_usec);
    }

    return STATUS_OK;
}
