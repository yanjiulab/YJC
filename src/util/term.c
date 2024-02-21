#include "term.h"

#include <unistd.h>

void print_line(char c, int len) {

    if (!len) {
        struct winsize ws;
        ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
        len = ws.ws_col;
    }

    for (int i = 0; i < len; i++) {
        putchar(c);
    }

    putchar('\n');
}