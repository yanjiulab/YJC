#include "netlink_socket.h"
#include "test.h"

void test_netlink_socket() {
    netlink_socket_t *nls = netlink_socket_new(NETLINK_ROUTE, "route");

    netlink_rt_get_req(nls);
    netlink_recvmsg(nls);

    netlink_socket_free(nls);
}