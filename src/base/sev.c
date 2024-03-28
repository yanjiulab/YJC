#include "sev.h"

#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

#define EVLOOP_MAX_BLOCK_TIME 100 // ms

/* Definitions */
static int id = 0;

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

// evidle
#define EVIDLE
evidle_t* evidle_add(evloop_t* loop, evidle_cb cb, uint32_t repeat) {
    evidle_t* idle;
    EV_ALLOC_SIZEOF(idle);
    idle->event_type = EVENT_TYPE_IDLE;
    idle->priority = EVENT_LOWEST_PRIORITY;
    idle->repeat = repeat;
    list_add(&idle->node, &loop->idles);
    event_add(loop, idle, cb);
    loop->nidles++;
    return idle;
}

static void __evidle_del(evidle_t* idle) {
    if (idle->destroy)
        return;
    idle->destroy = 1;
    list_del(&idle->node);
    idle->loop->nidles--;
}

void evidle_del(evidle_t* idle) {
    if (!idle->active)
        return;
    __evidle_del(idle);
    event_del(idle);
}

// evtimer
#define EVTIMER
static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->next_timeout < TIMER_ENTRY(rhs)->next_timeout;
}

evtimer_t* evtimer_add(evloop_t* loop, evtimer_cb cb, uint32_t timeout_ms, uint32_t repeat) {

    if (timeout_ms == 0)
        return NULL;
    evtimeout_t* timer;
    EV_ALLOC_SIZEOF(timer);
    timer->event_type = EVENT_TYPE_TIMEOUT;
    timer->priority = EVENT_HIGHEST_PRIORITY;
    timer->repeat = repeat;
    timer->timeout = timeout_ms;
    evloop_update_time(loop);
    timer->next_timeout = loop->cur_hrtime + (uint64_t)timeout_ms * 1000;
    // NOTE: Limit granularity to 100ms
    if (timeout_ms >= 1000 && timeout_ms % 100 == 0) {
        timer->next_timeout = timer->next_timeout / 100000 * 100000;
    }

    heap_insert(&loop->timers, &timer->node);
    event_add(loop, timer, cb);
    loop->ntimers++;
    return (evtimer_t*)timer;
}

evtimer_t* evtimer_add_period(evloop_t* loop, evtimer_cb cb, int8_t minute, int8_t hour, int8_t day, int8_t week,
                             int8_t month, uint32_t repeat) {
    if (minute > 59 || hour > 23 || day > 31 || week > 6 || month > 12) {
        return NULL;
    }
    evperiod_t* timer;
    EV_ALLOC_SIZEOF(timer);
    timer->event_type = EVENT_TYPE_PERIOD;
    timer->priority = EVENT_HIGH_PRIORITY;
    timer->repeat = repeat;
    timer->minute = minute;
    timer->hour = hour;
    timer->day = day;
    timer->month = month;
    timer->week = week;
    timer->next_timeout = (uint64_t)cron_next_timeout(minute, hour, day, week, month) * 1000000;
    heap_insert(&loop->realtimers, &timer->node);
    event_add(loop, timer, cb);
    loop->ntimers++;
    return (evtimer_t*)timer;
}

void evtimer_reset(evtimer_t* timer, uint32_t timeout_ms) {
    if (timer->event_type != EVENT_TYPE_TIMEOUT) {
        return;
    }
    evloop_t* loop = timer->loop;
    evtimeout_t* timeout = (evtimeout_t*)timer;
    if (timer->destroy) {
        loop->ntimers++;
    } else {
        heap_remove(&loop->timers, &timer->node);
    }
    if (timer->repeat == 0) {
        timer->repeat = 1;
    }
    if (timeout_ms > 0) {
        timeout->timeout = timeout_ms;
    }
    timer->next_timeout = loop->cur_hrtime + (uint64_t)timeout->timeout * 1000;
    // NOTE: Limit granularity to 100ms
    if (timeout->timeout >= 1000 && timeout->timeout % 100 == 0) {
        timer->next_timeout = timer->next_timeout / 100000 * 100000;
    }
    heap_insert(&loop->timers, &timer->node);
    event_reset(timer);
}

static void __evtimer_del(evtimer_t* timer) {
    if (timer->destroy)
        return;
    // if (timer->event_type == EVENT_TYPE_TIMEOUT) {
    heap_remove(&timer->loop->timers, &timer->node);
    // } else if (timer->event_type == EVENT_TYPE_PERIOD) {
    //     heap_remove(&timer->loop->realtimers, &timer->node);
    // }
    timer->loop->ntimers--;
    timer->destroy = 1;
}

void evtimer_del(evtimer_t* timer) {

    if (!timer->active)
        return;
    __evtimer_del(timer);
    event_del(timer);
}

// evio
#define EVIO
int evio_add(evloop_t* loop, int fd, evio_cb cb) {
    if (loop->nios >= loop->max_ios) {
        return -1;
    }
    loop->ios[loop->nios].loop = loop;
    loop->ios[loop->nios].fd = fd;
    loop->ios[loop->nios].func = cb;

    FD_SET(fd, &loop->allset);

    if (loop->ios[loop->nios].fd >= loop->nfds) {
        loop->nfds = loop->ios[loop->nios].fd + 1;
    }

    loop->nios++;

    return 0;
}

// evloop
#define EVLOOP
static int evloop_process_idles(evloop_t* loop) {
    int nidles = 0;
    struct list_node* node = loop->idles.next;
    evidle_t* idle = NULL;
    while (node != &loop->idles) {
        idle = IDLE_ENTRY(node);
        node = node->next;
        if (idle->repeat != INFINITE) {
            --idle->repeat;
        }
        if (idle->repeat == 0) {
            // NOTE: Just mark it as destroy and remove from list.
            // Real deletion occurs after evloop_process_pendings.
            __evidle_del(idle);
        }
        event_pending(idle);
        ++nidles;
    }
    return nidles;
}

static int __evloop_process_timers(struct heap* timers, uint64_t timeout) {
    int ntimers = 0;
    evtimer_t* timer = NULL;
    while (timers->root) {
        // NOTE: root of minheap has min timeout.
        timer = TIMER_ENTRY(timers->root);
        if (timer->next_timeout > timeout) {
            break;
        }
        if (timer->repeat != INFINITE) {
            --timer->repeat;
        }
        if (timer->repeat == 0) {
            // NOTE: Just mark it as destroy and remove from heap.
            // Real deletion occurs after evloop_process_pendings.
            __evtimer_del(timer);
        } else {
            // NOTE: calc next timeout, then re-insert heap.
            heap_dequeue(timers);
            if (timer->event_type == EVENT_TYPE_TIMEOUT) {
                while (timer->next_timeout <= timeout) {
                    timer->next_timeout += (uint64_t)((evtimeout_t*)timer)->timeout * 1000;
                }
            } else if (timer->event_type == EVENT_TYPE_PERIOD) {
                evperiod_t* period = (evperiod_t*)timer;
                timer->next_timeout = (uint64_t)cron_next_timeout(period->minute, period->hour, period->day,
                                                                  period->week, period->month) *
                                      1000000;
            }
            heap_insert(timers, &timer->node);
        }
        event_pending(timer);
        ++ntimers;
    }
    return ntimers;
}

static int evloop_process_timers(evloop_t* loop) {
    uint64_t now = evloop_now_us(loop);
    int ntimers = __evloop_process_timers(&loop->timers, loop->cur_hrtime);
    ntimers += __evloop_process_timers(&loop->realtimers, now);
    return ntimers;
}

static int evloop_process_pendings(evloop_t* loop) {
    if (loop->npendings == 0)
        return 0;

    evbase_t* cur = NULL;
    evbase_t* next = NULL;
    int ncbs = 0;
    // NOTE: invoke event callback from high to low sorted by priority.
    for (int i = EVENT_PRIORITY_SIZE - 1; i >= 0; --i) {
        cur = loop->pendings[i];
        while (cur) {
            next = cur->pending_next;
            if (cur->pending) {
                if (cur->active && cur->cb) {
                    cur->cb(cur);
                    ++ncbs;
                }
                cur->pending = 0;
                // NOTE: Now we can safely delete event marked as destroy.
                if (cur->destroy) {
                    event_del(cur);
                }
            }
            cur = next;
        }
        loop->pendings[i] = NULL;
    }
    loop->npendings = 0;
    return ncbs;
}

// evloop_process_ios -> evloop_process_timers -> evloop_process_idles -> evloop_process_pendings
static int evloop_process_events(evloop_t* loop) {
    // ios -> timers -> idles
    int nios, ntimers, nidles;
    nios = ntimers = nidles = 0;

    // calc blocktime
    int32_t blocktime_ms = EVLOOP_MAX_BLOCK_TIME;
    if (loop->ntimers) {
        evloop_update_time(loop);
        int64_t blocktime_us = blocktime_ms * 1000;
        if (loop->timers.root) {
            int64_t min_timeout = TIMER_ENTRY(loop->timers.root)->next_timeout - loop->cur_hrtime;
            blocktime_us = MIN(blocktime_us, min_timeout);
        }
        if (loop->realtimers.root) {
            int64_t min_timeout = TIMER_ENTRY(loop->realtimers.root)->next_timeout - evloop_now_us(loop);
            blocktime_us = MIN(blocktime_us, min_timeout);
        }
        if (blocktime_us <= 0)
            goto process_timers;
        blocktime_ms = blocktime_us / 1000 + 1;
        blocktime_ms = MIN(blocktime_ms, EVLOOP_MAX_BLOCK_TIME);
    }

    if (loop->nios) {
        // nios = evloop_process_ios(loop, blocktime_ms);
    } else {
        ev_msleep(blocktime_ms);
    }
    evloop_update_time(loop);
    // wakeup by evloop_stop
    if (loop->status == EVLOOP_STATUS_STOP) {
        return 0;
    }

process_timers:
    if (loop->ntimers) {
        ntimers = evloop_process_timers(loop);
    }

    int npendings = loop->npendings;
    if (npendings == 0) {
        if (loop->nidles) {
            nidles = evloop_process_idles(loop);
        }
    }
    int ncbs = evloop_process_pendings(loop);
    // printd("blocktime=%d nios=%d/%u ntimers=%d/%u nidles=%d/%u nactives=%d npendings=%d ncbs=%d\n",
    //        blocktime_ms, nios, loop->nios, ntimers, loop->ntimers, nidles, loop->nidles,
    //        loop->nactives, npendings, ncbs);
    return ncbs;
    return 0;
}

evloop_t* evloop_new(int max) {
    evloop_t* loop;
    EV_ALLOC_SIZEOF(loop);

    loop->status = EVLOOP_STATUS_STOP;
    // loop->pid = getpid();
    // loop->tid = gettid();

    // idels
    list_init(&loop->idles);

    // timers
    heap_init(&loop->timers, timers_compare);
    heap_init(&loop->realtimers, timers_compare);

    // ios
    loop->max_ios = max;
    loop->ios = calloc(max, sizeof(struct evio));
    loop->nios = 0;
    FD_ZERO(&loop->allset);
    FD_ZERO(&loop->rfds);

    // NOTE: init start_time here, because evtimer_add use it.
    loop->start_ms = gettimeofday_ms();
    loop->start_hrtime = loop->cur_hrtime = gethrtime_us();

    // loop->flags |= flags;
    loop->flags |= EVLOOP_FLAG_AUTO_FREE;
    return loop;
}

static void evloop_cleanup(evloop_t* loop) {
    // pendings
    printd("cleanup pendings...\n");
    for (int i = 0; i < EVENT_PRIORITY_SIZE; ++i) {
        loop->pendings[i] = NULL;
    }

    // ios
    // printd("cleanup ios...\n");
    // for (int i = 0; i < loop->ios.maxsize; ++i) {
    //     eio_t* io = loop->ios.ptr[i];
    //     if (io) {
    //         eio_free(io);
    //     }
    // }
    // io_array_cleanup(&loop->ios);

    // idles
    // printd("cleanup idles...\n");
    // struct list_node* node = loop->idles.next;
    // evidle_t* idle;
    // while (node != &loop->idles) {
    //     idle = IDLE_ENTRY(node);
    //     node = node->next;
    //     EV_FREE(idle);
    // }
    // list_init(&loop->idles);

    // timers
    printd("cleanup timers...\n");
    evtimer_t* timer;
    while (loop->timers.root) {
        timer = TIMER_ENTRY(loop->timers.root);
        heap_dequeue(&loop->timers);
        EV_FREE(timer);
    }
    heap_init(&loop->timers, NULL);
    // while (loop->realtimers.root) {
    //     timer = TIMER_ENTRY(loop->realtimers.root);
    //     heap_dequeue(&loop->realtimers);
    //     EV_FREE(timer);
    // }
    // heap_init(&loop->realtimers, NULL);

    // readbuf
    // if (loop->readbuf.base && loop->readbuf.len) {
    //     EV_FREE(loop->readbuf.base);
    //     loop->readbuf.base = NULL;
    //     loop->readbuf.len = 0;
    // }

    // iowatcher
    // iowatcher_cleanup(loop);

    // custom_events
    // mutex_lock(&loop->custom_events_mutex);
    // evloop_destroy_eventfds(loop);
    // event_queue_cleanup(&loop->custom_events);
    // mutex_unlock(&loop->custom_events_mutex);
    // mutex_destroy(&loop->custom_events_mutex);
}

void evloop_free(evloop_t** pp) {
    if (pp && *pp) {
        evloop_cleanup(*pp);
        EV_FREE(*pp);
        *pp = NULL;
    }
}

int evloop_run(evloop_t* loop) {
    if (loop == NULL)
        return -1;
    if (loop->status == EVLOOP_STATUS_RUNNING)
        return -2;

    loop->status = EVLOOP_STATUS_RUNNING;

    struct timeval tv, difftime, curtime, lasttime, *timeout;
    int n, i, ms, diffms;
    difftime.tv_usec = 0;
    difftime.tv_sec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;

    /* Main loop */
    while (loop->status != EVLOOP_STATUS_STOP) {
        // new section
        ++loop->loop_cnt;
        if ((loop->flags & EVLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS) && loop->nactives == 0) {
            break;
        }
        evloop_process_events(loop);
        if (loop->flags & EVLOOP_FLAG_RUN_ONCE) {
            break;
        }
        // end new section

        // memcpy(&loop->rfds, &loop->allset, sizeof(loop->rfds));
        // ms = evtimer_next(loop);

        // if (ms == -1)
        //     timeout = NULL;
        // else {
        //     timeout = &tv;
        //     timeout->tv_sec = ms / 1000;
        //     timeout->tv_usec = (ms % 1000) * 1000;
        // }

        // gettimeofday(&curtime, NULL);

        // if ((n = select(loop->nfds, &loop->rfds, NULL, NULL, timeout)) < 0) {
        //     if (errno != EINTR) /* SIGALRM is expected */
        //         printf("select failed\n");
        //     continue;
        // }

        // do {
        //     /*
        //      * If the select timed out, then there's no other
        //      * activity to account for and we don't need to
        //      * call gettimeofday.
        //      */
        //     if (n == 0) {
        //         curtime.tv_sec = lasttime.tv_sec + (ms / 1000);
        //         curtime.tv_usec = lasttime.tv_usec + ((ms % 1000) * 1000);
        //         n = -1; /* don't do this next time through the loop */
        //     } else {
        //         gettimeofday(&curtime, NULL);
        //     }

        //     difftime = timeval_subtract(curtime, lasttime);
        //     diffms = difftime.tv_sec * 1000 + difftime.tv_usec / 1000;
        //     lasttime = curtime;

        //     if (ms == 0 || (diffms) > 0) {
        //         evtimer_callout(loop, diffms);
        //     }

        //     ms = -1;
        // } while (diffms > 0);

        // /* Handle sockets */
        // if (n > 0) {
        //     for (i = 0; i < loop->nios; i++) {
        //         if (FD_ISSET(loop->ios[i].fd, &loop->rfds)) {
        //             // (*loop->ios[i].func)(&(loop->ios[i]));
        //             (*loop->ios[i].func)(loop->ios + i);
        //         }
        //     }
        // }

    } /* Main loop */

    loop->status = EVLOOP_STATUS_STOP;
    loop->end_hrtime = gethrtime_us();

    if (loop->flags & EVLOOP_FLAG_AUTO_FREE) {
        evloop_cleanup(loop);
        EV_FREE(loop);
    }
    return 0;
}

int evloop_stop(evloop_t* loop) {
    loop->status = EVLOOP_STATUS_STOP;
    return 0;
}

uint64_t evloop_next_event_id() {
    static atomic_long s_id = ATOMIC_VAR_INIT(0);
    return ++s_id;
}

void evloop_update_time(evloop_t* loop) {
    loop->cur_hrtime = gethrtime_us();
    if (evloop_now(loop) != time(NULL)) {
        // systemtime changed, we adjust start_ms
        loop->start_ms = gettimeofday_ms() - (loop->cur_hrtime - loop->start_hrtime) / 1000;
    }
}

uint64_t evloop_now(evloop_t* loop) {
    return loop->start_ms / 1000 + (loop->cur_hrtime - loop->start_hrtime) / 1000000;
}

uint64_t evloop_now_ms(evloop_t* loop) {
    return loop->start_ms + (loop->cur_hrtime - loop->start_hrtime) / 1000;
}

uint64_t evloop_now_us(evloop_t* loop) {
    return loop->start_ms * 1000 + (loop->cur_hrtime - loop->start_hrtime);
}

uint64_t evloop_now_hrtime(evloop_t* loop) {
    return loop->cur_hrtime;
}