#include "test.h"
#include "timer.h"

void work() { printf("Its my time to work!\n"); }

void test_timer() {
    printf("===== %s start =====\n", __func__);
    timer_set(10, work, 0);
    timer_set(3, work, 0);
    timer_set(5, work, 0);
    timer_set(9, work, 0);

    timer_print();

    timer_left(4);
    printf("timer_left(4): %d\n", timer_left(4));
    timer_elapse(5);
    timer_print();
    printf("next event will happen after %d seconds\n", timer_next());
    timer_clear(4);
    timer_print();
    timer_free();
    printf("===== %s end =====\n", __func__);
}