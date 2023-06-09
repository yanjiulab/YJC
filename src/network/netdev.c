#include "netdev.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* netdev_get_hwaddr(const char* ifname) {
    DIR* dir;
    struct dirent* entry;
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
        FILE* fp = fopen(path, "r");
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
