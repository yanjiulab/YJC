#include "sev.h"

#include <stdio.h>
#include <string.h>
#include <stdatomic.h>

#include "cmd.h"

#define EVLOOP_MAX_BLOCK_TIME 100 // ms

// evidle
#define EVIDLE
evidle_t* evidle_add(evloop_t* loop, evidle_cb cb, uint32_t repeat) {
    evidle_t* idle;
    EV_ALLOC_SIZEOF(idle);
    idle->event_type = EVENT_TYPE_IDLE;
    idle->priority = EVENT_LOWEST_PRIORITY;
    idle->repeat = repeat;
    list_add(&idle->node, &loop->idles);
    EVENT_ADD(loop, idle, cb);
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
    EVENT_DEL(idle);
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
    EVENT_ADD(loop, timer, cb);
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
    EVENT_ADD(loop, timer, cb);
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
    EVENT_RESET(timer);
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
    EVENT_DEL(timer);
}

// evio
#define EVIO

// evloop API
const char* evio_engine() {
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

evio_t* evio_get(evloop_t* loop, int fd) {
    if (fd >= loop->ios.maxsize) {
        int newsize = ceil2e(fd);
        io_array_resize(&loop->ios, newsize > fd ? newsize : 2 * fd);
    }

    evio_t* io = loop->ios.ptr[fd];
    if (io == NULL) {
        EV_ALLOC_SIZEOF(io);
        evio_init(io);
        io->event_type = EVENT_TYPE_IO;
        io->loop = loop;
        io->fd = fd;
        loop->ios.ptr[fd] = io;
    }

    if (!io->ready) {
        evio_ready(io);
    }

    return io;
}

void evio_detach(evio_t* io) {
    evloop_t* loop = io->loop;
    int fd = io->fd;
    assert(loop != NULL && fd < loop->ios.maxsize);
    loop->ios.ptr[fd] = NULL;
}

void evio_attach(evloop_t* loop, evio_t* io) {
    int fd = io->fd;
    if (fd >= loop->ios.maxsize) {
        int newsize = ceil2e(fd);
        io_array_resize(&loop->ios, newsize > fd ? newsize : 2 * fd);
    }

    // NOTE: hio was not freed for reused when closed, but attached hio can't be reused,
    // so we need to free it if fd exists to avoid memory leak.
    evio_t* previo = loop->ios.ptr[fd];
    if (previo != NULL && previo != io) {
        evio_free(previo);
    }

    io->loop = loop;
    // NOTE: use new_loop readbuf
    io->readbuf.base = loop->readbuf.base;
    io->readbuf.len = loop->readbuf.len;
    loop->ios.ptr[fd] = io;
}

bool evio_exists(evloop_t* loop, int fd) {
    if (fd >= loop->ios.maxsize) {
        return false;
    }
    return loop->ios.ptr[fd] != NULL;
}

int evio_add(evio_t* io, evio_cb cb, int events) {
    printd("evio_add fd=%d io->events=%d events=%d\n", io->fd, io->events, events);

    evloop_t* loop = io->loop;
    if (!io->active) {
        EVENT_ADD(loop, io, cb);
        loop->nios++;
    }

    if (!io->ready) {
        evio_ready(io);
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

int evio_del(evio_t* io, int events) {
    printd("evio_del fd=%d io->events=%d events=%d\n", io->fd, io->events, events);

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


uint32_t evio_next_id() {
    static atomic_long s_id = ATOMIC_VAR_INIT(0);
    return ++s_id;
}

// uint64_t evloop_next_event_id() {
//     static atomic_long s_id = ATOMIC_VAR_INIT(0);
//     return ++s_id;
// }


// io with loop






// io itself
void evio_init(evio_t* io) {
    // alloc localaddr,peeraddr when evio_socket_init
    /*
    if (io->localaddr == NULL) {
        EV_ALLOC(io->localaddr, sizeof(sockaddr_u));
    }
    if (io->peeraddr == NULL) {
        EV_ALLOC(io->peeraddr, sizeof(sockaddr_u));
    }
    */

    // write_queue init when hwrite try_write failed
    // write_queue_init(&io->write_queue, 4);

    recursive_mutex_init(&io->write_mutex);
}

void evio_ready(evio_t* io) {
    if (io->ready)
        return;
    // flags
    io->ready = 1;
    //     io->connected = 0;
    //     io->closed = 0;
    //     io->accept = io->connect = io->connectex = 0;
    //     io->recv = io->send = 0;
    //     io->recvfrom = io->sendto = 0;
    //     io->close = 0;
    //     // public:
    //     io->id = evio_next_id();
    //     io->io_type = EVIO_TYPE_UNKNOWN;
    //     io->error = 0;
    //     io->events = io->revents = 0;
    //     io->last_read_hrtime = io->last_write_hrtime = io->loop->cur_hrtime;
    //     // readbuf
    //     io->alloced_readbuf = 0;
    //     io->readbuf.base = io->loop->readbuf.base;
    //     io->readbuf.len = io->loop->readbuf.len;
    //     io->readbuf.head = io->readbuf.tail = 0;
    //     io->read_flags = 0;
    //     io->read_until_length = 0;
    //     io->max_read_bufsize = MAX_READ_BUFSIZE;
    //     io->small_readbytes_cnt = 0;
    //     // write_queue
    //     io->write_bufsize = 0;
    //     io->max_write_bufsize = MAX_WRITE_BUFSIZE;
    //     // callbacks
    //     io->read_cb = NULL;
    //     io->write_cb = NULL;
    //     io->close_cb = NULL;
    //     io->accept_cb = NULL;
    //     io->connect_cb = NULL;
    //     // timers
    //     io->connect_timeout = 0;
    //     io->connect_timer = NULL;
    //     io->close_timeout = 0;
    //     io->close_timer = NULL;
    //     io->read_timeout = 0;
    //     io->read_timer = NULL;
    //     io->write_timeout = 0;
    //     io->write_timer = NULL;
    //     io->keepalive_timeout = 0;
    //     io->keepalive_timer = NULL;
    //     io->heartbeat_interval = 0;
    //     io->heartbeat_fn = NULL;
    //     io->heartbeat_timer = NULL;
    //     // upstream
    //     io->upstream_io = NULL;
    //     // unpack
    //     io->unpack_setting = NULL;
    //     // ssl
    //     io->ssl = NULL;
    //     io->ssl_ctx = NULL;
    //     io->alloced_ssl_ctx = 0;
    //     io->hostname = NULL;
    //     // context
    //     io->ctx = NULL;
    //     // private:
    // #if defined(EVENT_POLL) || defined(EVENT_KQUEUE)
    //     io->event_index[0] = io->event_index[1] = -1;
    // #endif
    // #ifdef EVENT_IOCP
    //     io->hovlp = NULL;
    // #endif

    //     // io_type
    //     fill_io_type(io);
    //     if (io->io_type & EVIO_TYPE_SOCKET) {
    //         evio_socket_init(io);
    //     }
}

void evio_done(evio_t* io) {
    if (!io->ready)
        return;
    io->ready = 0;

    evio_del(io, EV_RDWR);

    // readbuf
    evio_free_readbuf(io);

    // write_queue
    offset_buf_t* pbuf = NULL;
    recursive_mutex_lock(&io->write_mutex);
    while (!write_queue_empty(&io->write_queue)) {
        pbuf = write_queue_front(&io->write_queue);
        EV_FREE(pbuf->base);
        write_queue_pop_front(&io->write_queue);
    }
    write_queue_cleanup(&io->write_queue);
    recursive_mutex_unlock(&io->write_mutex);

#if WITH_RUDP
    if ((io->io_type & EIO_TYPE_SOCK_DGRAM) || (io->io_type & EIO_TYPE_SOCK_RAW)) {
        rudp_cleanup(&io->rudp);
    }
#endif
}

void evio_free(evio_t* io) {
    if (io == NULL)
        return;
    evio_close(io);
    recursive_mutex_destroy(&io->write_mutex);
    EV_FREE(io->localaddr);
    EV_FREE(io->peeraddr);
    EV_FREE(io);
}

bool evio_is_opened(evio_t* io) {
    if (io == NULL)
        return false;
    return io->ready == 1 && io->closed == 0;
}

bool evio_is_connected(evio_t* io) {
    if (io == NULL)
        return false;
    return io->ready == 1 && io->connected == 1 && io->closed == 0;
}

bool evio_is_closed(evio_t* io) {
    if (io == NULL)
        return true;
    return io->ready == 0 && io->closed == 1;
}

uint32_t evio_id(evio_t* io) { return io->id; }

int evio_fd(evio_t* io) { return io->fd; }

evio_type_e evio_type(evio_t* io) { return io->io_type; }

int evio_error(evio_t* io) { return io->error; }

int evio_events(evio_t* io) { return io->events; }

int evio_revents(evio_t* io) { return io->revents; }

struct sockaddr* evio_localaddr(evio_t* io) { return io->localaddr; }

struct sockaddr* evio_peeraddr(evio_t* io) { return io->peeraddr; }

void evio_set_context(evio_t* io, void* ctx) { io->ctx = ctx; }

void* evio_context(evio_t* io) { return io->ctx; }

accept_cb evio_getcb_accept(evio_t* io) { return io->accept_cb; }

connect_cb evio_getcb_connect(evio_t* io) { return io->connect_cb; }

read_cb evio_getcb_read(evio_t* io) { return io->read_cb; }

write_cb evio_getcb_write(evio_t* io) { return io->write_cb; }

close_cb evio_getcb_close(evio_t* io) { return io->close_cb; }

void evio_setcb_accept(evio_t* io, accept_cb accept_cb) { io->accept_cb = accept_cb; }

void evio_setcb_connect(evio_t* io, connect_cb connect_cb) { io->connect_cb = connect_cb; }

void evio_setcb_read(evio_t* io, read_cb read_cb) { io->read_cb = read_cb; }

void evio_setcb_write(evio_t* io, write_cb write_cb) { io->write_cb = write_cb; }

void evio_setcb_close(evio_t* io, close_cb close_cb) { io->close_cb = close_cb; }


//-----------------iobuf---------------------------------------------
void evio_alloc_readbuf(evio_t* io, int len) {
    if (len > io->max_read_bufsize) {
        // ("read bufsize > %u, close it!", io->max_read_bufsize);
        // io->error = ERR_OVER_LIMIT;
        evio_close_async(io);
        return;
    }
    if (evio_is_alloced_readbuf(io)) {
        io->readbuf.base = (char*)ev_zrealloc(io->readbuf.base, len, io->readbuf.len);
    } else {
        EV_ALLOC(io->readbuf.base, len);
    }
    io->readbuf.len = len;
    io->alloced_readbuf = 1;
    io->small_readbytes_cnt = 0;
}

void evio_free_readbuf(evio_t* io) {
    if (evio_is_alloced_readbuf(io)) {
        EV_FREE(io->readbuf.base);
        io->alloced_readbuf = 0;
        // reset to loop->readbuf
        io->readbuf.base = io->loop->readbuf.base;
        io->readbuf.len = io->loop->readbuf.len;
    }
}

void evio_memmove_readbuf(evio_t* io) {
    fifo_buf_t* buf = &io->readbuf;
    if (buf->tail == buf->head) {
        buf->head = buf->tail = 0;
        return;
    }
    if (buf->tail > buf->head) {
        size_t size = buf->tail - buf->head;
        // [head, tail] => [0, tail - head]
        memmove(buf->base, buf->base + buf->head, size);
        buf->head = 0;
        buf->tail = size;
    }
}

void evio_set_readbuf(evio_t* io, void* buf, size_t len) {
    assert(io && buf && len != 0);
    evio_free_readbuf(io);
    io->readbuf.base = (char*)buf;
    io->readbuf.len = len;
    io->readbuf.head = io->readbuf.tail = 0;
    io->alloced_readbuf = 0;
}

evio_readbuf_t* evio_get_readbuf(evio_t* io) { return &io->readbuf; }

void evio_set_max_read_bufsize(evio_t* io, uint32_t size) { io->max_read_bufsize = size; }

void evio_set_max_write_bufsize(evio_t* io, uint32_t size) { io->max_write_bufsize = size; }

size_t evio_write_bufsize(evio_t* io) { return io->write_bufsize; }

// int evio_read(evio_t* io) {
//     if (io->closed) {
//         log_error("evio_read called but fd[%d] already closed!", io->fd);
//         return -1;
//     }
//     evio_add(io, evio_handle_events, EV_READ);
//     if (io->readbuf.tail > io->readbuf.head &&
//         // io->unpack_setting == NULL &&
//         io->read_flags == 0) {
//         evio_read_remain(io);
//     }
//     return 0;
// }

//------------------low-level apis-------------------------------------------

// evio_get -> evio_add
evio_t* ev_read(evloop_t* loop, int fd, /*void* buf, size_t len,*/ evio_cb read_cb) {
    evio_t* io = evio_get(loop, fd);
    assert(io != NULL);
    evio_add(io, read_cb, EV_READ);
    return io;
}

//------------------high-level apis-------------------------------------------
// evio_t* ev_hread(evloop_t* loop, int fd, void* buf, size_t len, evio_cb read_cb) {
//     evio_t* io = evio_get(loop, fd);
//     assert(io != NULL);
//     if (buf && len) {
//         io->readbuf.base = (char*)buf;
//         io->readbuf.len = len;
//     }
//     if (read_cb) {
//         io->read_cb = read_cb;
//     }

//     evio_read(io);
//     return io;
// }

// evio_t* ev_write(evloop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb) {
//     evio_t* io = evio_get(loop, fd);
//     assert(io != NULL);
//     if (write_cb) {
//         io->write_cb = write_cb;
//     }
//     evio_write(io, buf, len);
//     return io;
// }

// evio_t* haccept(evloop_t* loop, int listenfd, accept_cb accept_cb) {
//     evio_t* io = evio_get(loop, listenfd);
//     assert(io != NULL);
//     if (accept_cb) {
//         io->accept_cb = accept_cb;
//     }
//     if (evio_accept(io) != 0)
//         return NULL;
//     return io;
// }

// evio_t* hconnect(evloop_t* loop, int connfd, connect_cb connect_cb) {
//     evio_t* io = evio_get(loop, connfd);
//     assert(io != NULL);
//     if (connect_cb) {
//         io->connect_cb = connect_cb;
//     }
//     if (evio_connect(io) != 0)
//         return NULL;
//     return io;
// }

// void hclose(evloop_t* loop, int fd) {
//     evio_t* io = evio_get(loop, fd);
//     assert(io != NULL);
//     evio_close(io);
// }

// evio_t* hrecv(evloop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb) {
//     // evio_t* io = evio_get(loop, connfd);
//     // assert(io != NULL);
//     // io->recv = 1;
//     // if (io->io_type != EVIO_TYPE_SSL) {
//     // io->io_type = EVIO_TYPE_TCP;
//     //}
//     return hread(loop, connfd, buf, len, read_cb);
// }

// evio_t* hsend(evloop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb) {
//     // evio_t* io = evio_get(loop, connfd);
//     // assert(io != NULL);
//     // io->send = 1;
//     // if (io->io_type != EVIO_TYPE_SSL) {
//     // io->io_type = EVIO_TYPE_TCP;
//     //}
//     return hwrite(loop, connfd, buf, len, write_cb);
// }

// evio_t* hrecvfrom(evloop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb) {
//     // evio_t* io = evio_get(loop, sockfd);
//     // assert(io != NULL);
//     // io->recvfrom = 1;
//     // io->io_type = EVIO_TYPE_UDP;
//     return hread(loop, sockfd, buf, len, read_cb);
// }

// evio_t* hsendto(evloop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb) {
//     // evio_t* io = evio_get(loop, sockfd);
//     // assert(io != NULL);
//     // io->sendto = 1;
//     // io->io_type = EVIO_TYPE_UDP;
//     return hwrite(loop, sockfd, buf, len, write_cb);
// }

//------------------top-level apis-------------------------------------------

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
        EVENT_PENDING(idle);
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
        EVENT_PENDING(timer);
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

static int evloop_process_ios(evloop_t* loop, int timeout) {
    // That is to call IO multiplexing function such as select, poll, epoll, etc.
    int nevents = iowatcher_poll_events(loop, timeout);
    if (nevents < 0) {
        log_debug("poll_events error=%d", -nevents);
    }
    return nevents < 0 ? 0 : nevents;
}

static int evloop_process_pendings(evloop_t* loop) {
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
        nios = evloop_process_ios(loop, blocktime_ms);
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
}

evloop_t* evloop_new(int flags) {
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
    io_array_init(&loop->ios, IO_ARRAY_INIT_SIZE);
    // iowatcher
    iowatcher_init(loop);

    // NOTE: init start_time here, because evtimer_add use it.
    loop->start_ms = gettimeofday_ms();
    loop->start_hrtime = loop->cur_hrtime = gethrtime_us();

    loop->flags |= flags;
    return loop;
}

static void evloop_cleanup(evloop_t* loop) {
    // pendings
    printd("cleanup pendings...\n");
    for (int i = 0; i < EVENT_PRIORITY_SIZE; ++i) {
        loop->pendings[i] = NULL;
    }

    // ios
    printd("cleanup ios...\n");
    for (int i = 0; i < loop->ios.maxsize; ++i) {
        evio_t* io = loop->ios.ptr[i];
        if (io) {
            evio_free(io);
        }
    }
    io_array_cleanup(&loop->ios);

    // idles
    printd("cleanup idles...\n");
    struct list_node* node = loop->idles.next;
    evidle_t* idle;
    while (node != &loop->idles) {
        idle = IDLE_ENTRY(node);
        node = node->next;
        EV_FREE(idle);
    }
    list_init(&loop->idles);

    // timers
    printd("cleanup timers...\n");
    evtimer_t* timer;
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
    // if (loop->readbuf.base && loop->readbuf.len) {
    //     EV_FREE(loop->readbuf.base);
    //     loop->readbuf.base = NULL;
    //     loop->readbuf.len = 0;
    // }

    // iowatcher
    iowatcher_cleanup(loop);

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

    /* Main loop */
    while (loop->status != EVLOOP_STATUS_STOP) {
        ++loop->loop_cnt;
        if ((loop->flags & EVLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS) && loop->nactives == 0) {
            break;
        }
        evloop_process_events(loop);
        if (loop->flags & EVLOOP_FLAG_RUN_ONCE) {
            break;
        }
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

void evloop_set_userdata(evloop_t* loop, void* userdata) {
    loop->userdata = userdata;
}

void* evloop_userdata(evloop_t* loop) {
    return loop->userdata;
}