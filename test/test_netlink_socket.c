#include "netlink_socket.h"
#include "test.h"

void test_netlink_socket() {
    netlink_socket_t *nls = netlink_socket_new(NETLINK_ROUTE, "route");

    // netlink_request_route(nls, AF_INET, RTM_GETROUTE);
    // netlink_parse_info(nls, netlink_rtm_parse_route);

    netlink_request_macs(nls, AF_BRIDGE, RTM_GETNEIGH);
    netlink_parse_info(nls, netlink_macfdb_table);

    netlink_socket_free(nls);
}