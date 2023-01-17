#include "eventloop.h"

#include "base.h"
#include "defs.h"
#include "event.h"
#include "iowatcher.h"
#include "log.h"
// #include "hmath.h"
// #include "etime.h"
#include "concurrency.h"
#include "sock.h"

#if defined(OS_UNIX) && HAVE_EVENTFD
#include "sys/eventfd.h"
#endif

#define ELOOP_PAUSE_TIME 10       // ms
#define ELOOP_MAX_BLOCK_TIME 100  // ms
#define ELOOP_STAT_TIMEOUT 60000  // ms

#define IO_ARRAY_INIT_SIZE 1024
#define CUSTOM_EVENT_QUEUE_INIT_SIZE 16

#define EVENTFDS_READ_INDEX 0
#define EVENTFDS_WRITE_INDEX 1

static void __eidle_del(eidle_t* idle);
static void __etimer_del(etimer_t* timer);

static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return TIMER_ENTRY(lhs)->next_timeout < TIMER_ENTRY(rhs)->next_timeout;
}

static int eloop_process_idles(eloop_t* loop) {
    int nidles = 0;
    struct list_node* node = loop->idles.next;
    eidle_t* idle = NULL;
    while (node != &loop->idles) {
        idle = IDLE_ENTRY(node);
        node = node->next;
        if (idle->repeat != INFINITE) {
            --idle->repeat;
        }
        if (idle->repeat == 0) {
            // NOTE: Just mark it as destroy and remove from list.
            // Real deletion occurs after eloop_process_pendings.
            __eidle_del(idle);
        }
        EVENT_PENDING(idle);
        ++nidles;
    }
    return nidles;
}

static int __eloop_process_timers(struct heap* timers, uint64_t timeout) {
    int ntimers = 0;
    etimer_t* timer = NULL;
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
            // Real deletion occurs after eloop_process_pendings.
            __etimer_del(timer);
        } else {
            // NOTE: calc next timeout, then re-insert heap.
            heap_dequeue(timers);
            if (timer->event_type == EVENT_TYPE_TIMEOUT) {
                while (timer->next_timeout <= timeout) {
                    timer->next_timeout += (uint64_t)((etimeout_t*)timer)->timeout * 1000;
                }
            } else if (timer->event_type == EVENT_TYPE_PERIOD) {
                eperiod_t* period = (eperiod_t*)timer;
                timer->next_timeout = (uint64_t)cron_next_timeout(period->minute, period->hour, period->day,
                                                                  period->week, period->month) *
                                      1000000;
            }
            heap_insert(timers, &timer->node);
        }
        EVENT_PENDING(timer);
        ++ntimers;
    }
    return ntimers;
}

static int eloop_process_timers(eloop_t* loop) {
    uint64_t now = eloop_now_us(loop);
    int ntimers = __eloop_process_timers(&loop->timers, loop->cur_hrtime);
    ntimers += __eloop_process_timers(&loop->realtimers, now);
    return ntimers;
}

static int eloop_process_ios(eloop_t* loop, int timeout) {
    // That is to call IO multiplexing function such as select, poll, epoll, etc.
    int nevents = iowatcher_poll_events(loop, timeout);
    if (nevents < 0) {
        log_debug("poll_events error=%d", -nevents);
    }
    return nevents < 0 ? 0 : nevents;
}

static int eloop_process_pendings(eloop_t* loop) {
    if (loop->npendings == 0)
        return 0;

    event_t* cur = NULL;
    event_t* next = NULL;
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
                    EVENT_DEL(cur);
                }
            }
            cur = next;
        }
        loop->pendings[i] = NULL;
    }
    loop->npendings = 0;
    return ncbs;
}

// eloop_process_ios -> eloop_process_timers -> eloop_process_idles -> eloop_process_pendings
static int eloop_process_events(eloop_t* loop) {
    // ios -> timers -> idles
    int nios, ntimers, nidles;
    nios = ntimers = nidles = 0;

    // calc blocktime
    int32_t blocktime_ms = ELOOP_MAX_BLOCK_TIME;
    if (loop->ntimers) {
        eloop_update_time(loop);
        int64_t blocktime_us = blocktime_ms * 1000;
        if (loop->timers.root) {
            int64_t min_timeout = TIMER_ENTRY(loop->timers.root)->next_timeout - loop->cur_hrtime;
            blocktime_us = MIN(blocktime_us, min_timeout);
        }
        if (loop->realtimers.root) {
            int64_t min_timeout = TIMER_ENTRY(loop->realtimers.root)->next_timeout - eloop_now_us(loop);
            blocktime_us = MIN(blocktime_us, min_timeout);
        }
        if (blocktime_us <= 0)
            goto process_timers;
        blocktime_ms = blocktime_us / 1000 + 1;
        blocktime_ms = MIN(blocktime_ms, ELOOP_MAX_BLOCK_TIME);
    }

    if (loop->nios) {
        nios = eloop_process_ios(loop, blocktime_ms);
    } else {
        ev_msleep(blocktime_ms);
    }
    eloop_update_time(loop);
    // wakeup by eloop_stop
    if (loop->status == ELOOP_STATUS_STOP) {
        return 0;
    }

process_timers:
    if (loop->ntimers) {
        ntimers = eloop_process_timers(loop);
    }

    int npendings = loop->npendings;
    if (npendings == 0) {
        if (loop->nidles) {
            nidles = eloop_process_idles(loop);
        }
    }
    int ncbs = eloop_process_pendings(loop);
    // printd("blocktime=%d nios=%d/%u ntimers=%d/%u nidles=%d/%u nactives=%d npendings=%d ncbs=%d\n",
    //         blocktime, nios, loop->nios, ntimers, loop->ntimers, nidles, loop->nidles,
    //         loop->nactives, npendings, ncbs);
    return ncbs;
}

static void eloop_stat_timer_cb(etimer_t* timer) {
    eloop_t* loop = timer->loop;
    // hlog_set_level(LOG_LEVEL_DEBUG);
    log_debug("[loop] pid=%ld tid=%ld uptime=%lluus cnt=%llu nactives=%u nios=%u ntimers=%u nidles=%u", loop->pid,
          loop->tid, loop->cur_hrtime - loop->start_hrtime, loop->loop_cnt, loop->nactives, loop->nios, loop->ntimers,
          loop->nidles);
}

static void eventfd_read_cb(eio_t* io, void* buf, int readbytes) {
    eloop_t* loop = io->loop;
    event_t* pev = NULL;
    event_t ev;
    uint64_t count = readbytes;
#if defined(OS_UNIX) && HAVE_EVENTFD
    assert(readbytes == sizeof(count));
    count = *(uint64_t*)buf;
#endif
    for (uint64_t i = 0; i < count; ++i) {
        hmutex_lock(&loop->custom_events_mutex);
        if (event_queue_empty(&loop->custom_events)) {
            goto unlock;
        }
        pev = event_queue_front(&loop->custom_events);
        if (pev == NULL) {
            goto unlock;
        }
        ev = *pev;
        event_queue_pop_front(&loop->custom_events);
        // NOTE: unlock before cb, avoid deadlock if eloop_post_event called in cb.
        hmutex_unlock(&loop->custom_events_mutex);
        if (ev.cb) {
            ev.cb(&ev);
        }
    }
    return;
unlock:
    hmutex_unlock(&loop->custom_events_mutex);
}

static int eloop_create_eventfds(eloop_t* loop) {
#if defined(OS_UNIX) && HAVE_EVENTFD
    int efd = eventfd(0, 0);
    if (efd < 0) {
        log_error("eventfd create failed!");
        return -1;
    }
    loop->eventfds[0] = loop->eventfds[1] = efd;
#elif defined(OS_UNIX) && HAVE_PIPE
    if (pipe(loop->eventfds) != 0) {
        log_error("pipe create failed!");
        return -1;
    }
#else
    if (Socketpair(AF_INET, SOCK_STREAM, 0, loop->eventfds) != 0) {
        log_error("socketpair create failed!");
        return -1;
    }
#endif
    eio_t* io =
        hread(loop, loop->eventfds[EVENTFDS_READ_INDEX], loop->readbuf.base, loop->readbuf.len, eventfd_read_cb);
    io->priority = EVENT_HIGH_PRIORITY;
    ++loop->intern_nevents;
    return 0;
}

static void eloop_destroy_eventfds(eloop_t* loop) {
#if defined(OS_UNIX) && HAVE_EVENTFD
    // NOTE: eventfd has only one fd
    SAFE_CLOSE(loop->eventfds[0]);
#elif defined(OS_UNIX) && HAVE_PIPE
    SAFE_CLOSE(loop->eventfds[0]);
    SAFE_CLOSE(loop->eventfds[1]);
#else
    // NOTE: Avoid duplication closesocket in eio_cleanup
    // SAFE_CLOSESOCKET(loop->eventfds[EVENTFDS_READ_INDEX]);
    SAFE_CLOSESOCKET(loop->eventfds[EVENTFDS_WRITE_INDEX]);
#endif
    loop->eventfds[0] = loop->eventfds[1] = -1;
}

void eloop_post_event(eloop_t* loop, event_t* ev) {
    if (ev->loop == NULL) {
        ev->loop = loop;
    }
    if (ev->event_type == 0) {
        ev->event_type = EVENT_TYPE_CUSTOM;
    }
    if (ev->event_id == 0) {
        ev->event_id = eloop_next_event_id();
    }

    int nwrite = 0;
    uint64_t count = 1;
    hmutex_lock(&loop->custom_events_mutex);
    if (loop->eventfds[EVENTFDS_WRITE_INDEX] == -1) {
        if (eloop_create_eventfds(loop) != 0) {
            goto unlock;
        }
    }
#if defined(OS_UNIX) && HAVE_EVENTFD
    nwrite = write(loop->eventfds[EVENTFDS_WRITE_INDEX], &count, sizeof(count));
#elif defined(OS_UNIX) && HAVE_PIPE
    nwrite = write(loop->eventfds[EVENTFDS_WRITE_INDEX], "e", 1);
#else
    nwrite = send(loop->eventfds[EVENTFDS_WRITE_INDEX], "e", 1, 0);
#endif
    if (nwrite <= 0) {
        log_error("eloop_post_event failed!");
        goto unlock;
    }
    event_queue_push_back(&loop->custom_events, ev);
unlock:
    hmutex_unlock(&loop->custom_events_mutex);
}

static void eloop_init(eloop_t* loop) {
#ifdef OS_WIN
    WSAInit();
#endif
#ifdef SIGPIPE
    // NOTE: if not ignore SIGPIPE, write twice when peer close will lead to exit process by SIGPIPE.
    signal(SIGPIPE, SIG_IGN);
#endif

    loop->status = ELOOP_STATUS_STOP;
    loop->pid = ev_getpid();
    loop->tid = gettid();

    // idles
    list_init(&loop->idles);

    // timers
    heap_init(&loop->timers, timers_compare);
    heap_init(&loop->realtimers, timers_compare);

    // ios
    io_array_init(&loop->ios, IO_ARRAY_INIT_SIZE);

    // readbuf
    loop->readbuf.len = ELOOP_READ_BUFSIZE;
    EV_ALLOC(loop->readbuf.base, loop->readbuf.len);

    // iowatcher
    iowatcher_init(loop);

    // custom_events
    hmutex_init(&loop->custom_events_mutex);
    event_queue_init(&loop->custom_events, CUSTOM_EVENT_QUEUE_INIT_SIZE);
    // NOTE: eloop_create_eventfds when eloop_post_event or eloop_run
    loop->eventfds[0] = loop->eventfds[1] = -1;

    // NOTE: init start_time here, because etimer_add use it.
    loop->start_ms = gettimeofday_ms();
    loop->start_hrtime = loop->cur_hrtime = gethrtime_us();
}

static void eloop_cleanup(eloop_t* loop) {
    // pendings
    printd("cleanup pendings...\n");
    for (int i = 0; i < EVENT_PRIORITY_SIZE; ++i) {
        loop->pendings[i] = NULL;
    }

    // ios
    printd("cleanup ios...\n");
    for (int i = 0; i < loop->ios.maxsize; ++i) {
        eio_t* io = loop->ios.ptr[i];
        if (io) {
            eio_free(io);
        }
    }
    io_array_cleanup(&loop->ios);

    // idles
    printd("cleanup idles...\n");
    struct list_node* node = loop->idles.next;
    eidle_t* idle;
    while (node != &loop->idles) {
        idle = IDLE_ENTRY(node);
        node = node->next;
        EV_FREE(idle);
    }
    list_init(&loop->idles);

    // timers
    printd("cleanup timers...\n");
    etimer_t* timer;
    while (loop->timers.root) {
        timer = TIMER_ENTRY(loop->timers.root);
        heap_dequeue(&loop->timers);
        EV_FREE(timer);
    }
    heap_init(&loop->timers, NULL);
    while (loop->realtimers.root) {
        timer = TIMER_ENTRY(loop->realtimers.root);
        heap_dequeue(&loop->realtimers);
        EV_FREE(timer);
    }
    heap_init(&loop->realtimers, NULL);

    // readbuf
    if (loop->readbuf.base && loop->readbuf.len) {
        EV_FREE(loop->readbuf.base);
        loop->readbuf.base = NULL;
        loop->readbuf.len = 0;
    }

    // iowatcher
    iowatcher_cleanup(loop);

    // custom_events
    hmutex_lock(&loop->custom_events_mutex);
    eloop_destroy_eventfds(loop);
    event_queue_cleanup(&loop->custom_events);
    hmutex_unlock(&loop->custom_events_mutex);
    hmutex_destroy(&loop->custom_events_mutex);
}

eloop_t* eloop_new(int flags) {
    eloop_t* loop;
    EV_ALLOC_SIZEOF(loop);
    eloop_init(loop);
    loop->flags |= flags;
    return loop;
}

void eloop_free(eloop_t** pp) {
    if (pp && *pp) {
        eloop_cleanup(*pp);
        EV_FREE(*pp);
        *pp = NULL;
    }
}

// while (loop->status) { eloop_process_events(loop); }
int eloop_run(eloop_t* loop) {
    if (loop == NULL)
        return -1;
    if (loop->status == ELOOP_STATUS_RUNNING)
        return -2;

    loop->status = ELOOP_STATUS_RUNNING;
    loop->pid = ev_getpid();
    loop->tid = gettid();

    if (loop->intern_nevents == 0) {
        hmutex_lock(&loop->custom_events_mutex);
        if (loop->eventfds[EVENTFDS_WRITE_INDEX] == -1) {
            eloop_create_eventfds(loop);
        }
        hmutex_unlock(&loop->custom_events_mutex);

#ifdef DEBUG
        etimer_add(loop, eloop_stat_timer_cb, ELOOP_STAT_TIMEOUT, INFINITE);
        ++loop->intern_nevents;
#endif
    }

    while (loop->status != ELOOP_STATUS_STOP) {
        if (loop->status == ELOOP_STATUS_PAUSE) {
            ev_msleep(ELOOP_PAUSE_TIME);
            eloop_update_time(loop);
            continue;
        }
        ++loop->loop_cnt;
        if ((loop->flags & ELOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS) && loop->nactives <= loop->intern_nevents) {
            break;
        }
        eloop_process_events(loop);
        if (loop->flags & ELOOP_FLAG_RUN_ONCE) {
            break;
        }
    }

    loop->status = ELOOP_STATUS_STOP;
    loop->end_hrtime = gethrtime_us();

    if (loop->flags & ELOOP_FLAG_AUTO_FREE) {
        eloop_cleanup(loop);
        EV_FREE(loop);
    }
    return 0;
}

int eloop_wakeup(eloop_t* loop) {
    event_t ev;
    memset(&ev, 0, sizeof(ev));
    eloop_post_event(loop, &ev);
    return 0;
}

int eloop_stop(eloop_t* loop) {
    if (gettid() != loop->tid) {
        eloop_wakeup(loop);
    }
    loop->status = ELOOP_STATUS_STOP;
    return 0;
}

int eloop_pause(eloop_t* loop) {
    if (loop->status == ELOOP_STATUS_RUNNING) {
        loop->status = ELOOP_STATUS_PAUSE;
    }
    return 0;
}

int eloop_resume(eloop_t* loop) {
    if (loop->status == ELOOP_STATUS_PAUSE) {
        loop->status = ELOOP_STATUS_RUNNING;
    }
    return 0;
}

eloop_status_e eloop_status(eloop_t* loop) { return loop->status; }

void eloop_update_time(eloop_t* loop) {
    loop->cur_hrtime = gethrtime_us();
    if (eloop_now(loop) != time(NULL)) {
        // systemtime changed, we adjust start_ms
        loop->start_ms = gettimeofday_ms() - (loop->cur_hrtime - loop->start_hrtime) / 1000;
    }
}

uint64_t eloop_now(eloop_t* loop) { return loop->start_ms / 1000 + (loop->cur_hrtime - loop->start_hrtime) / 1000000; }

uint64_t eloop_now_ms(eloop_t* loop) { return loop->start_ms + (loop->cur_hrtime - loop->start_hrtime) / 1000; }

uint64_t eloop_now_us(eloop_t* loop) { return loop->start_ms * 1000 + (loop->cur_hrtime - loop->start_hrtime); }

uint64_t eloop_now_hrtime(eloop_t* loop) { return loop->cur_hrtime; }

uint64_t eio_last_read_time(eio_t* io) {
    eloop_t* loop = io->loop;
    return loop->start_ms + (io->last_read_hrtime - loop->start_hrtime) / 1000;
}

uint64_t eio_last_write_time(eio_t* io) {
    eloop_t* loop = io->loop;
    return loop->start_ms + (io->last_write_hrtime - loop->start_hrtime) / 1000;
}

long eloop_pid(eloop_t* loop) { return loop->pid; }

long eloop_tid(eloop_t* loop) { return loop->tid; }

uint64_t eloop_count(eloop_t* loop) { return loop->loop_cnt; }

uint32_t eloop_nios(eloop_t* loop) { return loop->nios; }

uint32_t eloop_ntimers(eloop_t* loop) { return loop->ntimers; }

uint32_t eloop_nidles(eloop_t* loop) { return loop->nidles; }

uint32_t eloop_nactives(eloop_t* loop) { return loop->nactives; }

void eloop_set_userdata(eloop_t* loop, void* userdata) { loop->userdata = userdata; }

void* eloop_userdata(eloop_t* loop) { return loop->userdata; }

eidle_t* eidle_add(eloop_t* loop, eidle_cb cb, uint32_t repeat) {
    eidle_t* idle;
    EV_ALLOC_SIZEOF(idle);
    idle->event_type = EVENT_TYPE_IDLE;
    idle->priority = EVENT_LOWEST_PRIORITY;
    idle->repeat = repeat;
    list_add(&idle->node, &loop->idles);
    EVENT_ADD(loop, idle, cb);
    loop->nidles++;
    return idle;
}

static void __eidle_del(eidle_t* idle) {
    if (idle->destroy)
        return;
    idle->destroy = 1;
    list_del(&idle->node);
    idle->loop->nidles--;
}

void eidle_del(eidle_t* idle) {
    if (!idle->active)
        return;
    __eidle_del(idle);
    EVENT_DEL(idle);
}

etimer_t* etimer_add(eloop_t* loop, etimer_cb cb, uint32_t timeout_ms, uint32_t repeat) {
    if (timeout_ms == 0)
        return NULL;
    etimeout_t* timer;
    EV_ALLOC_SIZEOF(timer);
    timer->event_type = EVENT_TYPE_TIMEOUT;
    timer->priority = EVENT_HIGHEST_PRIORITY;
    timer->repeat = repeat;
    timer->timeout = timeout_ms;
    eloop_update_time(loop);
    timer->next_timeout = loop->cur_hrtime + (uint64_t)timeout_ms * 1000;
    // NOTE: Limit granularity to 100ms
    if (timeout_ms >= 1000 && timeout_ms % 100 == 0) {
        timer->next_timeout = timer->next_timeout / 100000 * 100000;
    }
    heap_insert(&loop->timers, &timer->node);
    EVENT_ADD(loop, timer, cb);
    loop->ntimers++;
    return (etimer_t*)timer;
}

void etimer_reset(etimer_t* timer, uint32_t timeout_ms) {
    if (timer->event_type != EVENT_TYPE_TIMEOUT) {
        return;
    }
    eloop_t* loop = timer->loop;
    etimeout_t* timeout = (etimeout_t*)timer;
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
    EVENT_RESET(timer);
}

etimer_t* etimer_add_period(eloop_t* loop, etimer_cb cb, int8_t minute, int8_t hour, int8_t day, int8_t week,
                            int8_t month, uint32_t repeat) {
    if (minute > 59 || hour > 23 || day > 31 || week > 6 || month > 12) {
        return NULL;
    }
    eperiod_t* timer;
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
    EVENT_ADD(loop, timer, cb);
    loop->ntimers++;
    return (etimer_t*)timer;
}

static void __etimer_del(etimer_t* timer) {
    if (timer->destroy)
        return;
    if (timer->event_type == EVENT_TYPE_TIMEOUT) {
        heap_remove(&timer->loop->timers, &timer->node);
    } else if (timer->event_type == EVENT_TYPE_PERIOD) {
        heap_remove(&timer->loop->realtimers, &timer->node);
    }
    timer->loop->ntimers--;
    timer->destroy = 1;
}

void etimer_del(etimer_t* timer) {
    if (!timer->active)
        return;
    __etimer_del(timer);
    EVENT_DEL(timer);
}

const char* eio_engine() {
#ifdef EVENT_SELECT
    return "select";
#elif defined(EVENT_POLL)
    return "poll";
#elif defined(EVENT_EPOLL)
    return "epoll";
#elif defined(EVENT_KQUEUE)
    return "kqueue";
#elif defined(EVENT_IOCP)
    return "iocp";
#elif defined(EVENT_PORT)
    return "evport";
#else
    return "noevent";
#endif
}

eio_t* eio_get(eloop_t* loop, int fd) {
    if (fd >= loop->ios.maxsize) {
        int newsize = ceil2e(fd);
        io_array_resize(&loop->ios, newsize > fd ? newsize : 2 * fd);
    }

    eio_t* io = loop->ios.ptr[fd];
    if (io == NULL) {
        EV_ALLOC_SIZEOF(io);
        eio_init(io);
        io->event_type = EVENT_TYPE_IO;
        io->loop = loop;
        io->fd = fd;
        loop->ios.ptr[fd] = io;
    }

    if (!io->ready) {
        eio_ready(io);
    }

    return io;
}

void eio_detach(eio_t* io) {
    eloop_t* loop = io->loop;
    int fd = io->fd;
    assert(loop != NULL && fd < loop->ios.maxsize);
    loop->ios.ptr[fd] = NULL;
}

void eio_attach(eloop_t* loop, eio_t* io) {
    int fd = io->fd;
    if (fd >= loop->ios.maxsize) {
        int newsize = ceil2e(fd);
        io_array_resize(&loop->ios, newsize > fd ? newsize : 2 * fd);
    }

    // NOTE: hio was not freed for reused when closed, but attached hio can't be reused,
    // so we need to free it if fd exists to avoid memory leak.
    eio_t* preio = loop->ios.ptr[fd];
    if (preio != NULL && preio != io) {
        eio_free(preio);
    }

    io->loop = loop;
    // NOTE: use new_loop readbuf
    io->readbuf.base = loop->readbuf.base;
    io->readbuf.len = loop->readbuf.len;
    loop->ios.ptr[fd] = io;
}

bool eio_exists(eloop_t* loop, int fd) {
    if (fd >= loop->ios.maxsize) {
        return false;
    }
    return loop->ios.ptr[fd] != NULL;
}

int eio_add(eio_t* io, eio_cb cb, int events) {
    printd("eio_add fd=%d io->events=%d events=%d\n", io->fd, io->events, events);

    eloop_t* loop = io->loop;
    if (!io->active) {
        EVENT_ADD(loop, io, cb);
        loop->nios++;
    }

    if (!io->ready) {
        eio_ready(io);
    }

    if (cb) {
        io->cb = (event_cb)cb;
    }

    if (!(io->events & events)) {
        iowatcher_add_event(loop, io->fd, events);
        io->events |= events;
    }
    return 0;
}

int eio_del(eio_t* io, int events) {
    printd("eio_del fd=%d io->events=%d events=%d\n", io->fd, io->events, events);

    if (!io->active)
        return -1;

    if (io->events & events) {
        iowatcher_del_event(io->loop, io->fd, events);
        io->events &= ~events;
    }
    if (io->events == 0) {
        io->loop->nios--;
        // NOTE: not EVENT_DEL, avoid free
        EVENT_INACTIVE(io);
    }
    return 0;
}

static void eio_close_event_cb(event_t* ev) {
    eio_t* io = (eio_t*)ev->userdata;
    uint32_t id = (uintptr_t)ev->privdata;
    if (io->id != id)
        return;
    eio_close(io);
}

int eio_close_async(eio_t* io) {
    event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.cb = eio_close_event_cb;
    ev.userdata = io;
    ev.privdata = (void*)(uintptr_t)io->id;
    eloop_post_event(io->loop, &ev);
    return 0;
}

//------------------high-level apis-------------------------------------------
eio_t* hread(eloop_t* loop, int fd, void* buf, size_t len, read_cb read_cb) {
    eio_t* io = eio_get(loop, fd);
    assert(io != NULL);
    if (buf && len) {
        io->readbuf.base = (char*)buf;
        io->readbuf.len = len;
    }
    if (read_cb) {
        io->read_cb = read_cb;
    }
    eio_read(io);
    return io;
}

eio_t* hwrite(eloop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb) {
    eio_t* io = eio_get(loop, fd);
    assert(io != NULL);
    if (write_cb) {
        io->write_cb = write_cb;
    }
    eio_write(io, buf, len);
    return io;
}

eio_t* haccept(eloop_t* loop, int listenfd, accept_cb accept_cb) {
    eio_t* io = eio_get(loop, listenfd);
    assert(io != NULL);
    if (accept_cb) {
        io->accept_cb = accept_cb;
    }
    if (eio_accept(io) != 0)
        return NULL;
    return io;
}

eio_t* hconnect(eloop_t* loop, int connfd, connect_cb connect_cb) {
    eio_t* io = eio_get(loop, connfd);
    assert(io != NULL);
    if (connect_cb) {
        io->connect_cb = connect_cb;
    }
    if (eio_connect(io) != 0)
        return NULL;
    return io;
}

void hclose(eloop_t* loop, int fd) {
    eio_t* io = eio_get(loop, fd);
    assert(io != NULL);
    eio_close(io);
}

eio_t* hrecv(eloop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb) {
    // eio_t* io = eio_get(loop, connfd);
    // assert(io != NULL);
    // io->recv = 1;
    // if (io->io_type != EIO_TYPE_SSL) {
    // io->io_type = EIO_TYPE_TCP;
    //}
    return hread(loop, connfd, buf, len, read_cb);
}

eio_t* hsend(eloop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb) {
    // eio_t* io = eio_get(loop, connfd);
    // assert(io != NULL);
    // io->send = 1;
    // if (io->io_type != EIO_TYPE_SSL) {
    // io->io_type = EIO_TYPE_TCP;
    //}
    return hwrite(loop, connfd, buf, len, write_cb);
}

eio_t* hrecvfrom(eloop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb) {
    // eio_t* io = eio_get(loop, sockfd);
    // assert(io != NULL);
    // io->recvfrom = 1;
    // io->io_type = EIO_TYPE_UDP;
    return hread(loop, sockfd, buf, len, read_cb);
}

eio_t* hsendto(eloop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb) {
    // eio_t* io = eio_get(loop, sockfd);
    // assert(io != NULL);
    // io->sendto = 1;
    // io->io_type = EIO_TYPE_UDP;
    return hwrite(loop, sockfd, buf, len, write_cb);
}

//-----------------top-level apis---------------------------------------------
eio_t* eio_create_socket(eloop_t* loop, const char* host, int port, eio_type_e type, eio_side_e side) {
    int sock_type = type & EIO_TYPE_SOCK_STREAM  ? SOCK_STREAM
                    : type & EIO_TYPE_SOCK_DGRAM ? SOCK_DGRAM
                    : type & EIO_TYPE_SOCK_RAW   ? SOCK_RAW
                                                 : -1;
    if (sock_type == -1)
        return NULL;
    sockaddr_u addr;
    memset(&addr, 0, sizeof(addr));
    int ret = -1;
#ifdef ENABLE_UDS
    if (port < 0) {
        sockaddr_set_path(&addr, host);
        ret = 0;
    }
#endif
    if (port >= 0) {
        ret = sockaddr_set_ipport(&addr, host, port);
    }
    if (ret != 0) {
        // fprintf(stderr, "unknown host: %s\n", host);
        return NULL;
    }
    int sockfd = socket(addr.sa.sa_family, sock_type, 0);
    if (sockfd < 0) {
        perror("socket");
        return NULL;
    }
    eio_t* io = NULL;
    if (side == EIO_SERVER_SIDE) {
#ifdef OS_UNIX
        so_reuseaddr(sockfd, 1);
        // so_reuseport(sockfd, 1);
#endif
        if (bind(sockfd, &addr.sa, sockaddr_len(&addr)) < 0) {
            perror("bind");
            closesocket(sockfd);
            return NULL;
        }
        if (sock_type == SOCK_STREAM) {
            if (listen(sockfd, SOMAXCONN) < 0) {
                perror("listen");
                closesocket(sockfd);
                return NULL;
            }
        }
    }
    io = eio_get(loop, sockfd);
    assert(io != NULL);
    io->io_type = type;
    if (side == EIO_SERVER_SIDE) {
        eio_set_localaddr(io, &addr.sa, sockaddr_len(&addr));
        io->priority = EVENT_HIGH_PRIORITY;
    } else {
        eio_set_peeraddr(io, &addr.sa, sockaddr_len(&addr));
    }
    return io;
}

eio_t* eloop_create_tcp_server(eloop_t* loop, const char* host, int port, accept_cb accept_cb) {
    eio_t* io = eio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_SERVER_SIDE);
    if (io == NULL)
        return NULL;
    eio_setcb_accept(io, accept_cb);
    if (eio_accept(io) != 0)
        return NULL;
    return io;
}

eio_t* eloop_create_tcp_client(eloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb) {
    eio_t* io = eio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_CLIENT_SIDE);
    if (io == NULL)
        return NULL;
    eio_setcb_connect(io, connect_cb);
    eio_setcb_close(io, close_cb);
    if (eio_connect(io) != 0)
        return NULL;
    return io;
}

eio_t* eloop_create_ssl_server(eloop_t* loop, const char* host, int port, accept_cb accept_cb) {
    eio_t* io = eio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_SERVER_SIDE);
    if (io == NULL)
        return NULL;
    eio_setcb_accept(io, accept_cb);
    if (eio_accept(io) != 0)
        return NULL;
    return io;
}

eio_t* eloop_create_ssl_client(eloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb) {
    eio_t* io = eio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_CLIENT_SIDE);
    if (io == NULL)
        return NULL;
    eio_setcb_connect(io, connect_cb);
    eio_setcb_close(io, close_cb);
    if (eio_connect(io) != 0)
        return NULL;
    return io;
}

eio_t* eloop_create_udp_server(eloop_t* loop, const char* host, int port) {
    return eio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_SERVER_SIDE);
}

eio_t* eloop_create_udp_client(eloop_t* loop, const char* host, int port) {
    return eio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_CLIENT_SIDE);
}
