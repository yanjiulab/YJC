#include "array.h"
#include "queue.h"
#include "test.h"

// 定义动态数组
ARRAY_DECL(int, ip_array);
#define INIT_SIZE 16

void test_array() {
    ip_array ips;

    ip_array_init(&ips, INIT_SIZE);

    // for (int i = 0; i < 20; i++)
    // {
    //     // ip_array_back
    // }

    int i = 1;
    ip_array_push_back(&ips, &i);
    ip_array_push_back(&ips, &i);
    ip_array_push_back(&ips, &i);
    ip_array_push_back(&ips, &i);

    for (int i = 0; i < ip_array_size(&ips); i++) {
        printf("%d ", *ip_array_at(&ips, i));
    }
    printf("\n");
}

// void test_queue() {}
