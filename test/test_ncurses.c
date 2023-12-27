#include <form.h>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>

#include "test.h"

void basic() {
    initscr();                 /* Start curses mode */
    printw("Hello World !!!"); /* Print Hello World */
    refresh();                 /* Print it on to the real screen */
    getch();                   /* Wait for user input */
    endwin();                  /* End curses mode */
}

void init() {
    int ch;
    initscr();            /* Start curses mode */
    raw();                /* Line buffering disabled */
    keypad(stdscr, TRUE); /* We get F1, F2 etc.. */
    noecho();             /* Don't echo() while we do getch */
    printw("Type any character to see it in bold\n");
    ch = getch();                 /* If raw() hadn't been called
                                   * we have to press enter before it
                                   * gets to the program */
    if (ch == KEY_F(1))           /* Without keypad enabled this will */
        printw("F1 Key pressed"); /* not get to us either */
    /* Without noecho() some ugly escape
     * charachters might have been printed
     * on screen */
    else {
        printw("The pressed key is ");
        addch('c' | A_BOLD | A_UNDERLINE);
        attron(A_BOLD);
        printw("%c", ch);
        attroff(A_BOLD);
    }
    refresh(); /* Print it on to the real screen */
    getch();   /* Wait for user input */
    endwin();  /* End curses mode */
}

void wp() {
    char* string = "hello";
    int y = 10, x = 10;
    WINDOW* win = stdscr;
    printw(string);         /* Print on stdscr at present cursor position */
    mvprintw(y, x, string); /* Move to (y, x) then print string */
    wprintw(win, string);   /* Print on window win at present cursor position */
    /* in the window */
    mvwprintw(win, y, x, string); /* Move to (y, x) relative to window */
    /* co-ordinates and then print */

    // 1. addch() class: Print single character with attributes
    // 2. printw() class: Print formatted output similar to printf()
    // 3. addstr() class: Print strings
}

void pw() {
    char mesg[] = "Just a string"; /* message to be appeared on the screen */
    int row, col;                  /* to store the number of rows and *
                                    * the number of colums of the screen */
    initscr();                     /* start the curses mode */
    getmaxyx(stdscr, row, col);    /* get the number of rows and columns */
    mvprintw(row / 2, (col - strlen(mesg)) / 2, "%s", mesg);
    /* print the message at the center of the screen */
    mvprintw(row - 2, 0, "This screen has %d rows and %d columns\n", row, col);
    printw("Try resizing your window(if possible) and then run this program again");
    refresh();
    getch();
    mvprintw(row - 2, 0, "aaaaaaaaaaaaaaaaaaaa\n");
    refresh();
    getch();
    endwin();
}

void sw() {
    char mesg[] = "Enter a string: "; /* message to be appeared on the screen */
    char str[80];
    int row, col;               /* to store the number of rows and *
                                 * the number of colums of the screen */
    initscr();                  /* start the curses mode */
    getmaxyx(stdscr, row, col); /* get the number of rows and columns */
    mvprintw(row / 2, (col - strlen(mesg)) / 2, "%s", mesg);
    /* print the message at the center of the screen */
    getstr(str);
    mvprintw(LINES - 2, 0, "You Entered: %s", str);
    getch();
    endwin();
}

WINDOW* create_newwin(int height, int width, int starty, int startx) {
    WINDOW* local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0, 0); /* 0, 0 gives default characters
                           * for the vertical and horizontal
                           * lines */
    // wborder(local_win, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(local_win); /* Show that box */
    return local_win;
}

void destroy_win(WINDOW* local_win) {
    /* box(local_win, ' ', ' '); : This won't produce the desired
     * result of erasing the window. It will leave it's four corners
     * and so an ugly remnant of window.
     */
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    // wborder(local_win, '|', '|', '-', '-', '+', '+', '+', '+');
    /* The parameters taken are
     * 1. win: the window on which to operate
     * 2. ls: character to be used for the left side of the window
     * 3. rs: character to be used for the right side of the window
     * 4. ts: character to be used for the top side of the window
     * 5. bs: character to be used for the bottom side of the window
     * 6. tl: character to be used for the top left corner of the window
     * 7. tr: character to be used for the top right corner of the window
     * 8. bl: character to be used for the bottom left corner of the window
     * 9. br: character to be used for the bottom right corner of the window
     */
    wrefresh(local_win);
    delwin(local_win);
}

void window() {
    WINDOW* my_win;
    int startx, starty, width, height;
    int ch;
    initscr();            /* Start curses mode */
    cbreak();             /* Line buffering disabled, Pass on
                           * everty thing to me */
    keypad(stdscr, TRUE); /* I need that nifty F1 */
    height = 3;
    width = 10;
    starty = (LINES - height) / 2; /* Calculating for a center placement */
    startx = (COLS - width) / 2;   /* of the window */
    printw("Press F1 to exit");
    refresh();
    my_win = create_newwin(height, width, starty, startx);
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_LEFT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, --startx);
                break;
            case KEY_RIGHT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, ++startx);
                break;
            case KEY_UP:
                destroy_win(my_win);
                my_win = create_newwin(height, width, --starty, startx);
                break;
            case KEY_DOWN:
                destroy_win(my_win);
                my_win = create_newwin(height, width, ++starty, startx);
                break;
        }
    }
    endwin(); /* End curses mode */
    return 0;
}

void gui() {
    WINDOW *text, *fb, *cmd;
    initscr(); /* Start curses mode */
    noecho();
    cbreak();             /* Line buffering disabled, Pass on
                           * everty thing to me */
    keypad(stdscr, TRUE); /* I need that nifty F1 */
    int row, col;
    getmaxyx(stdscr, row, col); /* get the number of rows and columns */

    // printw("Press F1 to exit");
    mousemask(ALL_MOUSE_EVENTS, NULL);
    refresh();
    row--;
    int fb_h = row, fb_w = col * 0.1;
    int tx_h = row * 0.8, tx_w = col - fb_w;
    int cmd_h = row - tx_h, cmd_w = col - fb_w;
    int x, y;
    fb = create_newwin(fb_h, fb_w, 0, 0);
    text = create_newwin(tx_h, tx_w, 0, fb_w);
    cmd = create_newwin(cmd_h, cmd_w, tx_h, fb_w);

    int c;
    MEVENT event;

    while ((c = getch()) != KEY_F(1)) {
        // getyx(curscr, y, x);
        // mvprintw(row, 0, "current position: (%d, %d)\n", x, y);
        // refresh();
        switch (c) {
            case 't':
                getyx(text, y, x);
                mvwprintw(text, y + 1, x + 1, "Enter text mode!");
                wrefresh(text);
                break;
            case 'c':
                getyx(cmd, y, x);
                mvwprintw(cmd, y + 1, x + 1, "Enter cmd mode!");
                wrefresh(cmd);
                break;
            case 'f':
                getyx(fb, y, x);
                mvwprintw(fb, y + 1, x + 1, "Enter file mode!");
                wrefresh(fb);
                break;
            case KEY_MOUSE:
                if (getmouse(&event) == OK) {
                    if (event.bstate & BUTTON1_CLICKED) {
                        mvprintw(row, 0, "current position: (%d, %d, %d)\n", event.x, event.y, event.z);
                        refresh();
                    }
                }
                break;
            default:
                break;
        }
    }

    endwin(); /* End curses mode */
    return 0;
    // test = create_newwin(height, width, starty, startx);
}

void panel_basic() {
    WINDOW* my_wins[3];
    PANEL* my_panels[3];
    int lines = 10, cols = 40, y = 2, x = 4, i;
    initscr();
    cbreak();
    noecho();
    /* Create windows for the panels */
    my_wins[0] = newwin(lines, cols, y, x);
    my_wins[1] = newwin(lines, cols, y + 1, x + 5);
    my_wins[2] = newwin(lines, cols, y + 2, x + 10);
    /*
     * Create borders around the windows so that you can see the effect
     * of panels
     */
    for (i = 0; i < 3; ++i) box(my_wins[i], 0, 0);
    /* Attach a panel to each window */   /* Order is bottom up */
    my_panels[0] = new_panel(my_wins[0]); /* Push 0, order: stdscr-0 */
    my_panels[1] = new_panel(my_wins[1]); /* Push 1, order: stdscr-0-1 */
    my_panels[2] = new_panel(my_wins[2]); /* Push 2, order: stdscr-0-1-2 */
    /* Update the stacking order. 2nd panel will be on top */
    update_panels();
    /* Show it on the screen */
    doupdate();
    getch();
    endwin();
}

/**
 * @brief panel test
 *
 */

#define NLINES 10
#define NCOLS 40
/* Put all the windows */
void init_wins(WINDOW** wins, int n) {
    int x, y, i;
    char label[80];
    y = 2;
    x = 10;
    for (i = 0; i < n; ++i) {
        wins[i] = newwin(NLINES, NCOLS, y, x);
        sprintf(label, "Window Number %d", i + 1);
        win_show(wins[i], label, i + 1);
        y += 3;
        x += 7;
    }
}

/* Show the window with a border and a label */
void win_show(WINDOW* win, char* label, int label_color) {
    int startx, starty, height, width;
    getbegyx(win, starty, startx);
    getmaxyx(win, height, width);
    box(win, 0, 0);
    mvwaddch(win, 2, 0, ACS_LTEE);
    mvwhline(win, 2, 1, ACS_HLINE, width - 2);
    mvwaddch(win, 2, width - 1, ACS_RTEE);
    print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
}

void print_in_middle(WINDOW* win, int starty, int startx, int width, char* string, chtype color) {
    int length, x, y;
    float temp;
    if (win == NULL) win = stdscr;
    getyx(win, y, x);
    if (startx != 0) x = startx;
    if (starty != 0) y = starty;
    if (width == 0) width = 80;
    length = strlen(string);
    temp = (width - length) / 2;
    x = startx + (int)temp;
    wattron(win, color);
    mvwprintw(win, y, x, "%s", string);
    wattroff(win, color);
    refresh();
}

void panel_browser() {
    WINDOW* my_wins[3];
    PANEL* my_panels[3];
    PANEL* top;
    int ch;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* Initialize all the colors */
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
    init_wins(my_wins, 3);
    /* Attach a panel to each window */   /* Order is bottom up */
    my_panels[0] = new_panel(my_wins[0]); /* Push 0, order: stdscr-0 */
    my_panels[1] = new_panel(my_wins[1]); /* Push 1, order: stdscr-0-1 */
    my_panels[2] = new_panel(my_wins[2]); /* Push 2, order: stdscr-0-1-2 */
    /* Set up the user pointers to the next panel */
    set_panel_userptr(my_panels[0], my_panels[1]);
    set_panel_userptr(my_panels[1], my_panels[2]);
    set_panel_userptr(my_panels[2], my_panels[0]);
    /* Update the stacking order. 2nd panel will be on top */
    update_panels();
    /* Show it on the screen */
    attron(COLOR_PAIR(4));
    mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");
    attroff(COLOR_PAIR(4));
    doupdate();
    top = my_panels[2];
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case 9:
                top = (PANEL*)panel_userptr(top);
                top_panel(top);
                break;
        }
        update_panels();
        doupdate();
    }
    endwin();
    return 0;
}

/**
 * @brief menu
 *
 */
#define array_size(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 4
void menu_basic() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4", "Exit",
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    int n_choices, i;
    ITEM* cur_item;
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices + 1, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    my_items[n_choices] = (ITEM*)NULL;
    my_menu = new_menu((ITEM**)my_items);
    mvprintw(LINES - 2, 0, "F1 to Exit");
    post_menu(my_menu);
    refresh();
    while ((c = getch()) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
        }
    }
    free_item(my_items[0]);
    free_item(my_items[1]);
    free_menu(my_menu);
    endwin();
}

void menu_window() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4", "Exit", (char*)NULL,
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    WINDOW* my_menu_win;
    int n_choices, i;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    /* Create items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    /* Crate menu */
    my_menu = new_menu((ITEM**)my_items);
    /* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 40, 4, 4);
    keypad(my_menu_win, TRUE);
    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));
    /* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " * ");
    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
    print_in_middle(my_menu_win, 1, 0, 40, "My Menu", COLOR_PAIR(1));
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
    mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
    mvprintw(LINES - 2, 0, "F1 to exit");
    refresh();
    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);
    while ((c = wgetch(my_menu_win)) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
        }
        wrefresh(my_menu_win);
    }
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i) free_item(my_items[i]);
    endwin();
}

void menu_scroll() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4",  "Choice 5", "Choice 6",
        "Choice 7", "Choice 8", "Choice 9", "Choice 10", "Exit",     (char*)NULL,
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    WINDOW* my_menu_win;
    int n_choices, i;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    /* Create items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    /* Crate menu */
    my_menu = new_menu((ITEM**)my_items);
    /* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 40, 4, 4);
    keypad(my_menu_win, TRUE);
    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));
    set_menu_format(my_menu, 5, 1);
    /* Set menu mark to the string " * " */
    set_menu_mark(my_menu, " * ");
    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
    print_in_middle(my_menu_win, 1, 0, 40, "My Menu", COLOR_PAIR(1));
    mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
    mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
    mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);
    attron(COLOR_PAIR(2));
    mvprintw(LINES - 2, 0, "Use PageUp and PageDown to scoll down or up a page of items");
    mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
    attroff(COLOR_PAIR(2));
    refresh();
    while ((c = wgetch(my_menu_win)) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case KEY_NPAGE:
                menu_driver(my_menu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(my_menu, REQ_SCR_UPAGE);
                break;
        }
        wrefresh(my_menu_win);
    }
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i) free_item(my_items[i]);
    endwin();
}

void menu_multicol() {
    char* choices[] = {
        "Choice 1",  "Choice 2",  "Choice 3",  "Choice 4",  "Choice 5",  "Choice 6",  "Choice 7",  "Choice 8",
        "Choice 9",  "Choice 10", "Choice 11", "Choice 12", "Choice 13", "Choice 14", "Choice 15", "Choice 16",
        "Choice 17", "Choice 18", "Choice 19", "Choice 20", "Exit",      (char*)NULL,
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    WINDOW* my_menu_win;
    int n_choices, i;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    /* Create items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    /* Crate menu */
    my_menu = new_menu((ITEM**)my_items);
    /* Set menu option not to show the description */
    menu_opts_off(my_menu, O_SHOWDESC);
    /* Create the window to be associated with the menu */
    my_menu_win = newwin(10, 70, 4, 4);
    keypad(my_menu_win, TRUE);
    /* Set main window and sub window */
    set_menu_win(my_menu, my_menu_win);
    set_menu_sub(my_menu, derwin(my_menu_win, 6, 68, 3, 1));
    set_menu_format(my_menu, 5, 3);
    set_menu_mark(my_menu, " * ");
    /* Print a border around the main window and print a title */
    box(my_menu_win, 0, 0);
    attron(COLOR_PAIR(2));
    mvprintw(LINES - 3, 0, "Use PageUp and PageDown to scroll");
    mvprintw(LINES - 2, 0, "Use Arrow Keys to navigate (F1 to Exit)");
    attroff(COLOR_PAIR(2));
    refresh();
    /* Post the menu */
    post_menu(my_menu);
    wrefresh(my_menu_win);
    while ((c = wgetch(my_menu_win)) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case KEY_LEFT:
                menu_driver(my_menu, REQ_LEFT_ITEM);
                break;
            case KEY_RIGHT:
                menu_driver(my_menu, REQ_RIGHT_ITEM);
                break;
            case KEY_NPAGE:
                menu_driver(my_menu, REQ_SCR_DPAGE);
                break;
            case KEY_PPAGE:
                menu_driver(my_menu, REQ_SCR_UPAGE);
                break;
        }
        wrefresh(my_menu_win);
    }
    /* Unpost and free all the memory taken up */
    unpost_menu(my_menu);
    free_menu(my_menu);
    for (i = 0; i < n_choices; ++i) free_item(my_items[i]);
    endwin();
}

void menu_multival() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4", "Choice 5", "Choice 6", "Choice 7", "Exit",
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    int n_choices, i;
    ITEM* cur_item;
    /* Initialize curses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* Initialize items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices + 1, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    my_items[n_choices] = (ITEM*)NULL;
    my_menu = new_menu((ITEM**)my_items);
    /* Make the menu multi valued */
    menu_opts_off(my_menu, O_ONEVALUE);
    mvprintw(LINES - 3, 0, "Use <SPACE> to select or unselect an item.");
    mvprintw(LINES - 2, 0, "<ENTER> to see presently selected items(F1 to Exit)");
    post_menu(my_menu);
    refresh();
    while ((c = getch()) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case ' ':
                menu_driver(my_menu, REQ_TOGGLE_ITEM);
                break;
            case 10: /* Enter */
            {
                char temp[200];
                ITEM** items;
                items = menu_items(my_menu);
                temp[0] = '\0';
                for (i = 0; i < item_count(my_menu); ++i)
                    if (item_value(items[i]) == TRUE) {
                        strcat(temp, item_name(items[i]));
                        strcat(temp, " ");
                    }
                move(20, 0);
                clrtoeol();
                mvprintw(20, 0, temp);
                refresh();
            } break;
        }
    }
    free_item(my_items[0]);
    free_item(my_items[1]);
    free_menu(my_menu);
    endwin();
}

void menu_color() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4", "Choice 5", "Choice 6", "Choice 7", "Exit",
    };
    ITEM** my_items;
    int c;
    MENU* my_menu;
    int n_choices, i;
    ITEM* cur_item;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    /* Initialize items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices + 1, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) my_items[i] = new_item(choices[i], choices[i]);
    my_items[n_choices] = (ITEM*)NULL;
    item_opts_off(my_items[3], O_SELECTABLE);
    item_opts_off(my_items[6], O_SELECTABLE);
    /* Create menu */
    my_menu = new_menu((ITEM**)my_items);
    /* Set fore ground and back ground of the menu */
    set_menu_fore(my_menu, COLOR_PAIR(1) | A_REVERSE);
    set_menu_back(my_menu, COLOR_PAIR(2));
    set_menu_grey(my_menu, COLOR_PAIR(3));
    /* Post the menu */
    mvprintw(LINES - 3, 0, "Press <ENTER> to see the option selected");
    mvprintw(LINES - 2, 0, "Up and Down arrow keys to naviage (F1 to Exit)");
    post_menu(my_menu);
    refresh();
    while ((c = getch()) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case 10: /* Enter */
                move(20, 0);
                clrtoeol();
                mvprintw(20, 0, "Item selected is : %s", item_name(current_item(my_menu)));
                pos_menu_cursor(my_menu);
                break;
        }
    }
    unpost_menu(my_menu);
    for (i = 0; i < n_choices; ++i) free_item(my_items[i]);
    free_menu(my_menu);
    endwin();
}

void func(char* name) {
    move(20, 0);
    clrtoeol();
    mvprintw(20, 0, "Item selected is : %s", name);
}

void menu_user() {
    char* choices[] = {
        "Choice 1", "Choice 2", "Choice 3", "Choice 4", "Choice 5", "Choice 6", "Choice 7", "Exit",
    };

    ITEM** my_items;
    int c;
    MENU* my_menu;
    int n_choices, i;
    ITEM* cur_item;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK);
    /* Initialize items */
    n_choices = array_size(choices);
    my_items = (ITEM**)calloc(n_choices + 1, sizeof(ITEM*));
    for (i = 0; i < n_choices; ++i) {
        my_items[i] = new_item(choices[i], choices[i]);
        /* Set the user pointer */
        set_item_userptr(my_items[i], func);
    }
    my_items[n_choices] = (ITEM*)NULL;
    /* Create menu */
    my_menu = new_menu((ITEM**)my_items);
    /* Post the menu */
    mvprintw(LINES - 3, 0, "Press <ENTER> to see the option selected");
    mvprintw(LINES - 2, 0, "Up and Down arrow keys to naviage (F1 to Exit)");
    post_menu(my_menu);
    refresh();
    while ((c = getch()) != KEY_F(1)) {
        switch (c) {
            case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case 10: /* Enter */
            {
                ITEM* cur;
                void (*p)(char*);
                cur = current_item(my_menu);
                p = item_userptr(cur);
                p((char*)item_name(cur));
                pos_menu_cursor(my_menu);
                break;
            } break;
        }
    }
    unpost_menu(my_menu);
    for (i = 0; i < n_choices; ++i) free_item(my_items[i]);
    free_menu(my_menu);
    endwin();
}

void form_basic() {
    FIELD* field[3];
    FORM* my_form;
    int ch;
    /* Initialize curses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* Initialize the fields */
    field[0] = new_field(1, 10, 4, 18, 0, 0);
    field[1] = new_field(1, 10, 6, 18, 0, 0);
    field[2] = NULL;
    /* Set field options */
    set_field_back(field[0], A_UNDERLINE); /* Print a line for the option */
    field_opts_off(field[0], O_AUTOSKIP);  /* Don't go to next field when this */
    /* Field is filled up */
    set_field_back(field[1], A_UNDERLINE);
    field_opts_off(field[1], O_AUTOSKIP);
    /* Create the form and post it */
    my_form = new_form(field);
    post_form(my_form);
    refresh();
    mvprintw(4, 10, "Value 1:");
    mvprintw(6, 10, "Value 2:");
    refresh();
    /* Loop through to get user requests */
    while ((ch = getch()) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                /* Go to next field */
                form_driver(my_form, REQ_NEXT_FIELD);
                /* Go to the end of the present buffer */
                /* Leaves nicely at the last character */
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                /* Go to previous field */
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            default:
                /* If this is a normal character, it gets */
                /* Printed */
                form_driver(my_form, ch);
                break;
        }
    }
    /* Un post form and free the memory */
    unpost_form(my_form);
    free_form(my_form);
    free_field(field[0]);
    free_field(field[1]);
    endwin();
    return 0;
}

// int set_field_userptr(FIELD *field,
// char *userptr); /* the user pointer you wish to associate */
// /* with the field */
// char *field_userptr(FIELD *field); /* fetch user pointer of the field */

void form_windows() {
    FIELD* field[3];
    FORM* my_form;
    WINDOW* my_form_win;
    int ch, rows, cols;
    /* Initialize curses */
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    /* Initialize few color pairs */
    init_pair(1, COLOR_RED, COLOR_BLACK);
    /* Initialize the fields */
    field[0] = new_field(1, 10, 6, 1, 0, 0);
    field[1] = new_field(1, 10, 8, 1, 0, 0);
    field[2] = NULL;
    /* Set field options */
    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP); /* Don't go to next field when this */
    /* Field is filled up */
    set_field_back(field[1], A_UNDERLINE);
    field_opts_off(field[1], O_AUTOSKIP);
    set_field_userptr(field[0], func);

    /* Create the form and post it */
    my_form = new_form(field);
    /* Calculate the area required for the form */
    scale_form(my_form, &rows, &cols);
    /* Create the window to be associated with the form */
    my_form_win = newwin(rows + 4, cols + 4, 4, 4);
    keypad(my_form_win, TRUE);
    /* Set main window and sub window */
    set_form_win(my_form, my_form_win);
    set_form_sub(my_form, derwin(my_form_win, rows, cols, 2, 2));
    /* Print a border around the main window and print a title */
    box(my_form_win, 0, 0);
    print_in_middle(my_form_win, 1, 0, cols + 4, "My Form", COLOR_PAIR(1));
    post_form(my_form);
    wrefresh(my_form_win);
    mvprintw(LINES - 2, 0, "Use UP, DOWN arrow keys to switch between fields");
    refresh();
    /* Loop through to get user requests */
    while ((ch = wgetch(my_form_win)) != KEY_F(1)) {
        switch (ch) {
            case KEY_DOWN:
                /* Go to next field */
                form_driver(my_form, REQ_NEXT_FIELD);
                /* Go to the end of the present buffer */
                /* Leaves nicely at the last character */
                form_driver(my_form, REQ_END_LINE);
                break;
            case KEY_UP:
                /* Go to previous field */
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;
            case 10: {
                void (*p)(char*);
                p = field_userptr(field[0]);

                p("sab");
                break;
            } break;

            default:
                /* If this is a normal character, it gets */
                /* Printed */
                form_driver(my_form, ch);
                break;
        }
    }
    /* Un post form and free the memory */
    unpost_form(my_form);
    free_form(my_form);
    free_field(field[0]);
    free_field(field[1]);
    endwin();
    return 0;
}

void test_ncurses() {
    // basic();
    // init();
    // wp();
    // sw();
    // window();
    // gui();
    // menu_basic();
    // menu_window();
    // menu_scroll();
    // menu_multicol();
    // menu_multival();
    // menu_color();
    // menu_user();
    // form_basic();
    form_windows();
}