#include <stdio.h>

#include "crc.h"
#include "inet.h"
#include "log.h"
#include "socket.h"
#include "test.h"

void test_sock() {
    assert(is_ipv4("192.168.1.1") == true);
    assert(is_ipv4("1.168.1.1") == true);
    assert(is_ipv4("255.300.1.1") == false);
    assert(is_ipv6("::1") == true);
    assert(is_ipv6("fe80::f99e:e589:b358:ab89") == true);
    assert(is_ipv6(":1:1") == false);

    sockaddr_u addr = {0};
    sockaddr_set_ip(&addr, "10.0.0.1");
    sockaddr_set_port(&addr, 8877);
    sockaddr_print(&addr);

    sockaddr_set_ipport(&addr, "fe80::f99e:e589:b358:ab89", 8989);
    sockaddr_print(&addr);

    char buf[SOCKADDR_STRLEN];
    int len = sizeof(buf);
    printf("len: %d\n", sockaddr_len(&addr));
    printf("str: %s\n", sockaddr_str(&addr, buf, len));
    printf("ip: %s\n", sockaddr_ip(&addr, buf, len));
    printf("port: %d\n", sockaddr_port(&addr));

    int udp_fd = Bind(8989, "localhost", SOCK_DGRAM);
    int tcp_fd = Listen(8990, ANYADDR);

    printf("udp server: %d\ntcp server: %d\n", udp_fd, tcp_fd);
}

void test_inet() {
    char data[2] = {0x00, 0x30};
    // short r = crc16_ccitt(data, 2);
    short r = crc16_cus(data, 2);
    log_info("%x", r);
}