// https://gohalo.me/post/linux-libev.html
#include <ev.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ini.h"
#include "ipa.h"
#include "sock.h"
#include "str.h"
#include "timer.h"

// all watcher callbacks have a similar signature
// this callback is called when data is readable on stdin
static void stdin_cb(EV_P_ ev_io *w, int revents) {
    char *line = NULL;
    size_t size;
    int ret = getline(&line, &size, stdin);
    printf("> %s", line);
    // for one-shot events, one must manually stop the watcher
    // with its corresponding stop function.
    // ev_io_stop(EV_A_ w);

    // this causes all nested ev_run's to stop iterating
    // ev_break(EV_A_ EVBREAK_ALL);
}

// another callback, this time for a time-out
static void timeout_cb(EV_P_ ev_timer *w, int revents) {
    static int retry = 0;
    printf("timeout: ");
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    printf("%d:%d\n", curtime.tv_sec, curtime.tv_usec);

    // this causes the innermost ev_run to stop iterating
    // if (++retry == 3) ev_break(EV_A_ EVBREAK_ONE);
    if (++retry == 3) ev_timer_stop(loop, w);
}

static void sigint_cb(struct ev_loop *loop, ev_signal *w, int revents) {
    printf("recv sigint, clean and exit ....\n");
    ev_break(loop, EVBREAK_ALL);
}

static void confile_cb(struct ev_loop *loop, ev_stat *w, int revents) {
    /* /etc/passwd changed in some way */
    if (w->attr.st_nlink) {
        printf("passwd current size  %ld\n", (long)w->attr.st_size);
        printf("passwd current atime %ld\n", (long)w->attr.st_mtime);
        printf("passwd current mtime %ld\n", (long)w->attr.st_mtime);

        ini_t *config = ini_load(w->path);
        const char *name = ini_get(config, "owner", "name");
        if (name) {
            printf("name: %s\n", name);
        }
        ini_free(config);
    } else
        /* you shalt not abuse printf for puts */
        puts(
            "wow, config is not there, expect problems. "
            "if this is windows, they already arrived\n");
}

static void idle_cb(struct ev_loop *loop, ev_idle *w, int revents) {
    // stop the watcher
    ev_idle_stop(loop, w);

    // now we can free it
    free(w);

    // now do something you wanted to do when the program has
    // no longer anything immediate to do.
}

int main(int argc, char *argv[]) {
    printf("Hello YJC\n");

    for (int i = 0; i < argc; i++) {
        printf("args(%d/%d): %s\n", i, argc, argv[i]);
    }

    //  every watcher type has its own typedef'd struct
    //  with the name ev_TYPE

    // use the default event loop unless you have special needs
    struct ev_loop *loop = EV_DEFAULT;

    // initialise an io watcher, then start it
    // this one will watch for stdin to become readable
    ev_io stdin_watcher;
    ev_io_init(&stdin_watcher, stdin_cb, /*STDIN_FILENO*/ 0, EV_READ);
    ev_io_start(loop, &stdin_watcher);

    // initialise a timer watcher, then start it simple non-repeating 5.5 second timeout, and repeating 3 second
    ev_timer timeout_watcher;
    ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 3.);
    ev_timer_start(loop, &timeout_watcher);

    // Try to exit cleanly on SIGINT.
    ev_signal signal_watcher;
    ev_signal_init(&signal_watcher, sigint_cb, SIGINT);
    ev_signal_start(loop, &signal_watcher);

    // config file
    ev_stat passwd;
    ev_stat_init(&passwd, confile_cb, "/home/liyj/code/YJC/config.ini", 0.);
    ev_stat_start(loop, &passwd);

    // idle
    ev_idle *idle_watcher = malloc(sizeof(ev_idle));
    ev_idle_init(idle_watcher, idle_cb);
    ev_idle_start(loop, idle_watcher);

    // now wait for events to arrive
    ev_run(loop, 0);

    // break was called, so exit
    return 0;
}
