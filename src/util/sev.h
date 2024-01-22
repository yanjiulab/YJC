#ifndef _SEV_H_
#define _SEV_H_

#include <errno.h>
#include <stdlib.h>
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

#define event_loop(ev) (((struct evtimer*)(ev))->loop)

typedef enum { EVLOOP_STATUS_STOP,
               EVLOOP_STATUS_RUNNING } evloop_status_e;

typedef int EV_RETURN;
typedef EV_RETURN (*evtimer_cb)(struct evtimer*, void*);
typedef EV_RETURN (*evio_cb)(int, fd_set*);

/* Error codes (for EV_RETURN) */
#define EV_OK 1
// #define EV_ERROR_* -1

typedef struct evloop_s {
    evloop_status_e status;
    // ios
    int max_ios;
    int nios;
    struct evio* ios;
    fd_set rfds;   // select read fds
    fd_set allset; // select all fds
    int nfds;      // the max fd

    // timers
    struct evtimer* timers;

} evloop_t;

struct evio {
    int fd;       /* File descriptor				 */
    evio_cb func; /* Function to call with &fd_set */
};

typedef struct evtimer {
    evloop_t* loop;
    struct evtimer* next; /* next timer event */
    int id;
    evtimer_cb func; /* function to call */
    void* data;      /* func's data */
    int time;        /* time offset to next timer event*/
    int repeat;
} evtimer_t;

// evloop
evloop_t* evloop_new(int max);
void evloop_free(evloop_t** pp);
int evloop_run(evloop_t* loop);
void evloop_stop(evloop_t* loop);

// evtimer
void evtimer_add(evloop_t* loop, evtimer_cb cb, void* data, uint32_t etimeout_ms);
void evtimer_del(evloop_t* loop, int timer_id);
void evtimer_clean(evloop_t* loop);
// Return in how many ms evtimer_callout() would like to be called.
// Return -1 if there are no events pending.
int evtimer_next(evloop_t* loop);
/* elapsed_time seconds have passed; perform all the events that should happen. */
void evtimer_callout(evloop_t* loop, int elapsed_time);
/* returns the time until the timer is scheduled */
int evtimer_left(evloop_t* loop, int timer_id);
int evtimer_reset(evloop_t* loop, int timer_id, int delay);

// evio
int evio_add(evloop_t* loop, int fd, evio_cb cb);

#endif