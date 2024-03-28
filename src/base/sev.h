#ifndef _SEV_H_
#define _SEV_H_

#include <errno.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "heap.h" // for heap
#include "base.h" // for container of
#include "list.h"

#define NHANDLERS          16
#define TIMER_SECOND_MICRO 1000000L

#define SEV_SELECT         0
#define SEV_POLL           1
#define SEV_EPOLL          2

// #define evloop(ev)         (((evbase_t*)(ev))->loop)

// typedef int EV_RETURN;
typedef struct evloop evloop_t;
typedef struct evbase evbase_t;
// NOTE: The following structures are subclasses of evbase_t,
// inheriting evbase_t data members and function members.
typedef struct evidle evidle_t;
typedef struct evtimer evtimer_t;
typedef struct evtimeout evtimeout_t;
typedef struct evperiod evperiod_t;
typedef struct evio evio_t;

typedef void (*evbase_cb)(evbase_t*);
typedef void (*evtimer_cb)(evtimer_t*);
typedef void (*evidle_cb)(evidle_t*);
typedef void (*evio_cb)(evio_t*);

typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_IO = 0x00000001,
    EVENT_TYPE_TIMEOUT = 0x00000010,
    EVENT_TYPE_PERIOD = 0x00000020,
    EVENT_TYPE_TIMER = EVENT_TYPE_TIMEOUT | EVENT_TYPE_PERIOD,
    EVENT_TYPE_IDLE = 0x00000100,
    EVENT_TYPE_CUSTOM = 0x00000400, // 1024
} event_type_t;

typedef enum {
    EVLOOP_STATUS_STOP,
    EVLOOP_STATUS_RUNNING
} evloop_status_t;

/* Event priority */
#define EVENT_LOWEST_PRIORITY          (-5)
#define EVENT_LOW_PRIORITY             (-3)
#define EVENT_NORMAL_PRIORITY          0
#define EVENT_HIGH_PRIORITY            3
#define EVENT_HIGHEST_PRIORITY         5
#define EVENT_PRIORITY_SIZE            (EVENT_HIGHEST_PRIORITY - EVENT_LOWEST_PRIORITY + 1)
#define EVENT_PRIORITY_INDEX(priority) (priority - EVENT_LOWEST_PRIORITY)

struct evloop {
    uint32_t flags;
    evloop_status_t status;
    uint64_t start_ms;     // ms
    uint64_t start_hrtime; // us
    uint64_t end_hrtime;
    uint64_t cur_hrtime;
    uint64_t loop_cnt;
    long pid;
    long tid;
    void* userdata;
    // private:
    //  events
    uint32_t nactives;
    uint32_t npendings;
    // pendings: with priority as array.index
    evbase_t* pendings[EVENT_PRIORITY_SIZE];
    // idles
    struct list_head idles;
    uint32_t nidles;
    // timers
    struct heap timers;     // monotonic time
    struct heap realtimers; // realtime
    uint32_t ntimers;

    // ios
    int max_ios;      // max io number
    int nios;         // number of ios
    struct evio* ios; // registed ios
    fd_set rfds;      // select read fds
    fd_set allset;    // select all fds
    int nfds;         // the max number of fd
};

#define EVENT_FLAGS       \
    unsigned destroy : 1; \
    unsigned active : 1;  \
    unsigned pending : 1;

#define EVENT_FIELDS             \
    evloop_t* loop;              \
    event_type_t event_type;     \
    uint64_t event_id;           \
    evbase_cb cb;                \
    void* userdata;              \
    void* privdata;              \
    struct evbase* pending_next; \
    int priority;                \
    EVENT_FLAGS

struct evbase {
    EVENT_FIELDS
};

struct evidle {
    EVENT_FIELDS
    uint32_t repeat;
    // private:
    struct list_node node;
};

#define TIMER_FIELDS       \
    EVENT_FIELDS           \
    uint32_t repeat;       \
    uint64_t next_timeout; \
    struct heap_node node;

struct evtimer {
    TIMER_FIELDS
};

struct evtimeout {
    TIMER_FIELDS
    uint32_t timeout;
};

struct evperiod {
    TIMER_FIELDS
    int8_t minute;
    int8_t hour;
    int8_t day;
    int8_t week;
    int8_t month;
};

struct evio {
    EVENT_FIELDS
    int fd;       /* File descriptor				 */
    evio_cb func; /* Function to call with &fd_set */
};

// evloop
#define EVLOOP_FLAG_RUN_ONCE                   0x00000001
#define EVLOOP_FLAG_AUTO_FREE                  0x00000002
#define EVLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS 0x00000004
evloop_t* evloop_new(int max);
void evloop_free(evloop_t** pp);
int evloop_run(evloop_t* loop);
int evloop_stop(evloop_t* loop);
void evloop_update_time(evloop_t* loop);
uint64_t evloop_now(evloop_t* loop);        // s
uint64_t evloop_now_ms(evloop_t* loop);     // ms
uint64_t evloop_now_us(evloop_t* loop);     // us
uint64_t evloop_now_hrtime(evloop_t* loop); // us

// evbase
#define event_set_id(ev, id)          ((evbase_t*)(ev))->event_id = id
#define event_set_cb(ev, cb)          ((evbase_t*)(ev))->cb = cb
#define event_set_priority(ev, prio)  ((evbase_t*)(ev))->priority = prio
#define event_set_userdata(ev, udata) ((evbase_t*)(ev))->userdata = (void*)udata

#define event_loop(ev)                (((evbase_t*)(ev))->loop)
// #define event_type(ev)                (((evbase_t*)(ev))->event_type)
#define event_id(ev)                  (((evbase_t*)(ev))->event_id)
#define event_cb(ev)                  (((evbase_t*)(ev))->cb)
#define event_priority(ev)            (((evbase_t*)(ev))->priority)
#define event_userdata(ev)            (((evbase_t*)(ev))->userdata)
uint64_t evloop_next_event_id();

#define TIMER_ENTRY(p) container_of(p, evtimer_t, node)
#define EVENT_ENTRY(p) container_of(p, evbase_t, pending_node)
#define IDLE_ENTRY(p)  container_of(p, evidle_t, node)

#define event_active(ev)      \
    if (!ev->active) {        \
        ev->active = 1;       \
        ev->loop->nactives++; \
    }

#define event_inactive(ev)    \
    if (ev->active) {         \
        ev->active = 0;       \
        ev->loop->nactives--; \
    }

#define event_pending(ev)                                                               \
    do {                                                                                \
        if (!ev->pending) {                                                             \
            ev->pending = 1;                                                            \
            ev->loop->npendings++;                                                      \
            evbase_t** phead = &ev->loop->pendings[EVENT_PRIORITY_INDEX(ev->priority)]; \
            ev->pending_next = *phead;                                                  \
            *phead = (evbase_t*)ev;                                                     \
        }                                                                               \
    } while (0)

#define event_add(loop, ev, cb)                \
    do {                                       \
        ev->loop = loop;                       \
        ev->event_id = evloop_next_event_id(); \
        ev->cb = (evbase_cb)cb;                \
        event_active(ev);                      \
    } while (0)

#define event_del(ev)       \
    do {                    \
        event_inactive(ev); \
        if (!ev->pending) { \
            EV_FREE(ev);    \
        }                   \
    } while (0)

#define event_reset(ev)   \
    do {                  \
        ev->destroy = 0;  \
        event_active(ev); \
        ev->pending = 0;  \
    } while (0)

// evidle
evidle_t* evidle_add(evloop_t* loop, evidle_cb cb, uint32_t repeat);
void evidle_del(evidle_t* idle);

// evtimer
evtimer_t* evtimer_add(evloop_t* loop, evtimer_cb cb, uint32_t timeout_ms, uint32_t repeat);
/*
 * minute   hour    day     week    month       cb
 * 0~59     0~23    1~31    0~6     1~12
 *  -1      -1      -1      -1      -1          cron.minutely
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       5      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
evtimer_t* evtimer_add_period(evloop_t* loop, evtimer_cb cb, int8_t minute, int8_t hour,
                            int8_t day, int8_t week, int8_t month,
                            uint32_t repeat);
void evtimer_del(evtimer_t* timer);
void evtimer_reset(evtimer_t* timer, uint32_t etimeout_ms);

// evio
int evio_add(evloop_t* loop, int fd, evio_cb cb);

#endif