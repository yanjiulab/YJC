#ifndef _SEV_H_
#define _SEV_H_

#include <errno.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define NHANDLERS 16
#define TIMER_SECOND_MICRO 1000000L

#define SEV_SELECT 0
#define SEV_POLL 1
#define SEV_EPOLL 2

typedef void (*sev_cb)(int, fd_set*);
typedef void (*cfunc_t)(unsigned int);

struct sev_handler {
    int fd;        /* File descriptor				 */
    sev_cb func; /* Function to call with &fd_set */
} ;

void sev_runloop();
int ser_register(int fd, sev_cb func);

#endif