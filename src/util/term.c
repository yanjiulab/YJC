#include "term.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int get_ws_col() {
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    return (int)ws.ws_col;
}

int get_ws_row() {
    struct winsize ws;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
    return (int)ws.ws_row;
}

void print_line(char c, int len) {

    if (!len) {
        len = get_ws_col();
    }

    if (!isprint(c)) {
        c = '-';
    }

    for (int i = 0; i < len; i++) {
        putchar(c);
    }

    putchar('\n');
}

void print_title(char* title, char c) {

    int l = strlen(title);
    int m = get_ws_col();
    int cl = (m - l - 2) / 2;

    if (!isprint(c)) {
        c = '=';
    }

    if (l >= m || cl <= 0) {
        putchar(c);
        putchar(' ');
        fputs(title, stdout);
        putchar(' ');
        putchar(c);
        putchar('\n');
        return;
    }

    for (int i = 0; i < cl; i++)
        putchar(c);
    putchar(' ');
    fputs(title, stdout);
    putchar(' ');
    for (int i = 0; i < cl; i++)
        putchar(c);
    putchar('\n');
}