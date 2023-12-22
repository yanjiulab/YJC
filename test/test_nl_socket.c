#include "datetime.h"
#include "ip_address.h"
#include "log.h"
#include "nl_kernel.h"
#include "rt_netlink.h"
#include "test.h"

void test_nl_socket() {
    log_set_level(LOG_INFO);

    nl_socket_t* nls = nl_socket_new(NETLINK_ROUTE, "route", 0);

    // read ipv4 route table
    printf("dump `ip route list table all`\n");
    netlink_route_read(nls);

    // read macfdb
    printf("dump `bridge fdb`\n");
    netlink_macfdb_read(nls);

    printf("dump `ip neigh show nud all`\n");
    netlink_neigh_read(nls);

    // netlink_parse_info(nls, NULL);
    // netlink_parse_info(nls, NULL);
    // netlink_request_route(nls, AF_INET6, RTM_GETROUTE);
    // netlink_parse_info(nls, NULL);

    // ip_address_t dst;
    // string_to_ip("10.10.10.0", &dst, NULL);
    // ip_address_t gw;
    // string_to_ip("12.2.2.2", &gw, NULL);
    // print_data(&dst, sizeof(dst));

    // netlink_request_route_add(nls, RTM_NEWROUTE, &dst, &gw, 0, 2);
    // netlink_request_route(nls, AF_INET, RTM_GETROUTE);
    // netlink_parse_info(nls, netlink_rtm_parse_route);

    nl_socket_free(nls);
}