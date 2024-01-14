#include "sev.h"

#include <errno.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

/* Definitions */
static int sev_nhandlers = 0;

// static struct sev_handler sev_handlers[NHANDLERS];

static struct sev_handler {
    int fd;        /* File descriptor				 */
    sev_cb func; /* Function to call with &fd_set */
} sev_handlers[NHANDLERS];

static struct timeval timeval_adjust(struct timeval a) {
    while (a.tv_usec >= TIMER_SECOND_MICRO) {
        a.tv_usec -= TIMER_SECOND_MICRO;
        a.tv_sec++;
    }
    while (a.tv_usec < 0) {
        a.tv_usec += TIMER_SECOND_MICRO;
        a.tv_sec--;
    }
    if (a.tv_sec < 0) /* Change negative timeouts to 0. */
        a.tv_sec = a.tv_usec = 0;
    return a;
}

static struct timeval timeval_subtract(struct timeval a, struct timeval b) {
    struct timeval ret;
    ret.tv_usec = a.tv_usec - b.tv_usec;
    ret.tv_sec = a.tv_sec - b.tv_sec;
    return timeval_adjust(ret);
}

int ser_register(int fd, sev_cb func) {
    if (sev_nhandlers >= NHANDLERS) {
        return -1;
    }

    sev_handlers[sev_nhandlers].fd = fd;
    sev_handlers[sev_nhandlers++].func = func;
    return 0;
}

void sev_runloop() {
    struct timeval tv, difftime, curtime, lasttime, *timeout;
    fd_set rfds, allset;
    int nfds = 0, n, i, secs, ch;
    char* tmp;

    difftime.tv_usec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;

    /* Main loop */
    while (1) {
        memcpy(&rfds, &allset, sizeof(rfds));
        secs = timer_nextTimer();
        log_trace("[TIMEOUT: %2d secs]  > ", secs);

        if (secs == -1)
            timeout = NULL;
        else {
            timeout = &tv;
            timeout->tv_sec = secs;
            timeout->tv_usec = 0;
        }

        gettimeofday(&curtime, NULL);

        if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0) {
            // printf("select<0\n");
            if (errno != EINTR) /* SIGALRM is expected */
                log_warn("select failed");
            continue;
        }

        do {
            /*
             * If the select timed out, then there's no other
             * activity to account for and we don't need to
             * call gettimeofday.
             */
            if (n == 0) {
                curtime.tv_sec = lasttime.tv_sec + secs;
                curtime.tv_usec = lasttime.tv_usec;
                n = -1; /* don't do this next time through the loop */
            } else {
                gettimeofday(&curtime, NULL);
            }

            difftime.tv_sec = curtime.tv_sec - lasttime.tv_sec;
            difftime.tv_usec += curtime.tv_usec - lasttime.tv_usec;

            while (difftime.tv_usec > 1000000) {
                difftime.tv_sec++;
                difftime.tv_usec -= 1000000;
            }

            if (difftime.tv_usec < 0) {
                difftime.tv_sec--;
                difftime.tv_usec += 1000000;
            }

            lasttime = curtime;

            if (secs == 0 || difftime.tv_sec > 0) {
                age_callout_queue(difftime.tv_sec);
            }

            secs = -1;
        } while (difftime.tv_sec > 0);

        /* Handle sockets */
        if (n > 0) {
            for (i = 0; i < sev_nhandlers; i++) {
                if (FD_ISSET(sev_handlers[i].fd, &rfds)) {
                    (*sev_handlers[i].func)(sev_handlers[i].fd, &rfds);
                }
            }
        }

    } /* Main loop */
}