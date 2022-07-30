#include "test.h"
#include "name.h"


void test_name() {
    struct addrinfo *res;
    // res = host_serv("baidu.com", "ftp", AF_INET, 0);
    res = host_serv("192.168.244.128", "8000", AF_INET, SOCK_STREAM);
    print_addrinfo(res);
    freeaddrinfo(res);
}