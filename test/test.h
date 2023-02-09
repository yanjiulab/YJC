#ifndef TEST_H
#define TEST_H
#include <assert.h>
// base
void test_base();

// event

// net

// util
void test_log();
void test_ini();
void test_base64_md5_sha1();

<<<<<<< HEAD
// container
void test_list();
void test_map();
void test_heap();
void test_rbtree();
void test_container();
void test_vector();
void test_linklist();

=======
void test_str();
>>>>>>> a3c76dbe8816fb4a782e468d091531eba3c13ec1
void test_str_split();
void test_str_trim();
void test_timer();
void test_json();

<<<<<<< HEAD
=======
// container
void test_vector();
void test_map();
void test_linklist();
void test_heap();
void test_array();
void test_queue();

>>>>>>> a3c76dbe8816fb4a782e468d091531eba3c13ec1
// netbase
void test_ifi();
void test_name();
void test_sock();
void test_inet();
// cliser

// event
int test_ev();

// shell
void test_shell();

// my ev
void test_ev_my();

// epoll
void test_epoll();

void test_math();

#endif  // !TEST_H