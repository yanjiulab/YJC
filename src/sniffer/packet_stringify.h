#ifndef __PACKET_STRINGIFY_H__
#define __PACKET_STRINGIFY_H__

#include "packet.h"

enum dump_format_t {
    DUMP_SHORT,   /* brief format used in scripts */
    DUMP_FULL,    /* add local and remote address and port */
    DUMP_VERBOSE, /* add hex dump */
};

/* Returns in *ascii_string a human-readable representation of the
 * packet 'packet'. Returns STATUS_OK on success; on failure returns
 * STATUS_ERR and sets error message.
 */
extern int packet_stringify(struct packet* packet,
                            enum dump_format_t format,
                            char** ascii_string, char** error);

#endif /* __PACKET_STRINGIFY_H__ */