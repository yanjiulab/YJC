#ifndef TIMER_H
#define TIMER_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef void (*cfunc_t)(unsigned int);

void timer_elapse(int elapsed_time);
int timer_next();
int timer_set(int delay, cfunc_t action, unsigned int data);
int timer_left(int timer_id);
void timer_clear(int timer_id);
void timer_print();
void timer_free();

#endif