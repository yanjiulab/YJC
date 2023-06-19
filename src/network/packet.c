#include "packet.h"

/* ---------------------------- packet ---------------------------- */

struct packet *packet_new(uint32_t buffer_bytes) {
    struct packet *packet = calloc(1, sizeof(struct packet));
    packet->buffer = malloc(buffer_bytes);
    packet->buffer_bytes = buffer_bytes;
    return packet;
}

void packet_free(struct packet *packet) {
    free(packet->buffer);
    memset(packet, 0, sizeof(*packet)); /* paranoia to help catch bugs */
    free(packet);
}

struct packet_list *packet_list_new(void) {
    struct packet_list *list = calloc(1, sizeof(struct packet_list));
    list->packet = NULL;
    list->next = NULL;
    return list;
}

void packet_list_free(struct packet_list *list) {
    while (list != NULL) {
        struct packet_list *dead_list = list;
        if (list->packet) packet_free(list->packet);
        list = list->next;
        free(dead_list);
    }
}

/* ---------------------------- packet dump ---------------------------- */
static void packet_buffer_to_string(FILE *s, struct packet *packet) {
    char *hex = NULL;
    hex_dump(packet->buffer, packet->buffer_bytes, &hex);
    fputc('\n', s);
    fprintf(s, "%s", hex);
    free(hex);
}

int packet_to_string(struct packet *packet, enum dump_format_t format,
                     char **ascii_string, char **error) {
    assert(packet != NULL);
    int result = STATUS_ERR; /* return value */
    size_t size = 0;
    FILE *s = open_memstream(ascii_string, &size); /* output string */
    int i;
    
    packet_buffer_to_string(s, packet);

    // int header_count = packet_header_count(packet);

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

void hex_dump(const uint8_t *buffer, int bytes, char **hex) {
    size_t size = 0;
    FILE *s = open_memstream(hex, &size); /* output string */
    int i;
    for (i = 0; i < bytes; ++i) {
        if (i % 16 == 0) {
            if (i > 0) fprintf(s, "\n");
            fprintf(s, "0x%04x: ", i); /* show buffer offset */
        }
        fprintf(s, "%02x ", buffer[i]);
    }
    fprintf(s, "\n");
    fclose(s);
}

char *to_printable_string(const char *in, int in_len) {
    int out_len, i, j;
    char *out;

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

/* ---------------------------- packet parse ---------------------------- */

/* ---------------------------- packet generate ---------------------------- */