#include "datetime.h"
#include "ethernet.h"
#include "rt_netlink.h"
#include "test.h"

// test add, delete and get
void test_neigh(nl_socket_t* nls) {
    ipaddr_t peer;
    string_to_ip("10.10.10.1", &peer, NULL);
    ipaddr_t peer6;
    string_to_ip("fe80::226:8b9b:60ee:a010", &peer6, NULL);
    ethaddr_t mac;
    ether_str2mac("01:02:03:04:05:06", &mac);

    // ip neigh add 10.10.10.1 lladdr 01:02:03:04:05:06 dev ens38
    netlink_neigh_update(nls, RTM_NEWNEIGH, peer, mac, "ens38", true, RTPROT_STATIC, 0);
    // ip neigh add fe80::226:8b9b:60ee:a010 lladdr 01:02:03:04:05:06 dev ens38 nud reachable
    netlink_neigh_update(nls, RTM_NEWNEIGH, peer6, mac, "ens38", false, RTPROT_STATIC, 0);
    // ip neigh show nud all
    netlink_neigh_read(nls);
    // ip neigh del 10.10.10.1 lladdr 01:02:03:04:05:06 dev ens38
    netlink_neigh_update(nls, RTM_DELNEIGH, peer, mac, "ens38", true, RTPROT_STATIC, 0);
    // ip neigh del fe80::226:8b9b:60ee:a010 lladdr 01:02:03:04:05:06 dev ens38
    netlink_neigh_update(nls, RTM_DELNEIGH, peer6, mac, "ens38", false, RTPROT_STATIC, 0);
}

// test add, delete and get
void test_route(nl_socket_t* nls) {
    ipaddr_t dst;
    string_to_ip("10.10.10.0", &dst, NULL);
    ipaddr_t gw;
    string_to_ip("192.168.50.11", &gw, NULL);

    // ip route add 10.10.10.0/24 via 192.168.50.11 dev ens38
    // netlink_route_add(nls, &dst, 24, &gw, 3, false);
    // ip route list table all
    netlink_route_read(nls);
    // ip route del 10.10.10.0/24 via 192.168.50.11 dev ens38
    // netlink_route_del(nls, &dst, 24, &gw, 3, false);
}

void test_macfdb(nl_socket_t* nls) {
    // test
    // test notification
    // read macfdb
    // printf("dump `bridge fdb`\n");
    // netlink_macfdb_read(nls);
}

void test_notification() {
    // netlink_route_change()
    // int groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
    int groups = RTMGRP_IPV4_ROUTE | RTMGRP_IPV4_MROUTE;
    nl_socket_t* nls = nl_socket_new(NETLINK_ROUTE, "route", groups);
    fcntl(nls->sock, F_SETFL, fcntl(nls->sock, F_GETFL) & ~O_NONBLOCK);

    netlink_parse_info(nls, netlink_route_change);
    // netlink_neigh_change()
}

void test_rt_netlink() {

    // log_set_level(LOG_INFO);

    // nl_socket_t* nls = nl_socket_new(NETLINK_ROUTE, "route", 0);
    // // test_neigh(nls);
    // test_route(nls);
    // nl_socket_free(nls);

    test_notification();
}