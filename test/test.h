#ifndef TEST_H
#define TEST_H
#include <assert.h>
// util
void test_log();
void test_ini();
void test_str_split();
void test_str_trim();
void test_timer();
void test_json();

// container
void test_vector();
void test_map();
void test_linklist();

// netbase
void test_ifi();
void test_name();
void test_sock();

// cliser

// event
int test_ev();

// shell
void test_shell();

// my ev
void test_ev_my();

// epoll
void test_epoll();

void test_rbtree();
void test_list();

void test_math();
#endif  // !TEST_H