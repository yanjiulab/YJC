#include "base.h"
#include "test.h"

void test_base() {

    EV_MEMCHECK
    char *ptr;
    EV_ALLOC(ptr, 100);
    strcpy(ptr, "hello world");
    EV_FREE(ptr);
}