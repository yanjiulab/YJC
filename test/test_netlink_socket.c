#include "datetime.h"
#include "ip_address.h"
#include "log.h"
#include "nl_socket.h"
#include "rt_netlink.h"
#include "test.h"

static const struct message rip_msg[] = {{2, "REQUEST"},
                                         {3, "RESPONSE"},
                                         {4, "TRACEON"},
                                         {5, "TRACEOFF"},
                                         {6, "POLL"},
                                         {7, "POLL ENTRY"},
                                         {0}};

void test_netlink_socket() {
    // log_set_level(LOG_DEBUG);
    nl_socket_t *nls = nl_socket_new(NETLINK_ROUTE, "route");
    
    // netlink_request_route(nls, AF_INET, RTM_GETROUTE);
    // netlink_parse_info(nls, netlink_rtm_parse_route);

    ip_address_t dst;
    string_to_ip("10.10.10.0", &dst, NULL);
    ip_address_t gw;
    string_to_ip("12.2.2.2", &gw, NULL);
    // print_data(&dst, sizeof(dst));

    netlink_request_route_add(nls, RTM_NEWROUTE, &dst, &gw, 0, 2);
    // netlink_request_route(nls, AF_INET, RTM_GETROUTE);
    netlink_parse_info(nls, netlink_rtm_parse_route);
    // netlink_request_macs(nls, AF_INET6, RTM_GETNEIGH);
    // netlink_parse_info(nls, netlink_macfdb_table);

    nl_socket_free(nls);
}