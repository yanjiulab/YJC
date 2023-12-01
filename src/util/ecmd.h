/*
 * ecmd.h - A library for writing enhanced command-line applications.
 */

#ifndef ECMD_H_INCLUDE
#define ECMD_H_INCLUDE

#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>

#ifdef CMDF_READLINE_SUPPORT
#include <readline/history.h>
#include <readline/readline.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



/* For the C++ support. */
#ifdef __cplusplus
}
#endif

#endif