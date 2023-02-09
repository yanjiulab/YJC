#include "defs.h"
#include "heap.h"
#include "test.h"

// 定义堆节点
typedef struct timer_s {
    int sec;
    struct heap_node node;
} timer;

// 取出堆节点
#define TIMER_ENTRY(p) container_of(p, timer, node)

// 定义比较函数
// if compare is less_than, root is min of heap
// if compare is larger_than, root is max of heap
static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->sec < TIMER_ENTRY(rhs)->sec;
}

void test_heap() {
    // 定义并初始化堆
    struct heap timers;
    heap_init(&timers, timers_compare);

    // 向堆中添加节点
    timer* t;
    for (int i = 0; i < 10; i++) {
        t = calloc(1, sizeof(timer));
        t->sec = rand() % 10;
        printf("%d ", t->sec);
        heap_insert(&timers, &t->node);
    }
    printf("\n");

    // 依次取出根节点
    while (timers.root) {
        printf("%d ", TIMER_ENTRY(timers.root)->sec);
        heap_dequeue(&timers);
    }
    printf("\n");
}