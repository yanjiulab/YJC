#include "test.h"
#include "vtysh.h"
#include "base.h"

void test_shell() {
    shell();
    printd("df\n");
    fprintf(stderr, "dsf\n");


    char *ptr = MALLOC(10);
    
}