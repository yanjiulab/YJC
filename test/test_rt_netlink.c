#include "datetime.h"
#include "rt_netlink.h"
#include "test.h"

void test_neigh(nl_socket_t* nls) {
    // test ipv4
    int ifindex = 3;
    ip_address_t peer;
    string_to_ip("10.10.10.0", &peer, NULL);
    char* lla = "123456";
    int llalen = 6;
    netlink_neigh_update(nls, RTM_NEWNEIGH, ifindex, &(peer.ip.v4), lla, llalen, 0, peer.address_family, true, RTPROT_STATIC);
    netlink_neigh_read(nls);
    netlink_neigh_update(nls, RTM_DELNEIGH, ifindex, &(peer.ip.v4), lla, llalen, 0, peer.address_family, true, RTPROT_STATIC);

    // test ipv6
    ip_address_t peer6;
    string_to_ip("fe80::226:8b9b:60ee:a010", &peer6, NULL);
    lla = "654321";
    netlink_neigh_update(nls, RTM_NEWNEIGH, ifindex, &(peer6.ip.v6), lla, llalen, 0, peer6.address_family, true, RTPROT_STATIC);
    netlink_neigh_read(nls);
    netlink_neigh_update(nls, RTM_DELNEIGH, ifindex, &(peer6.ip.v6), lla, llalen, 0, peer6.address_family, true, RTPROT_STATIC);

    // test notification
    // TODO
}

void test_route(nl_socket_t* nls) {
    // test ipv4
    // test ipv6
    // test notification
    // read ipv4 route table
    // printf("dump `ip route list table all`\n");
    // netlink_route_read(nls);

    // printf("dump `ip neigh show nud all`\n");
    // netlink_neigh_read(nls);

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
}

void test_macfdb(nl_socket_t* nls) {
    // test
    // test notification
    // read macfdb
    // printf("dump `bridge fdb`\n");
    // netlink_macfdb_read(nls);
}

void test_rt_netlink() {
    // log_set_level(LOG_INFO);

    nl_socket_t* nls = nl_socket_new(NETLINK_ROUTE, "route", 0);

    test_neigh(nls);

    nl_socket_free(nls);
}