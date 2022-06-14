#include "test.h"
#include "name.h"


void test_name() {
    struct addrinfo *res;
    // res = host_serv("baidu.com", "ftp", AF_INET, 0);
     res = host_serv("127.0.0.1", NULL, AF_INET6, 0);
    print_addrinfo(res);
    freeaddrinfo(res);
}