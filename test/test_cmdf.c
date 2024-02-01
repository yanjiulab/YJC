/*
 * test.c - A test program for the libcmdf library
 * Public domain; no warrenty applied, use at your own risk!
 * Authored by Ronen Lapushner, 2017.
 *
 * License:
 * --------
 * This software is dual-licensed to the public domain and under the following license:
 * you are granted a perpetual, irrevocable license to copy, modify,
 * publish and distribute this file as you see fit.
 */

#define _CRT_SECURE_NO_WARNINGS
#define LIBCMDF_IMPL
#define CMDF_READLINE_SUPPORT

#include "cmdf.h"
#include "test.h"
#include <stdio.h>

#define PROG_INTRO "test - A simple test program for libcmdf.\n" \
                   "You can use this as a reference on how to use the library!"
#define SUBMENU_INTRO  "This is a submenu!"
#define PRINTARGS_HELP "This is a very long help string for a command.\n" \
                       "As you can see, this is concatenated properly. It's pretty good!"

static CMDF_RETURN do_hello(cmdf_arglist* arglist) {
    printf("\nHello, world!\n");

    return CMDF_OK;
}

static CMDF_RETURN do_printargs(cmdf_arglist* arglist) {
    int i;

    if (!arglist) {
        printf("\nNo arguments provided!\n");
        return CMDF_OK;
    }

    printf("\nTotal arguments = %lu", arglist->count);
    for (i = 0; i < arglist->count; i++)
        printf("\nArgument %d: \'%s\'", i, arglist->args[i]);

    printf("\n");

    return CMDF_OK;
}

static CMDF_RETURN do_submenu(cmdf_arglist* arglist) {
    cmdf_init("libcmdf-test/submenu> ", SUBMENU_INTRO, NULL, NULL, 0, 1, 0, 0);

    /* Register our custom commands */
    cmdf_register_command(do_hello, "sub-hello1", NULL);
    cmdf_register_command(do_hello, "sub-hello2", NULL);
    cmdf_register_command(do_hello, "sub-hello3", NULL);
    cmdf_register_command(do_hello, "sub-hello4", NULL);
    cmdf_register_command(do_hello, "sub-hello5", NULL);
    cmdf_register_command(do_hello, "sub-hello6", NULL);
    cmdf_register_command(do_hello, "sub-hello7", NULL);
    cmdf_register_command(do_hello, "sub-hello8", NULL);
    cmdf_register_command(do_hello, "sub-hello9", NULL);
    cmdf_register_command(do_hello, "sub-hello10", NULL);
    cmdf_register_command(do_hello, "sub-hello11", NULL);
    cmdf_register_command(do_hello, "sub-hello12", NULL);
    cmdf_register_command(do_hello, "sub-hello13", NULL);
    cmdf_register_command(do_hello, "sub-hello14", NULL);
    cmdf_register_command(do_hello, "sub-hello15", NULL);
    cmdf_register_command(do_hello, "sub-hello16", NULL);
    cmdf_register_command(do_hello, "sub-hello17", NULL);
    cmdf_register_command(do_hello, "sub-hello18", NULL);
    cmdf_register_command(do_hello, "sub-hello19", NULL);
    cmdf_register_command(do_hello, "sub-hello20", NULL);
    cmdf_register_command(do_hello, "sub-hello21", NULL);
    cmdf_register_command(do_hello, "sub-hello22", NULL);
    cmdf_register_command(do_hello, "sub-hello23", NULL);
    cmdf_register_command(do_hello, "sub-hello24", NULL);
    cmdf_register_command(do_hello, "sub-hello25", NULL);
    cmdf_register_command(do_hello, "sub-hello26", NULL);
    cmdf_register_command(do_hello, "sub-hello27", NULL);
    cmdf_register_command(do_hello, "sub-hello28", NULL);
    cmdf_register_command(do_hello, "sub-hello29", NULL);
    cmdf_register_command(do_hello, "sub-hello30", NULL);

    cmdf_commandloop();

    return CMDF_OK;
}

static CMDF_RETURN do_submenu1(cmdf_arglist* arglist) {
    cmdf_init("libcmdf-test/submenu1> ", SUBMENU_INTRO, NULL, NULL, 0, 1, 0, 0);

    /* Register our custom commands */
    cmdf_register_command(do_hello, "sub-hello1", NULL);
    cmdf_register_command(do_hello, "sub-hello2", NULL);
    cmdf_register_command(do_hello, "sub-hello3", NULL);
    cmdf_register_command(do_hello, "sub-hello4", NULL);
    cmdf_register_command(do_hello, "sub-hello5", NULL);
    cmdf_register_command(do_hello, "sub-hello6", NULL);
    cmdf_register_command(do_hello, "sub-hello7", NULL);
    cmdf_register_command(do_hello, "sub-hello8", NULL);
    cmdf_register_command(do_hello, "sub-hello9", NULL);
    cmdf_register_command(do_hello, "sub-hello10", NULL);

    cmdf_commandloop();

    return CMDF_OK;
}

void test_cmdf(void) {
    cmdf_init("libcmdf-test> ", PROG_INTRO, NULL, NULL, 0, 1, 0, 0);

    /* Register our custom commands */

    cmdf_register_command(do_submenu, "submenu", NULL);
    cmdf_register_command(do_submenu1, "submenu1", NULL);
    cmdf_register_command(do_hello, "hello1", NULL);
    cmdf_register_command(do_hello, "hello2", NULL);
    cmdf_register_command(do_hello, "hello3", NULL);
    cmdf_register_command(do_hello, "hello4", NULL);
    cmdf_register_command(do_hello, "hello5", NULL);
    cmdf_register_command(do_hello, "hello6", NULL);
    cmdf_register_command(do_hello, "hello7", NULL);
    cmdf_register_command(do_hello, "hello8", NULL);
    cmdf_register_command(do_hello, "hello9", NULL);
    cmdf_register_command(do_hello, "hello10", NULL);
    cmdf_register_command(do_hello, "hello11", NULL);
    cmdf_register_command(do_hello, "hello12", NULL);
    cmdf_register_command(do_hello, "hello13", NULL);
    cmdf_register_command(do_hello, "hello14", NULL);
    cmdf_register_command(do_hello, "hello15", NULL);
    cmdf_register_command(do_hello, "hello16", NULL);
    cmdf_register_command(do_hello, "hello17", NULL);
    cmdf_register_command(do_hello, "hello18", NULL);
    cmdf_register_command(do_hello, "hello19", NULL);
    cmdf_register_command(do_hello, "hello20", NULL);
    cmdf_commandloop();
}
