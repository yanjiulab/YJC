#include "base.h"
#include "iniparser.h"
#include "log.h"
#include "sev.h"
#include "thread.h"

#define LIBCMDF_IMPL
#define CMDF_READLINE_SUPPORT
#include "cmdf.h"
#define PROG_INTRO "A simple program for network programming.\n" \
                   "You can use this as a reference on how to use the library!"

evloop_t* loop = NULL;

static void sig_int() {
    cmdf__default_do_exit(NULL);
    evloop_stop(loop);
}

static CMDF_RETURN do_hello(cmdf_arglist* arglist) {
    printf("\nHello, world!\n");

    return CMDF_OK;
}

static CMDF_RETURN do_setlog(cmdf_arglist* arglist) {
    int level = 1;

    if (arglist && arglist->count) {
        level = str2int(arglist->args[0]) % 6;
    }
    log_set_level(level);

    return CMDF_OK;
}

static CMDF_RETURN do_quit(cmdf_arglist* arglist) {
    sig_int();

    return CMDF_OK;
}

static EV_RETURN on_period_hello(evtimer_t* timer, void* arg) {
    log_info("Hello %s", (char*)arg);
    evtimer_add(event_loop(timer), on_period_hello, "World", 1000);

    return EV_OK;
}

int main(int argc, char* argv[]) {

    /* Parse command line args */
    int opt, isdaemon = 0;
    const char* usage = "Usage: %s [-h] [-d]\n";
    while ((opt = getopt(argc, argv, "hd")) != -1) {
        switch (opt) {
        case 'h':
            printf(usage, argv[0]);
            exit(0);
        case 'd':
            isdaemon = 1;
            break;
        default:
            fprintf(stderr, usage, argv[0]);
            exit(1);
        }
    }

    /* Parse ini config file. */
    dictionary* ini = iniparser_load("config.ini");
    iniparser_dump(ini, stderr);
    // int i = iniparser_getint(ini, "sec:intkey", 0);
    // char *s = iniparser_getstring(ini, "sec:strkey", 0);
    // bool b = iniparser_getboolean(ini, "sec:boolkey", false);
    iniparser_freedict(ini);

    /* Get program name. */
    char* cmd;
    if ((cmd = strrchr(argv[0], '/')) == NULL) {
        cmd = argv[0];
    } else {
        cmd++;
    }

    /* Program running in daemon mode. */
    if (isdaemon) {
        log_info("%s running in daemon mode!", cmd);
        daemonize(cmd);
    }

    /* Check if program is already running. */
    if (already_running(cmd)) {
        if (isdaemon)
            syslog(LOG_ERR, "%s was already running!", cmd);
        else
            log_error("%s was already running!", cmd);
        exit(EXIT_FAILURE);
    }

    /* Setting signals and clean */
    if (signal(SIGINT, sig_int) == SIG_ERR) {
        exit(EXIT_FAILURE);
    }

    /* Start commandline loop for stdin */
    thread_t cmd_th;
    if (!isdaemon) {
        cmdf_init(str("%s> ", cmd), PROG_INTRO, NULL, NULL, 0, 0);
        cmdf_register_command(do_quit, "quit", "Quit the application");
        cmdf_register_command(do_setlog, "log", "Set log debug level");
        cmd_th = thread_create(cmdf_commandloop, NULL);
    }

    /* Main loop */
    loop = evloop_new(10);
    evtimer_add(loop, on_period_hello, "World", 1000);
    evloop_run(loop);

    /* Clean and exit */
    printf("\nClean and exit ...\n");
    printf("\nBye.\n");

    return 0;
}
