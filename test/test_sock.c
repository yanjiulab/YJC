#include <stdio.h>

#include "sock.h"
#include "test.h"

void test_sock() {
    int ser = tcp_listen(NULL, "9876", NULL);
    printf("%d\n", ser);

    int cli = tcp_connect(NULL, "9876");
    cli = udp_connect(NULL, "1111");
}