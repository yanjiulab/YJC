#ifndef _TERM_H_
#define _TERM_H_

#include <termios.h>
#include <sys/ioctl.h>

// get window size column
int get_ws_col();

// get window size row
int get_ws_row();

/**
 * @brief Print a string of length LEN consisting of C characters.
 * 
 * @param c separator
 * @param len length is terminal window column size if LEN is 0, otherwise LEN.
 */
void print_line(char c, int len);

/**
 * @brief Print the TITLE in center alignment style.
 * 
 * @param title print string
 * @param c separator
 */
void print_title(char *title, char c);

#endif