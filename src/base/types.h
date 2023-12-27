#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

/* We use kernel-style names for standard integer types. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

/* Most functions in this codebase return one of these two values to let the
 * caller know whether there was a problem.
 */
enum status_t {
    STATUS_OK = 0,
    STATUS_ERR = -1,
    STATUS_WARN = -2,   /* a non-fatal error or warning */
    STATUS_TIMEOUT = -3 /* timeout sniffing outbound packet */
};

/* VRF ID type. */
typedef uint32_t vrf_id_t;

/* Interface index type. */
typedef signed int ifindex_t;

#endif