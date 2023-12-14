#include "packet_stringify.h"
#include "defs.h"
#include "ip_address.h"

void hex_dump(const uint8_t* buffer, int bytes, char** hex) {
    size_t size = 0;
    FILE* s = open_memstream(hex, &size); /* output string */
    int i;
    for (i = 0; i < bytes; ++i) {
        if (i % 16 == 0) {
            if (i > 0)
                fprintf(s, "\n");
            fprintf(s, "0x%04x: ", i); /* show buffer offset */
        }
        fprintf(s, "%02x ", buffer[i]);
    }
    fprintf(s, "\n");
    fclose(s);
}

static void packet_buffer_to_string(FILE* s, struct packet* packet) {
    char* hex = NULL;
    hex_dump(packet->buffer, packet->buffer_active, &hex);
    fputc('\n', s);
    fprintf(s, "%s", hex);
    free(hex);
}

char* to_printable_string(const char* in, int in_len) {
    int out_len, i, j;
    char* out;

    out_len = in_len * 4; /* escape code is 4B */
    out_len += 1;         /* terminating null */
    out = malloc(out_len);

    for (i = 0, j = 0; i < in_len; i++) {
        if (isprint(in[i]) ||
            (i == in_len - 1 && in[i] == 0) /* terminating null */)
            out[j++] = in[i];
        else
            j += sprintf(out + j, "\\x%02hhx", in[i]);
    }
    out[j] = 0;
    return out;
}

static void endpoints_to_string(FILE* s, const struct packet* packet) {
    char src_string[ADDR_STR_LEN];
    char dst_string[ADDR_STR_LEN];
    struct tuple tuple;

    get_packet_tuple(packet, &tuple);

    fprintf(s, "%s:%u > %s:%u",
            ip_to_string(&tuple.src.ip, src_string), ntohs(tuple.src.port),
            ip_to_string(&tuple.dst.ip, dst_string), ntohs(tuple.dst.port));
}

static int udp_packet_to_string(FILE* s, struct packet* packet,
                                enum dump_format_t format, char** error) {
    int result = STATUS_OK; /* return value */

    if ((format == DUMP_FULL) || (format == DUMP_VERBOSE)) {
        endpoints_to_string(s, packet);
        fputc(' ', s);
    }

    fprintf(s, "udp (%u)", packet_payload_len(packet));

    if (format == DUMP_VERBOSE)
        packet_buffer_to_string(s, packet);

    return result;
}

/* Print a string representation of the TCP packet:
 *  direction opt_ip_info flags seq ack window tcp_options
 */
static int tcp_packet_to_string(FILE* s, struct packet* packet,
                                enum dump_format_t format, char** error) {
    int result = STATUS_OK; /* return value */
    int ace = 0;

    if ((format == DUMP_FULL) || (format == DUMP_VERBOSE)) {
        endpoints_to_string(s, packet);
        fputc(' ', s);
    }

    /* We print flags in the same order as tcpdump 4.1.1. */
    if (packet->tcp->fin)
        fputc('F', s);
    if (packet->tcp->syn)
        fputc('S', s);
    if (packet->tcp->rst)
        fputc('R', s);
    if (packet->tcp->psh)
        fputc('P', s);
    if (packet->tcp->ack)
        fputc('.', s);
    if (packet->tcp->urg)
        fputc('U', s);
    // if (packet->flags & FLAG_PARSE_ACE) {
    //     if (packet->tcp->ece)
    //         ace |= 1;
    //     if (packet->tcp->cwr)
    //         ace |= 2;
    //     if (packet->tcp->ae)
    //         ace |= 4;
    //     fputc('0' + ace, s);
    // } else {
    if (packet->tcp->ece)
        fputc('E', s); /* ECN *E*cho sent (ECN) */
    if (packet->tcp->cwr)
        fputc('W', s); /* Congestion *W*indow reduced (ECN) */
    if (packet->tcp->ae)
        fputc('A', s); /* *A*ccurate ECN */
    // }

    fprintf(s, " %u:%u(%u) ",
            ntohl(packet->tcp->seq),
            ntohl(packet->tcp->seq) + packet_payload_len(packet),
            packet_payload_len(packet));

    if (packet->tcp->ack)
        fprintf(s, "ack %u ", ntohl(packet->tcp->ack_seq));

    // if (!(packet->flags & FLAG_WIN_NOCHECK))
    //     fprintf(s, "win %u ", ntohs(packet->tcp->window));

    // if (packet_tcp_options_len(packet) > 0) {
    //     char* tcp_options = NULL;
    //     if (tcp_options_to_string(packet, &tcp_options, error))
    //         result = STATUS_ERR;
    //     else
    //         fprintf(s, "<%s>", tcp_options);
    //     free(tcp_options);
    // }

    if (format == DUMP_VERBOSE)
        packet_buffer_to_string(s, packet);

    return result;
}

static int ipv4_header_to_string(FILE* s, struct packet* packet,
                                 enum dump_format_t format, char** error) {
    char src_string[ADDR_STR_LEN];
    char dst_string[ADDR_STR_LEN];
    struct ip_address src_ip, dst_ip;
    const struct ipv4* ipv4 = packet->ipv4;

    ip_from_ipv4(&ipv4->src_ip, &src_ip);
    ip_from_ipv4(&ipv4->dst_ip, &dst_ip);

    fprintf(s, "+ ipv4, %s > %s: \n",
            ip_to_string(&src_ip, src_string),
            ip_to_string(&dst_ip, dst_string));

    return STATUS_OK;
}

static int ipv6_header_to_string(FILE* s, struct packet* packet,
                                 enum dump_format_t format, char** error) {
    char src_string[ADDR_STR_LEN];
    char dst_string[ADDR_STR_LEN];
    struct ip_address src_ip, dst_ip;
    const struct ipv6* ipv6 = packet->ipv6;

    ip_from_ipv6(&ipv6->src_ip, &src_ip);
    ip_from_ipv6(&ipv6->dst_ip, &dst_ip);

    fprintf(s, "+ ipv6, %s > %s: \n",
            ip_to_string(&src_ip, src_string),
            ip_to_string(&dst_ip, dst_string));

    return STATUS_OK;
}

static int ethernet_packet_to_string(FILE* s, ethernet_t* eth) {

    int result = STATUS_OK; /* return value */

    fprintf(s,
            "> ether, %.2X-%.2X-%.2X-%.2X-%.2X-%.2X -> "
            "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X (%s: 0x%.4x)\n",
            eth->h_source[0], eth->h_source[1], eth->h_source[2],
            eth->h_source[3], eth->h_source[4], eth->h_source[5],
            eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3],
            eth->h_dest[4], eth->h_dest[5], ether_type2str(ntohs((unsigned short)eth->h_proto)),
            ntohs((unsigned short)eth->h_proto));

    return result;
}

int packet_stringify(struct packet* packet, enum dump_format_t format,
                     char** ascii_string, char** error) {
    assert(packet != NULL);
    int result = STATUS_ERR; /* return value */
    size_t size = 0;
    FILE* s = open_memstream(ascii_string, &size); /* output string */
    int i;

    // int header_count = packet_header_count(packet);
    if (packet->eth) {
        ethernet_packet_to_string(s, packet->eth);
    }

    if (packet->ipv4) {
        ipv4_header_to_string(s, packet, format, error);
    } else if (packet->ipv6) {
        ipv6_header_to_string(s, packet, format, error);
    } else {
        fprintf(s, "[NO IP HEADER]");
        packet_buffer_to_string(s, packet);
    }

    if (packet->tcp) {
        tcp_packet_to_string(s, packet, format, error);
    }
    if (packet->udp) {
        udp_packet_to_string(s, packet, format, error);
    }

    // packet_buffer_to_string(s, packet);

    // /* Print any encapsulation headers preceding layer 3 and 4 headers. */
    // for (i = 0; i < header_count - 2; ++i) {
    //     if (packet->headers[i].type == HEADER_NONE) break;
    //     if (encap_header_to_string(s, packet, i, format, error)) goto out;
    // }

    // if ((packet->ipv4 == NULL) && (packet->ipv6 == NULL)) {
    //     fprintf(s, "[NO IP HEADER]");
    // } else {
    //     if (packet->tcp != NULL) {
    //         if (tcp_packet_to_string(s, packet, format, error)) goto out;
    //     } else if (packet->udp != NULL) {
    //         if (udp_packet_to_string(s, packet, format, error)) goto out;
    //     } else if (packet->icmpv4 != NULL) {
    //         if (icmpv4_packet_to_string(s, packet, format, error)) goto out;
    //     } else if (packet->icmpv6 != NULL) {
    //         if (icmpv6_packet_to_string(s, packet, format, error)) goto out;
    //     } else {
    //         fprintf(s, "[NO TCP OR ICMP HEADER]");
    //     }
    // }

    result = STATUS_OK;

out:
    fclose(s);
    return result;
}
