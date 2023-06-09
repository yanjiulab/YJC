#include "vtysh.h"

#include "vector.h"

vector cmdvec = NULL;

DEFUN(help, help_cmd, "help XXX", "Print help messages.") {
    //
    printf("syntax: %s\n", self->string);
    printf("help: %s\n", self->doc);
}

void install_element(struct cmd_element *cmd) {
    /* cmd_init hasn't been called */
    if (!cmdvec) {
        fprintf(stderr, "%s called before cmd_init, breakage likely\n",
                __func__);
        return;
    }

    vector_set(cmdvec, (void *)cmd);

}

void cmd_init() {
    /* Allocate initial top vector of commands. */
    cmdvec = vector_init(VECTOR_MIN_SIZE);

    /* Install top nodes. */
    // TODO

    /* Each node's basic commands. */
    install_element(&help_cmd);
}

void shell() {
    
}