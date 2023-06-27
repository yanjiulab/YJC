#include "netdev.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"
#include "ip_address.h"
#include "log.h"

static void verbose_system(const char *command) {
    int result;

    log_debug("running: '%s'", command);
    result = system(command);
    log_debug("result: %d", result);
    if (result != 0) log_debug("error executing command '%s'", command);
}

/* Configure a local IPv4 address and netmask for the device */
static void net_add_ipv4_address(const char *dev_name,
                                 const struct ip_address *ip, int prefix_len) {
    char *command = NULL;
    char ip_string[ADDR_STR_LEN];

    ip_to_string(ip, ip_string);

    asprintf(&command, "ip addr add %s/%d dev %s > /dev/null 2>&1", ip_string,
             prefix_len, dev_name);

    // asprintf(&command, "/sbin/ifconfig %s %s/%d alias", dev_name, ip_string,
    //          prefix_len);

    verbose_system(command);
    free(command);
}

/* Configure a local IPv6 address and prefix length for the device */
static void net_add_ipv6_address(const char *dev_name,
                                 const struct ip_address *ip, int prefix_len) {
    char *command = NULL;
    char ip_string[ADDR_STR_LEN];

    ip_to_string(ip, ip_string);

    asprintf(&command, "ip addr add %s/%d dev %s > /dev/null 2>&1", ip_string,
             prefix_len, dev_name);

    // asprintf(&command, "/sbin/ifconfig %s inet6 %s/%d", dev_name, ip_string,
    //          prefix_len);

    verbose_system(command);
    free(command);

    /* Wait for IPv6 duplicate address detection to converge,
     * so that this address no longer shows as "tentative".
     * e.g. "ip addr show" shows:
     * inet6 fd3d:fa7b:d17d::36/48 scope global tentative
     */
    if (!strstr(dev_name, "tun")) sleep(2);
}

void netdev_add_ip_address(const char *dev_name, const char *ip_str,
                           int prefix_len) {
    struct ip_address ip;
    int status = ip_parse(ip_str, &ip);

    if (status == STATUS_ERR) {
        log_debug("can not parse ip address: %s", ip_str);
        return;
    }

    switch (ip.address_family) {
        case AF_INET:
            net_add_ipv4_address(dev_name, &ip, prefix_len);
            break;
        case AF_INET6:
            net_add_ipv6_address(dev_name, &ip, prefix_len);
            break;
        default:
            assert(!"bad family");
            break;
    }
}

void netdev_del_ip_address(const char *dev_name, const char *ip_str,
                           int prefix_len) {
    char *command = NULL;
    char ip_string[ADDR_STR_LEN];

    struct ip_address ip;
    int status = ip_parse(ip_str, &ip);

    if (status == STATUS_ERR) {
        log_debug("can not parse ip address: %s", ip_str);
        return;
    }

    ip_to_string(&ip, ip_string);

    asprintf(&command, "ip addr del %s/%d dev %s > /dev/null 2>&1", ip_string,
             prefix_len, dev_name);

    // asprintf(&command, "/sbin/ifconfig %s %s %s/%d -alias", dev_name,
    //          ip->address_family == AF_INET6 ? "inet6" : "", ip_string,
    //          prefix_len);

    verbose_system(command);
    free(command);
}

char *netdev_get_hwaddr(const char *ifname) {
    DIR *dir;
    struct dirent *entry;
    char path[256], buf[256];

    dir = opendir(NETDEV_INFO_PATH);
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Read interface state
        snprintf(path, sizeof(path), NETDEV_INFO_PATH "%s/operstate",
                 entry->d_name);
        FILE *fp = fopen(path, "r");
        if (fp) {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
        } else
            strcpy(buf, "unknown");

        // Read interface speed
        snprintf(path, sizeof(path), NETDEV_INFO_PATH "%s/speed",
                 entry->d_name);
        fp = fopen(path, "r");
        if (fp) {
            fgets(buf + strlen(buf), sizeof(buf) - strlen(buf), fp);
            fclose(fp);
        }

        // Read interface MAC address
        snprintf(path, sizeof(path), NETDEV_INFO_PATH "%s/address",
                 entry->d_name);
        fp = fopen(path, "r");
        if (fp) {
            fgets(buf + strlen(buf), sizeof(buf) - strlen(buf), fp);
            fclose(fp);
        }

        printf("%-10s %-10s %-10s\n", entry->d_name, strtok(buf, "\n"),
               strtok(NULL, "\n"));
    }

    closedir(dir);

    return 0;
}

// void get_hw_address(const char *name, struct ether_addr *hw_address)
// {
// 	struct ifaddrs *ifaddrs_list, *ifaddr;

// 	DEBUGP("get_hw_address for device %s\n", name);

// 	if (getifaddrs(&ifaddrs_list) < 0)
// 		die_perror("getifaddrs");

// 	for (ifaddr = ifaddrs_list; ifaddr != NULL; ifaddr = ifaddr->ifa_next) {
// 		if (strcmp(name, ifaddr->ifa_name) == 0 &&
// 		    ifaddr->ifa_addr->sa_family == AF_LINK) {
// 			struct sockaddr_dl *sdl;
// 			sdl = (struct sockaddr_dl *)ifaddr->ifa_addr;
// 			if (sdl->sdl_type == IFT_ETHER) {
// 				memcpy(hw_address, LLADDR(sdl),
// 				       sizeof(*hw_address));
// 				freeifaddrs(ifaddrs_list);
// 				return;
// 			}
// 		}
// 	}

// 	die("unable to find hw address for %s\n", name);
// }
