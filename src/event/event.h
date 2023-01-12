#ifndef EV_EVENT_H_
#define EV_EVENT_H_

#include <pthread.h>

#include "array.h"
#include "buf.h"
#include "eventloop.h"
#include "heap.h"
#include "iowatcher.h"
#include "list.h"
#include "queue.h"

#define ELOOP_READ_BUFSIZE 8192              // 8K
#define READ_BUFSIZE_HIGH_WATER 65536        // 64K
#define WRITE_BUFSIZE_HIGH_WATER (1U << 23)  // 8M
#define MAX_READ_BUFSIZE (1U << 24)          // 16M
#define MAX_WRITE_BUFSIZE (1U << 24)         // 16M

// eio_read_flags
#define EIO_READ_ONCE 0x1
#define EIO_READ_UNTIL_LENGTH 0x2
#define EIO_READ_UNTIL_DELIM 0x4

ARRAY_DECL(eio_t*, io_array);
QUEUE_DECL(event_t, event_queue);

struct eloop_s {
    uint32_t flags;
    eloop_status_e status;
    uint64_t start_ms;      // ms
    uint64_t start_hrtime;  // us
    uint64_t end_hrtime;
    uint64_t cur_hrtime;
    uint64_t loop_cnt;
    long pid;
    long tid;
    void* userdata;
    // private:
    //  events
    uint32_t intern_nevents;
    uint32_t nactives;
    uint32_t npendings;
    // pendings: with priority as array.index
    event_t* pendings[EVENT_PRIORITY_SIZE];
    // idles
    struct list_head idles;
    uint32_t nidles;
    // timers
    struct heap timers;      // monotonic time
    struct heap realtimers;  // realtime
    uint32_t ntimers;
    // ios: with fd as array.index
    struct io_array ios;
    uint32_t nios;
    // one loop per thread, so one readbuf per loop is OK.
    buf_t readbuf;
    void* iowatcher;
    // custom_events
    int eventfds[2];
    event_queue custom_events;
    pthread_mutex_t custom_events_mutex;
};

uint64_t eloop_next_event_id();

struct eidle_s {
    EVENT_FIELDS
    uint32_t repeat;
    // private:
    struct list_node node;
};

#define ETIMER_FIELDS      \
    EVENT_FIELDS           \
    uint32_t repeat;       \
    uint64_t next_timeout; \
    struct heap_node node;

struct etimer_s {
    ETIMER_FIELDS
};

struct etimeout_s {
    ETIMER_FIELDS
    uint32_t timeout;
};

struct eperiod_s {
    ETIMER_FIELDS
    int8_t minute;
    int8_t hour;
    int8_t day;
    int8_t week;
    int8_t month;
};

QUEUE_DECL(offset_buf_t, write_queue);
// sizeof(struct eio_s)=416 on linux-x64
struct eio_s {
    EVENT_FIELDS
    // flags
    unsigned ready : 1;
    unsigned connected : 1;
    unsigned closed : 1;
    unsigned accept : 1;
    unsigned connect : 1;
    unsigned connectex : 1;  // for ConnectEx/DisconnectEx
    unsigned recv : 1;
    unsigned send : 1;
    unsigned recvfrom : 1;
    unsigned sendto : 1;
    unsigned close : 1;
    unsigned alloced_readbuf : 1;  // for eio_alloc_readbuf
    unsigned alloced_ssl_ctx : 1;  // for eio_new_ssl_ctx
                                   // public:
    eio_type_e io_type;
    uint32_t id;  // fd cannot be used as unique identifier, so we provide an id
    int fd;
    int error;
    int events;
    int revents;
    struct sockaddr* localaddr;
    struct sockaddr* peeraddr;
    uint64_t last_read_hrtime;
    uint64_t last_write_hrtime;
    // read
    fifo_buf_t readbuf;
    unsigned int read_flags;
    // for eio_read_until
    union {
        unsigned int read_until_length;
        unsigned char read_until_delim;
    };
    uint32_t max_read_bufsize;
    uint32_t small_readbytes_cnt;  // for readbuf autosize
    // write
    struct write_queue write_queue;
    pthread_mutex_t write_mutex;  // lock write and write_queue
    uint32_t write_bufsize;
    uint32_t max_write_bufsize;
    // callbacks
    read_cb read_cb;
    write_cb write_cb;
    close_cb close_cb;
    accept_cb accept_cb;
    connect_cb connect_cb;
    // timers
    int connect_timeout;     // ms
    int close_timeout;       // ms
    int read_timeout;        // ms
    int write_timeout;       // ms
    int keepalive_timeout;   // ms
    int heartbeat_interval;  // ms
    eio_send_heartbeat_fn heartbeat_fn;
    etimer_t* connect_timer;
    etimer_t* close_timer;
    etimer_t* read_timer;
    etimer_t* write_timer;
    etimer_t* keepalive_timer;
    etimer_t* heartbeat_timer;
    // upstream
    struct eio_s* upstream_io;  // for eio_setup_upstream
    // unpack
    unpack_setting_t* unpack_setting;  // for eio_set_unpack
    // ssl
    void* ssl;       // for eio_set_ssl
    void* ssl_ctx;   // for eio_set_ssl_ctx
    char* hostname;  // for hssl_set_sni_hostname
    // context
    void* ctx;  // for eio_context / eio_set_context
// private:
#if defined(EVENT_POLL) || defined(EVENT_KQUEUE)
    int event_index[2];  // for poll,kqueue
#endif
};
/*
 * hio lifeline:
 *
 * fd =>
 * eio_get => EV_ALLOC_SIZEOF(io) => eio_init => eio_ready
 *
 * eio_read  => eio_add(EV_READ) => eio_read_cb
 * eio_write => eio_add(EV_WRITE) => eio_write_cb
 * eio_close => eio_done => eio_del(EV_RDWR) => eio_close_cb
 *
 * eloop_stop => eloop_free => eio_free => EV_FREE(io)
 */
void eio_init(eio_t* io);
void eio_ready(eio_t* io);
void eio_done(eio_t* io);
void eio_free(eio_t* io);
uint32_t eio_next_id();

void eio_accept_cb(eio_t* io);
void eio_connect_cb(eio_t* io);
void eio_handle_read(eio_t* io, void* buf, int readbytes);
void eio_read_cb(eio_t* io, void* buf, int len);
void eio_write_cb(eio_t* io, const void* buf, int len);
void eio_close_cb(eio_t* io);

void eio_del_connect_timer(eio_t* io);
void eio_del_close_timer(eio_t* io);
void eio_del_read_timer(eio_t* io);
void eio_del_write_timer(eio_t* io);
void eio_del_keepalive_timer(eio_t* io);
void eio_del_heartbeat_timer(eio_t* io);

static inline bool eio_is_loop_readbuf(eio_t* io) { return io->readbuf.base == io->loop->readbuf.base; }
static inline bool eio_is_alloced_readbuf(eio_t* io) { return io->alloced_readbuf; }
void eio_alloc_readbuf(eio_t* io, int len);
void eio_free_readbuf(eio_t* io);
void eio_memmove_readbuf(eio_t* io);

#define EVENT_ENTRY(p) container_of(p, event_t, pending_node)
#define IDLE_ENTRY(p) container_of(p, eidle_t, node)
#define TIMER_ENTRY(p) container_of(p, etimer_t, node)

#define EVENT_ACTIVE(ev)      \
    if (!ev->active) {        \
        ev->active = 1;       \
        ev->loop->nactives++; \
    }

#define EVENT_INACTIVE(ev)    \
    if (ev->active) {         \
        ev->active = 0;       \
        ev->loop->nactives--; \
    }

#define EVENT_PENDING(ev)                                                              \
    do {                                                                               \
        if (!ev->pending) {                                                            \
            ev->pending = 1;                                                           \
            ev->loop->npendings++;                                                     \
            event_t** phead = &ev->loop->pendings[EVENT_PRIORITY_INDEX(ev->priority)]; \
            ev->pending_next = *phead;                                                 \
            *phead = (event_t*)ev;                                                     \
        }                                                                              \
    } while (0)

#define EVENT_ADD(loop, ev, cb)               \
    do {                                      \
        ev->loop = loop;                      \
        ev->event_id = eloop_next_event_id(); \
        ev->cb = (event_cb)cb;                \
        EVENT_ACTIVE(ev);                     \
    } while (0)

#define EVENT_DEL(ev)       \
    do {                    \
        EVENT_INACTIVE(ev); \
        if (!ev->pending) { \
            EV_FREE(ev);    \
        }                   \
    } while (0)

#define EVENT_RESET(ev)   \
    do {                  \
        ev->destroy = 0;  \
        EVENT_ACTIVE(ev); \
        ev->pending = 0;  \
    } while (0)

#endif  // EV_EVENT_H_
