#include "defs.h"
#include "heap.h"
#include "test.h"

typedef struct timer {
    int time;
    struct heap_node node;
} timer;

#define TIMER_ENTRY(p) container_of(p, timer, node)

static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->time < TIMER_ENTRY(rhs)->time;
}

void test_heap() {
    struct heap timers;
    heap_init(&timers, timers_compare);

    timer t[10];
    for (int i = 0; i < 10; i++) {
        t[i].time = 10 - i;
        printf("%d\n", t[i].time);
        heap_insert(&timers, &t[i].node);
    }
timer = TIMER_ENTRY(timers->root);
    printf("%d ", TIMER_ENTRY(timers.root)->time);
    heap_dequeue(&timers);

    // printf("%d ", TIMER_ENTRY(timers.root)->time);
    // heap_dequeue(&timers.root);

    // printf("%d", timers.root);
}