#include "base.h"

// #include <syslog.h>

int main(int argc, char* argv[]) {
    /* Parse command line args */
    int opt;
    const char* usage = "Usage: %s [-h] [-d]\n";
    while ((opt = getopt(argc, argv, "hd")) != -1) {
        switch (opt) {
        case 'h':
            printf(usage, argv[0]);
            exit(0);
        case 'd':
            // Parse here
            break;
        default:
            fprintf(stderr, usage, argv[0]);
            exit(1);
        }
    }

    /* Get program name */
    char* cmd;
    if ((cmd = strrchr(argv[0], '/')) == NULL) {
        cmd = argv[0];
    } else {
        cmd++;
    }
    printf("%s\n", cmd);

    /* Make program daemonize */
    daemonize(cmd);

    /* Check if program is running now */
    if (already_running(cmd)) {
        syslog(LOG_ERR, "program was already running!");
    }

    // Program block here
    while (1)
        ;
}
