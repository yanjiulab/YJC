#include <stdio.h>

#include "cmdf.h"
#include "log.h"
#include "sev.h"
#include "test.h"
#include "thread.h"

#define LIBCMDF_IMPL
#define CMDF_READLINE_SUPPORT

#define PROG_INTRO "test - A simple test program for libcmdf.\n" \
                   "You can use this as a reference on how to use the library!"

static CMDF_RETURN do_hello(cmdf_arglist* arglist) {
    printf("\nHello, world!\n");

    return CMDF_OK;
}

static CMDF_RETURN do_quiet(cmdf_arglist* arglist) {
    printf("Enter quiet mode!\n");
    log_set_quiet(1);

    return CMDF_OK;
}

static CMDF_RETURN do_noisy(cmdf_arglist* arglist) {
    printf("Exit quiet mode!\n");
    log_set_quiet(0);

    return CMDF_OK;
}

static EV_RETURN period_hello(evtimer_t* timer, void* arg) {
    log_info("Hello %s", (char*)arg);
    evtimer_add(event_loop(timer), period_hello, "World", 1000);

    return EV_OK;
}

void test_sev() {
    // For stdin command loop
    cmdf_init("libcmdf-test> ", PROG_INTRO, NULL, NULL, 0, 1, 0, 0);
    cmdf_register_command(do_hello, "hello", NULL);
    cmdf_register_command(do_quiet, "quiet", NULL);
    cmdf_register_command(do_noisy, "noisy", NULL);
    thread_create(cmdf_commandloop, NULL);

    evloop_t* loop = evloop_new(10);
    evtimer_add(loop, period_hello, "World", 1000);
    evloop_run(loop);
}
