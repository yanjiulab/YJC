#include "base.h"
#include "test.h"

void test_base() {
    char* p;
    MEMCHECK;
    EV_ALLOC(p, 100);
    strcpy(p, "hello world");
    EV_FREE(p);

    int i = 0;
    char* ptr[10] = {0};
    int size = 16, nsize = 32, osize = 16;
}