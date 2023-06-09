#ifndef _YJC_NETDEV_H_
#define _YJC_NETDEV_H_
#include <ifaddrs.h>

#define NETDEV_INFO_PATH "/sys/class/net/"
#define RUNTIME_NETINFO_PATH "/proc/net/"
#define RUNTIME_NETDEV_STATISTICS "/proc/net/dev"
#define TUN_PATH "/dev/net/tun"

struct netdev_info {
    /* data */
};

// getifaddrs(unix), ioctl(linux)
struct netdev_info* netdev_get_info();
char* netdev_get_hwaddr(const char* ifname);

char* netdev_get_ip_address(const char* ifname);

/* Add the given IP address, with the given subnet/prefix length,
 * to the given device.
 */
extern void netdev_add_ip_address(const char* dev_name, const char* ip,
                                  int prefix_len);

/* Delete the given IP address, with the given subnet/prefix length,
 * from the given device.
 */
extern void netdev_del_ip_address(const char* dev_name, const char* ip,
                                  int prefix_len);

/* See if the given IP address, with the given subnet/prefix length,
 * is already on the given device. If so, return without doing
 * anything.  If not, delete it from any device it's currently on, and
 * add it to the given network device.
 */
extern void netdev_setup_ip_address(const char* dev_name, const char* ip,
                                    int prefix_len);

#endif