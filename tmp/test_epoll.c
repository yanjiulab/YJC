#include "list.h"
#include "log.h"
#include "sock.h"
#include "test.h"


typedef struct S {
    int a;
    char b;
    float c;
    struct list_head list;
} S;

void test_epoll() {
    S s = {0};
    INIT_LIST_HEAD(&s.list);

    log_info("%p", &s);
    log_info("%p", &s.b);

    log_info("%d", offsetof(S, b));
}
