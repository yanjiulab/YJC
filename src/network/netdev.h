#ifndef _YJC_NETDEV_H_
#define _YJC_NETDEV_H_

#define NETDEV_INFO_PATH "/sys/class/net/"
#define RUNTIME_NETINFO_PATH "/proc/net/"
#define RUNTIME_NETDEV_STATISTICS "/proc/net/dev"

struct netdev {
    /* data */
};

char* netdev_get_hwaddr(const char* ifname);
char* netdev_get_ipaddr(const char* ifname);

#endif