
#include "cmd.h"

int do_echo(cmd_ctx_t* ctx, cmd_arglist_t* args) {

    if (!args) {
        puts("null args");
        return CMD_OK;
    }

    for (int i = 0; i < args->count; i++) {
        printf("%s ", args->args[i]);
    }
    puts("");

    return CMD_OK;
};

int main(int argc, char* argv[]) {
    log_info("hello cmd");

    cmd_ctx_t* ctx = cmd_ctx_new(0);
    cmd_register_command(ctx, do_echo, "echo", "echo the cmd");
    cmd_commandloop(ctx);

    // printf("echo: '%s'\n", line);
    // printf("cmd: '%s'\n", cmdptr);
    // printf("args: '%s'\n", argsptr);

    return 0;
}