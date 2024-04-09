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
#include "array.h"
#include "queue.h"
#include "sockunion.h"
#include "socket.h"

#define NHANDLERS                16
#define TIMER_SECOND_MICRO       1000000L

#define IO_ARRAY_INIT_SIZE       1024
#define SEV_SELECT               0
#define SEV_POLL                 1
#define SEV_EPOLL                2

#define EVIO_READ_ONCE           0x1
#define EVIO_READ_UNTIL_LENGTH   0x2
#define EVIO_READ_UNTIL_DELIM    0x4

#define EVLOOP_READ_BUFSIZE      8192       // 8K
#define READ_BUFSIZE_HIGH_WATER  65536      // 64K
#define WRITE_BUFSIZE_HIGH_WATER (1U << 23) // 8M
#define MAX_READ_BUFSIZE         (1U << 24) // 16M
#define MAX_WRITE_BUFSIZE        (1U << 24) // 16M

// iobuf
typedef struct buf_s {
    char* base;
    size_t len;
} buf_t;

typedef struct offset_buf_s {
    char* base;
    size_t len;
    size_t offset;
} offset_buf_t;

typedef struct fifo_buf_s {
    char* base;
    size_t len;
    size_t head;
    size_t tail;
} fifo_buf_t;

typedef struct fifo_buf_s evio_readbuf_t;

// evio
typedef enum {
    EVIO_TYPE_UNKNOWN = 0,
    EVIO_TYPE_STDIN = 0x00000001,
    EVIO_TYPE_STDOUT = 0x00000002,
    EVIO_TYPE_STDERR = 0x00000004,
    EVIO_TYPE_STDIO = 0x0000000F,

    EVIO_TYPE_FILE = 0x00000010,

    EVIO_TYPE_IP = 0x00000100,
    EVIO_TYPE_SOCK_RAW = 0x00000F00,

    EVIO_TYPE_UDP = 0x00001000,
    EVIO_TYPE_KCP = 0x00002000,
    EVIO_TYPE_DTLS = 0x00010000,
    EVIO_TYPE_SOCK_DGRAM = 0x000FF000,

    EVIO_TYPE_TCP = 0x00100000,
    EVIO_TYPE_SSL = 0x01000000,
    EVIO_TYPE_TLS = EVIO_TYPE_SSL,
    EVIO_TYPE_SOCK_STREAM = 0x0FF00000,

    EVIO_TYPE_SOCKET = 0x0FFFFF00,
} evio_type_e;

typedef enum {
    EVIO_SERVER_SIDE = 0,
    EVIO_CLIENT_SIDE = 1,
} evio_side_e;

#define EVIO_DEFAULT_CONNECT_TIMEOUT    10000 // ms
#define EVIO_DEFAULT_CLOSE_TIMEOUT      60000 // ms
#define EVIO_DEFAULT_KEEPALIVE_TIMEOUT  75000 // ms
#define EVIO_DEFAULT_HEARTBEAT_INTERVAL 10000 // ms
// #define evloop(ev)         (((event_t*)(ev))->loop)

// typedef int EV_RETURN;
typedef struct evloop evloop_t;
typedef struct event event_t;
// NOTE: The following structures are subclasses of event_t,
// inheriting event_t data members and function members.
typedef struct evidle evidle_t;
typedef struct evtimer evtimer_t;
typedef struct evtimeout evtimeout_t;
typedef struct evperiod evperiod_t;
typedef struct evio evio_t;

typedef void (*idle_cb)(event_t*);
typedef void (*event_cb)(event_t*);
typedef void (*evtimer_cb)(evtimer_t*);
typedef void (*evidle_cb)(evidle_t*);
typedef void (*evio_cb)(evio_t*);

typedef void (*accept_cb)(evio_t* io);
typedef void (*connect_cb)(evio_t* io);
typedef void (*read_cb)(evio_t* io, void* buf, int readbytes);
typedef void (*write_cb)(evio_t* io, const void* buf, int writebytes);
typedef void (*close_cb)(evio_t* io);

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

ARRAY_DECL(evio_t*, io_array);

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
    uint32_t intern_nevents;
    uint32_t nactives;
    uint32_t npendings;
    // pendings: with priority as array.index
    event_t* pendings[EVENT_PRIORITY_SIZE];
    // idles
    struct list_head idles;
    uint32_t nidles;
    // timers
    struct heap timers;     // monotonic time
    struct heap realtimers; // realtime
    uint32_t ntimers;
    // ios: with fd as array.index
    struct io_array ios;
    uint32_t nios;
    // one loop per thread, so one readbuf per loop is OK.
    buf_t readbuf;
    void* iowatcher;
    // custom_events
    // int eventfds[2];
    // event_queue custom_events;
    // pthread_mutex_t custom_events_mutex;
};

#define EVENT_FLAGS       \
    unsigned destroy : 1; \
    unsigned active : 1;  \
    unsigned pending : 1;

#define EVENT_FIELDS            \
    evloop_t* loop;             \
    event_type_t event_type;    \
    uint64_t event_id;          \
    event_cb cb;                \
    void* userdata;             \
    void* privdata;             \
    struct event* pending_next; \
    int priority;               \
    EVENT_FLAGS

struct event {
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

//-----------------unpack---------------------------------------------
typedef enum {
    UNPACK_MODE_NONE = 0,
    UNPACK_BY_FIXED_LENGTH = 1, // Not recommended
    UNPACK_BY_DELIMITER = 2,    // Suitable for text protocol
    UNPACK_BY_LENGTH_FIELD = 3, // Suitable for binary protocol
} unpack_mode_e;

#define DEFAULT_PACKAGE_MAX_LENGTH  (1 << 21) // 2M

// UNPACK_BY_DELIMITER
#define PACKAGE_MAX_DELIMITER_BYTES 8

// UNPACK_BY_LENGTH_FIELD
typedef enum {
    ENCODE_BY_VARINT = 17,                   // 1 MSB + 7 bits
    ENCODE_BY_LITTEL_ENDIAN = LITTLE_ENDIAN, // 1234
    ENCODE_BY_BIG_ENDIAN = BIG_ENDIAN,       // 4321
} unpack_coding_e;

typedef struct unpack_setting_s {
    unpack_mode_e mode;
    unsigned int package_max_length;
    union {
        // UNPACK_BY_FIXED_LENGTH
        struct {
            unsigned int fixed_length;
        };
        // UNPACK_BY_DELIMITER
        struct {
            unsigned char delimiter[PACKAGE_MAX_DELIMITER_BYTES];
            unsigned short delimiter_bytes;
        };
        /*
         * UNPACK_BY_LENGTH_FIELD
         *
         * package_len = head_len + body_len + length_adjustment
         *
         * if (length_field_coding == ENCODE_BY_VARINT) head_len = body_offset + varint_bytes - length_field_bytes;
         * else head_len = body_offset;
         *
         * length_field stores body length, exclude head length,
         * if length_field = head_len + body_len, then length_adjustment should be set to -head_len.
         *
         */
        struct {
            unsigned short body_offset; // Equal to head length usually
            unsigned short length_field_offset;
            unsigned short length_field_bytes;
            short length_adjustment;
            unpack_coding_e length_field_coding;
        };
    };
} unpack_setting_t;

QUEUE_DECL(offset_buf_t, write_queue);
struct evio {
    EVENT_FIELDS
    // flags
    unsigned ready : 1;
    unsigned connected : 1;
    unsigned closed : 1;
    unsigned accept : 1;
    unsigned connect : 1;
    unsigned connectex : 1; // for ConnectEx/DisconnectEx
    unsigned recv : 1;
    unsigned send : 1;
    unsigned recvfrom : 1;
    unsigned sendto : 1;
    unsigned close : 1;
    unsigned alloced_readbuf : 1; // for evio_alloc_readbuf
    unsigned alloced_ssl_ctx : 1; // for evio_new_ssl_ctx
    // public:
    evio_type_e io_type;
    uint32_t id; // fd cannot be used as unique identifier, so we provide an id
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
    // for evio_read_until
    union {
        unsigned int read_until_length;
        unsigned char read_until_delim;
    };
    uint32_t max_read_bufsize;
    uint32_t small_readbytes_cnt; // for readbuf autosize
    // write
    struct write_queue write_queue;
    pthread_mutex_t write_mutex; // lock write and write_queue
    uint32_t write_bufsize;
    uint32_t max_write_bufsize;
    // callbacks
    read_cb read_cb;
    write_cb write_cb;
    close_cb close_cb;
    accept_cb accept_cb;
    connect_cb connect_cb;
    // timers
    int connect_timeout;    // ms
    int close_timeout;      // ms
    int read_timeout;       // ms
    int write_timeout;      // ms
    int keepalive_timeout;  // ms
    int heartbeat_interval; // ms

    //     evio_send_heartbeat_fn heartbeat_fn;
    evtimer_t* connect_timer;
    evtimer_t* close_timer;
    evtimer_t* read_timer;
    evtimer_t* write_timer;
    evtimer_t* keepalive_timer;
    evtimer_t* heartbeat_timer;
    //     // upstream
    //     struct evio_s* upstream_io; // for evio_setup_upstream
    // unpack
    unpack_setting_t* unpack_setting; // for evio_set_unpack
                                      //     // ssl
                                      //     void* ssl;      // for evio_set_ssl
                                      //     void* ssl_ctx;  // for evio_set_ssl_ctx
                                      //     char* hostname; // for hssl_set_sni_hostname
    // context
    void* ctx; // for evio_context / evio_set_context
    // private:
#if defined(EVENT_POLL) || defined(EVENT_KQUEUE)
    int event_index[2]; // for poll,kqueue
#endif
};

/*
 * evio lifeline:
 *
 * fd =>
 * evio_get => EV_ALLOC_SIZEOF(io) => evio_init => evio_ready
 *
 * evio_read  => evio_add(EV_READ) => evio_read_cb
 * evio_write => evio_add(EV_WRITE) => evio_write_cb
 * evio_close => evio_done => evio_del(EV_RDWR) => evio_close_cb
 *
 * eloop_stop => eloop_free => evio_free => EV_FREE(io)
 */
void evio_init(evio_t* io);
void evio_ready(evio_t* io);
void evio_done(evio_t* io);
void evio_free(evio_t* io);
uint32_t evio_next_id();

void evio_accept_cb(evio_t* io);
void evio_connect_cb(evio_t* io);
void evio_handle_read(evio_t* io, void* buf, int readbytes);
void evio_read_cb(evio_t* io, void* buf, int len);
void evio_write_cb(evio_t* io, const void* buf, int len);
void evio_close_cb(evio_t* io);

void evio_del_connect_timer(evio_t* io);
void evio_del_close_timer(evio_t* io);
void evio_del_read_timer(evio_t* io);
void evio_del_write_timer(evio_t* io);
void evio_del_keepalive_timer(evio_t* io);
void evio_del_heartbeat_timer(evio_t* io);

static inline bool evio_is_loop_readbuf(evio_t* io) {
    return io->readbuf.base == io->loop->readbuf.base;
}
static inline bool evio_is_alloced_readbuf(evio_t* io) {
    return io->alloced_readbuf;
}
void evio_alloc_readbuf(evio_t* io, int len);
void evio_free_readbuf(evio_t* io);
void evio_memmove_readbuf(evio_t* io);

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
// userdata
void evloop_set_userdata(evloop_t* loop, void* userdata);
void* evloop_userdata(evloop_t* loop);

// event
#define event_set_id(ev, id)          ((event_t*)(ev))->event_id = id
#define event_set_cb(ev, cb)          ((event_t*)(ev))->cb = cb
#define event_set_priority(ev, prio)  ((event_t*)(ev))->priority = prio
#define event_set_userdata(ev, udata) ((event_t*)(ev))->userdata = (void*)udata

#define event_loop(ev)                (((event_t*)(ev))->loop)
// #define event_type(ev)                (((event_t*)(ev))->event_type)
#define event_id(ev)                  (((event_t*)(ev))->event_id)
#define event_cb(ev)                  (((event_t*)(ev))->cb)
#define event_priority(ev)            (((event_t*)(ev))->priority)
#define event_userdata(ev)            (((event_t*)(ev))->userdata)
uint64_t evloop_next_event_id();

#define TIMER_ENTRY(p) container_of(p, evtimer_t, node)
#define EVENT_ENTRY(p) container_of(p, event_t, pending_node)
#define IDLE_ENTRY(p)  container_of(p, evidle_t, node)

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

#define EVENT_ADD(loop, ev, cb)                \
    do {                                       \
        ev->loop = loop;                       \
        ev->event_id = evloop_next_event_id(); \
        ev->cb = (event_cb)cb;                 \
        EVENT_ACTIVE(ev);                      \
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

#define EV_READ  0x0001
#define EV_WRITE 0x0004
#define EV_RDWR  (EV_READ | EV_WRITE)

evio_t* evio_get(evloop_t* loop, int fd);
int evio_add(evio_t* io, evio_cb cb, int events);
int evio_del(evio_t* io, int events);

// high-level api
evio_t* ev_read(evloop_t* loop, int fd, evio_cb read_cb);
// evio_get -> evio_add
// evio_t* ev_hread(evloop_t* loop, int fd, void* buf, size_t len, evio_cb read_cb);

#endif