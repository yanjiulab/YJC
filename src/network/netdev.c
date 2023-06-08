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
