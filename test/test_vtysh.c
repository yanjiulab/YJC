
#include "test.h"
#include "vtysh.h"

void test_vtysh() {
    int argc = 1;
    char *argv[] = {"help"};

    cmd_init();

    void *val;
    int i;
    vector_foreach(cmdvec, val, i) {
        struct cmd_element *cmd;
        if (strcmp(argv[0], "help") == 0) {
            /* code */
            ((struct cmd_element *)val)->func(val, argc, argv);
        }
    }
}

