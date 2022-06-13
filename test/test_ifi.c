#include "ifi.h"
#include "test.h"

void test_ifi() {
    struct ifi_info *info;

    // info = get_ifi_info(AF_INET, 1);
    info = get_ifi_by_name("ens33");
    print_ifi_info(info);
    // printf(sock_itop(ntohl(0xC0A81711)));
    // printf("next:%p\n", info->ifi_next);
}