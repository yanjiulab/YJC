#ifndef EVENTEVLOOP_H_
#define EVENTEVLOOP_H_

#include "defs.h"
#include "export.h"
#include "platform.h"

typedef struct evloop_s evloop_t;
typedef struct event_s event_t;

// NOTE: The following structures are subclasses of event_t,
// inheriting event_t data members and function members.
typedef struct evidle_s evidle_t;
typedef struct evtimer_s evtimer_t;
typedef struct evtimeout_s evtimeout_t;
typedef struct eperiod_s eperiod_t;
typedef struct evio_s evio_t;

typedef void (*event_cb)(event_t* ev);
typedef void (*evidle_cb)(evidle_t* idle);
typedef void (*evtimer_cb)(evtimer_t* timer);
typedef void (*evio_cb)(evio_t* io);

typedef void (*accept_cb)(evio_t* io);
typedef void (*connect_cb)(evio_t* io);
typedef void (*read_cb)(evio_t* io, void* buf, int readbytes);
typedef void (*write_cb)(evio_t* io, const void* buf, int writebytes);
typedef void (*close_cb)(evio_t* io);

typedef enum { EVLOOP_STATUS_STOP,
               EVLOOP_STATUS_RUNNING,
               EVLOOP_STATUS_PAUSE } evloop_status_e;

typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_IO = 0x00000001,
    EVENT_TYPE_TIMEOUT = 0x00000010,
    EVENT_TYPE_PERIOD = 0x00000020,
    EVENT_TYPE_TIMER = EVENT_TYPE_TIMEOUT | EVENT_TYPE_PERIOD,
    EVENT_TYPE_IDLE = 0x00000100,
    EVENT_TYPE_CUSTOM = 0x00000400, // 1024
} event_type_e;

#define EVENT_LOWEST_PRIORITY          (-5)
#define EVENT_LOW_PRIORITY             (-3)
#define EVENT_NORMAL_PRIORITY          0
#define EVENT_HIGH_PRIORITY            3
#define EVENT_HIGHEST_PRIORITY         5
#define EVENT_PRIORITY_SIZE            (EVENT_HIGHEST_PRIORITY - EVENT_LOWEST_PRIORITY + 1)
#define EVENT_PRIORITY_INDEX(priority) (priority - EVENT_LOWEST_PRIORITY)

#define EVENT_FLAGS       \
    unsigned destroy : 1; \
    unsigned active : 1;  \
    unsigned pending : 1;

#define EVENT_FIELDS              \
    evloop_t* loop;                \
    event_type_e event_type;      \
    uint64_t event_id;            \
    event_cb cb;                  \
    void* userdata;               \
    void* privdata;               \
    struct event_s* pending_next; \
    int priority;                 \
    EVENT_FLAGS

// sizeof(struct event_s)=64 on x64
struct event_s {
    EVENT_FIELDS
};

#define event_set_id(ev, id)          ((event_t*)(ev))->event_id = id
#define event_set_cb(ev, cb)          ((event_t*)(ev))->cb = cb
#define event_set_priority(ev, prio)  ((event_t*)(ev))->priority = prio
#define event_set_userdata(ev, udata) ((event_t*)(ev))->userdata = (void*)udata

#define event_loop(ev)                (((event_t*)(ev))->loop)
#define event_type(ev)                (((event_t*)(ev))->event_type)
#define event_id(ev)                  (((event_t*)(ev))->event_id)
#define event_cb(ev)                  (((event_t*)(ev))->cb)
#define event_priority(ev)            (((event_t*)(ev))->priority)
#define event_userdata(ev)            (((event_t*)(ev))->userdata)

typedef enum {
    EIO_TYPE_UNKNOWN = 0,
    EIO_TYPE_STDIN = 0x00000001,
    EIO_TYPE_STDOUT = 0x00000002,
    EIO_TYPE_STDERR = 0x00000004,
    EIO_TYPE_STDIO = 0x0000000F,

    EIO_TYPE_FILE = 0x00000010,

    EIO_TYPE_IP = 0x00000100,
    EIO_TYPE_SOCK_RAW = 0x00000F00,

    EIO_TYPE_UDP = 0x00001000,
    EIO_TYPE_KCP = 0x00002000,
    EIO_TYPE_DTLS = 0x00010000,
    EIO_TYPE_SOCK_DGRAM = 0x000FF000,

    EIO_TYPE_TCP = 0x00100000,
    EIO_TYPE_SSL = 0x01000000,
    EIO_TYPE_TLS = EIO_TYPE_SSL,
    EIO_TYPE_SOCK_STREAM = 0x0FF00000,

    EIO_TYPE_SOCKET = 0x0FFFFF00,
} evio_type_e;

typedef enum {
    EIO_SERVER_SIDE = 0,
    EIO_CLIENT_SIDE = 1,
} evio_side_e;

#define EIO_DEFAULT_CONNECT_TIMEOUT           10000 // ms
#define EIO_DEFAULT_CLOSE_TIMEOUT             60000 // ms
#define EIO_DEFAULT_KEEPALIVE_TIMEOUT         75000 // ms
#define EIO_DEFAULT_HEARTBEAT_INTERVAL        10000 // ms

// loop
#define EVLOOP_FLAG_RUN_ONCE                   0x00000001
#define EVLOOP_FLAG_AUTO_FREE                  0x00000002
#define EVLOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS 0x00000004
evloop_t* evloop_new(int flags DEFAULT(EVLOOP_FLAG_AUTO_FREE));

// WARN: Forbid to call evloop_free if evloop_FLAG_AUTO_FREE set.
void evloop_free(evloop_t** pp);

// NOTE: when no active events, loop will quit if evloop_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS set.
int evloop_run(evloop_t* loop);
// NOTE: evloop_stop called in loop-thread just set flag to quit in next loop,
// if called in other thread, it will wakeup loop-thread from blocking poll system call,
// then you should join loop thread to safely exit loop thread.
int evloop_stop(evloop_t* loop);
int evloop_pause(evloop_t* loop);
int evloop_resume(evloop_t* loop);
int evloop_wakeup(evloop_t* loop);
evloop_status_e evloop_status(evloop_t* loop);

void evloop_update_time(evloop_t* loop);
uint64_t evloop_now(evloop_t* loop);        // s
uint64_t evloop_now_ms(evloop_t* loop);     // ms
uint64_t evloop_now_us(evloop_t* loop);     // us
uint64_t evloop_now_hrtime(evloop_t* loop); // us

// export some loop's members
// @return pid of evloop_run
long evloop_pid(evloop_t* loop);
// @return tid of evloop_run
long evloop_tid(evloop_t* loop);
// @return count of loop
uint64_t evloop_count(evloop_t* loop);
// @return number of ios
uint32_t evloop_nios(evloop_t* loop);
// @return number of timers
uint32_t evloop_ntimers(evloop_t* loop);
// @return number of idles
uint32_t evloop_nidles(evloop_t* loop);
// @return number of active events
uint32_t evloop_nactives(evloop_t* loop);

// userdata
void evloop_set_userdata(evloop_t* loop, void* userdata);
void* evloop_userdata(evloop_t* loop);

// custom_event
/*
 * event_t ev;
 * memset(&ev, 0, sizeof(event_t));
 * ev.event_type = (event_type_e)(EVENT_TYPE_CUSTOM + 1);
 * ev.cb = custom_event_cb;
 * ev.userdata = userdata;
 * evloop_post_event(loop, &ev);
 */
// NOTE: evloop_post_event is thread-safe, used to post event from other thread to loop thread.
void evloop_post_event(evloop_t* loop, event_t* ev);

// idle
evidle_t* evidle_add(evloop_t* loop, evidle_cb cb, uint32_t repeat DEFAULT(INFINITE));
void evidle_del(evidle_t* idle);

// timer
evtimer_t* evtimer_add(evloop_t* loop, evtimer_cb cb, uint32_t timeout_ms, uint32_t repeat DEFAULT(INFINITE));
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
evtimer_t* evtimer_add_period(evloop_t* loop, evtimer_cb cb, int8_t minute DEFAULT(0), int8_t hour DEFAULT(-1),
                            int8_t day DEFAULT(-1), int8_t week DEFAULT(-1), int8_t month DEFAULT(-1),
                            uint32_t repeat DEFAULT(INFINITE));

void evtimer_del(evtimer_t* timer);
void evtimer_reset(evtimer_t* timer, uint32_t evtimeout_ms DEFAULT(0));

// io
//-----------------------low-level apis---------------------------------------
#define EV_READ  0x0001
#define EV_WRITE 0x0004
#define EV_RDWR  (EV_READ | EV_WRITE)
/*
const char* evio_engine() {
#ifdef EVENT_SELECT
    return  "select";
#elif defined(EVENT_POLL)
    return  "poll";
#elif defined(EVENT_EPOLL)
    return  "epoll";
#elif defined(EVENT_KQUEUE)
    return  "kqueue";
#elif defined(EVENT_IOCP)
    return  "iocp";
#elif defined(EVENT_PORT)
    return  "evport";
#else
    return  "noevent";
#endif
}
*/
const char* evio_engine();

evio_t* evio_get(evloop_t* loop, int fd);
int evio_add(evio_t* io, evio_cb cb, int events DEFAULT(EV_READ));
int evio_del(evio_t* io, int events DEFAULT(EV_RDWR));

// NOTE: io detach from old loop and attach to new loop
/* @see examples/multi-thread/one-acceptor-multi-workers.c
void new_conn_event(event_t* ev) {
    evloop_t* loop = ev->loop;
    evio_t* io = (evio_t*)event_userdata(ev);
    evio_attach(loop, io);
}

void on_accpet(evio_t* io) {
    evio_detach(io);

    evloop_t* worker_loop = get_one_loop();
    event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.loop = worker_loop;
    ev.cb = new_conn_event;
    ev.userdata = io;
    evloop_post_event(worker_loop, &ev);
}
 */
void evio_detach(/*evloop_t* loop,*/ evio_t* io);
void evio_attach(evloop_t* loop, evio_t* io);
bool evio_exists(evloop_t* loop, int fd);

// evio_t fields
// NOTE: fd cannot be used as unique identifier, so we provide an id.
uint32_t evio_id(evio_t* io);
int evio_fd(evio_t* io);
int evio_error(evio_t* io);
int evio_events(evio_t* io);
int evio_revents(evio_t* io);
evio_type_e evio_type(evio_t* io);
struct sockaddr* evio_localaddr(evio_t* io);
struct sockaddr* evio_peeraddr(evio_t* io);
void evio_set_context(evio_t* io, void* ctx);
void* evio_context(evio_t* io);
bool evio_is_opened(evio_t* io);
bool evio_is_connected(evio_t* io);
bool evio_is_closed(evio_t* io);

// iobuf
// #include "hbuf.h"
typedef struct fifo_buf_s evio_readbuf_t;
// NOTE: One loop per thread, one readbuf per loop.
// But you can pass in your own readbuf instead of the default readbuf to avoid memcopy.
void evio_set_readbuf(evio_t* io, void* buf, size_t len);
evio_readbuf_t* evio_get_readbuf(evio_t* io);
void evio_set_max_read_bufsize(evio_t* io, uint32_t size);
void evio_set_max_write_bufsize(evio_t* io, uint32_t size);
// NOTE: evio_write is non-blocking, so there is a write queue inside evio_t to cache unwritten data and wait for writable.
// @return current buffer size of write queue.
size_t evio_write_bufsize(evio_t* io);
#define evio_write_is_complete(io) (evio_write_bufsize(io) == 0)

uint64_t evio_last_read_time(evio_t* io);  // ms
uint64_t evio_last_write_time(evio_t* io); // ms

// set callbacks
void evio_setcb_accept(evio_t* io, accept_cb accept_cb);
void evio_setcb_connect(evio_t* io, connect_cb connect_cb);
void evio_setcb_read(evio_t* io, read_cb read_cb);
void evio_setcb_write(evio_t* io, write_cb write_cb);
void evio_setcb_close(evio_t* io, close_cb close_cb);
// get callbacks
accept_cb evio_getcb_accept(evio_t* io);
connect_cb evio_getcb_connect(evio_t* io);
read_cb evio_getcb_read(evio_t* io);
write_cb evio_getcb_write(evio_t* io);
close_cb evio_getcb_close(evio_t* io);

// Enable SSL/TLS is so easy :)
// int evio_enable_ssl(evio_t* io);
// bool evio_is_ssl(evio_t* io);
// int evio_set_ssl(evio_t* io, hssl_t ssl);
// int evio_set_ssl_ctx(evio_t* io, hssl_ctx_t ssl_ctx);
// // hssl_ctx_new(opt) -> evio_set_ssl_ctx
// int evio_new_ssl_ctx(evio_t* io, hssl_ctx_opt_t* opt);
// hssl_t evio_get_ssl(evio_t* io);
// hssl_ctx_t evio_get_ssl_ctx(evio_t* io);
// // for hssl_set_sni_hostname
// int evio_set_hostname(evio_t* io, const char* hostname);
// const char* evio_get_hostname(evio_t* io);

// connect timeout => close_cb
void evio_set_connect_timeout(evio_t* io, int evtimeout_ms DEFAULT(EIO_DEFAULT_CONNECT_TIMEOUT));
// close timeout => close_cb
void evio_set_close_timeout(evio_t* io, int evtimeout_ms DEFAULT(EIO_DEFAULT_CLOSE_TIMEOUT));
// read timeout => close_cb
void evio_set_read_timeout(evio_t* io, int evtimeout_ms);
// write timeout => close_cb
void evio_set_write_timeout(evio_t* io, int evtimeout_ms);
// keepalive timeout => close_cb
void evio_set_keepalive_timeout(evio_t* io, int evtimeout_ms DEFAULT(EIO_DEFAULT_KEEPALIVE_TIMEOUT));
/*
void send_heartbeat(evio_t* io) {
    static char buf[] = "PING\r\n";
    evio_write(io, buf, 6);
}
evio_set_heartbeat(io, 3000, send_heartbeat);
*/
typedef void (*evio_send_heartbeat_fn)(evio_t* io);
// heartbeat interval => evio_send_heartbeat_fn
void evio_set_heartbeat(evio_t* io, int interval_ms, evio_send_heartbeat_fn fn);

// Nonblocking, poll IO events in the loop to call corresponding callback.
// evio_add(io, EV_READ) => accept => accept_cb
int evio_accept(evio_t* io);

// connect => evio_add(io, EV_WRITE) => connect_cb
int evio_connect(evio_t* io);

// evio_add(io, EV_READ) => read => read_cb
int evio_read(evio_t* io);
#define evio_read_start(io) evio_read(io)
#define evio_read_stop(io)  evio_del(io, EV_READ)
// evio_read_start => read_cb => evio_read_stop
int evio_read_once(evio_t* io);
// evio_read_once => read_cb(len)
int evio_read_until_length(evio_t* io, unsigned int len);
// evio_read_once => read_cb(...delim)
int evio_read_until_delim(evio_t* io, unsigned char delim);
int evio_read_remain(evio_t* io);
// @see examples/tinyhttpd.c examples/tinyproxyd.c
#define evio_readline(io)        evio_read_until_delim(io, '\n')
#define evio_readstring(io)      evio_read_until_delim(io, '\0')
#define evio_readbytes(io, len)  evio_read_until_length(io, len)
#define evio_read_until(io, len) evio_read_until_length(io, len)
// evio_get => evio_add(io, EV_READ) => evio_cb
evio_t* evio_read_raw(evloop_t* loop, int fd, evio_cb read_cb);

// NOTE: evio_write is thread-safe, locked by recursive_mutex, allow to be called by other threads.
// evio_try_write => evio_add(io, EV_WRITE) => write => write_cb
int evio_write(evio_t* io, const void* buf, size_t len);

// NOTE: evio_close is thread-safe, evio_close_async will be called actually in other thread.
// evio_del(io, EV_RDWR) => close => close_cb
int evio_close(evio_t* io);
// NOTE: evloop_post_event(evio_close_event)
int evio_close_async(evio_t* io);

//------------------high-level apis-------------------------------------------
// evio_get -> evio_set_readbuf -> evio_setcb_read -> evio_read
evio_t* hread(evloop_t* loop, int fd, void* buf, size_t len, read_cb read_cb);
// evio_get -> evio_setcb_write -> evio_write
evio_t* hwrite(evloop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));
// evio_get -> evio_close
void hclose(evloop_t* loop, int fd);

// tcp
// evio_get -> evio_setcb_accept -> evio_accept
evio_t* haccept(evloop_t* loop, int listenfd, accept_cb accept_cb);
// evio_get -> evio_setcb_connect -> evio_connect
evio_t* hconnect(evloop_t* loop, int connfd, connect_cb connect_cb);
// evio_get -> evio_set_readbuf -> evio_setcb_read -> evio_read
evio_t* hrecv(evloop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb);
// evio_get -> evio_setcb_write -> evio_write
evio_t* hsend(evloop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));

// udp
void evio_set_type(evio_t* io, evio_type_e type);
void evio_set_localaddr(evio_t* io, struct sockaddr* addr, int addrlen);
void evio_set_peeraddr(evio_t* io, struct sockaddr* addr, int addrlen);
// NOTE: must call evio_set_peeraddr before hrecvfrom/hsendto
// evio_get -> evio_set_readbuf -> evio_setcb_read -> evio_read
evio_t* hrecvfrom(evloop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb);
// evio_get -> evio_setcb_write -> evio_write
evio_t* hsendto(evloop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));

//-----------------top-level apis---------------------------------------------
// @evio_create_socket: socket -> bind -> listen
// sockaddr_set_ipport -> socket -> evio_get(loop, sockfd) ->
// side == EIO_SERVER_SIDE ? bind ->
// type & EIO_TYPE_SOCK_STREAM ? listen ->
evio_t* evio_create_socket(evloop_t* loop, const char* host, int port, evio_type_e type DEFAULT(EIO_TYPE_TCP),
                         evio_side_e side DEFAULT(EIO_SERVER_SIDE));

// @tcp_server: evio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_SERVER_SIDE) -> evio_setcb_accept -> evio_accept
// @see examples/tcp_echo_server.c
evio_t* evloop_create_tcp_server(evloop_t* loop, const char* host, int port, accept_cb accept_cb);

// @tcp_client: evio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_CLIENT_SIDE) -> evio_setcb_connect -> evio_setcb_close ->
// evio_connect
// @see examples/nc.c
evio_t* evloop_create_tcp_client(evloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb);

// @ssl_server: evio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_SERVER_SIDE) -> evio_setcb_accept -> evio_accept
// @see examples/tcp_echo_server.c => #define TEST_SSL 1
evio_t* evloop_create_ssl_server(evloop_t* loop, const char* host, int port, accept_cb accept_cb);

// @ssl_client: evio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_CLIENT_SIDE) -> evio_setcb_connect -> evio_setcb_close ->
// evio_connect
// @see examples/nc.c => #define TEST_SSL 1
evio_t* evloop_create_ssl_client(evloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb);

// @udp_server: evio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_SERVER_SIDE)
// @see examples/udp_echo_server.c
evio_t* evloop_create_udp_server(evloop_t* loop, const char* host, int port);

// @udp_server: evio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_CLIENT_SIDE)
// @see examples/nc.c
evio_t* evloop_create_udp_client(evloop_t* loop, const char* host, int port);

//-----------------upstream---------------------------------------------
// evio_read(io)
// evio_read(io->upstream_io)
void evio_read_upstream(evio_t* io);
// on_write(io) -> evio_write_is_complete(io) -> evio_read(io->upstream_io)
void evio_read_upstream_on_write_complete(evio_t* io, const void* buf, int writebytes);
// evio_write(io->upstream_io, buf, bytes)
void evio_write_upstream(evio_t* io, void* buf, int bytes);
// evio_close(io->upstream_io)
void evio_close_upstream(evio_t* io);

// io1->upstream_io = io2;
// io2->upstream_io = io1;
// @see examples/socks5_proxy_server.c
void evio_setup_upstream(evio_t* io1, evio_t* io2);

// @return io->upstream_io
evio_t* evio_get_upstream(evio_t* io);

// @tcp_upstream: evio_create_socket -> evio_setup_upstream -> evio_connect -> on_connect -> evio_read_upstream
// @return upstream_io
// @see examples/tcp_proxy_server.c
evio_t* evio_setup_tcp_upstream(evio_t* io, const char* host, int port, int ssl DEFAULT(0));
#define evio_setup_ssl_upstream(io, host, port) evio_setup_tcp_upstream(io, host, port, 1)

// @udp_upstream: evio_create_socket -> evio_setup_upstream -> evio_read_upstream
// @return upstream_io
// @see examples/udp_proxy_server.c
evio_t* evio_setup_udp_upstream(evio_t* io, const char* host, int port);

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

/*
 * @see examples/jsonrpc examples/protorpc
 *
 * NOTE: unpack_setting_t of multiple IOs of the same function also are same,
 *       so only the pointer of unpack_setting_t is stored in evio_t,
 *       the life time of unpack_setting_t shoud be guaranteed by caller.
 */
void evio_set_unpack(evio_t* io, unpack_setting_t* setting);
void evio_unset_unpack(evio_t* io);

// unpack examples
/*
unpack_setting_t ftp_unpack_setting;
memset(&ftp_unpack_setting, 0, sizeof(unpack_setting_t));
ftp_unpack_setting.package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
ftp_unpack_setting.mode = UNPACK_BY_DELIMITER;
ftp_unpack_setting.delimiter[0] = '\r';
ftp_unpack_setting.delimiter[1] = '\n';
ftp_unpack_setting.delimiter_bytes = 2;

unpack_setting_t mqtt_unpack_setting = {
    .mode = UNPACK_BY_LENGTH_FIELD,
    .package_max_length = DEFAULT_PACKAGE_MAX_LENGTH,
    .body_offset = 2,
    .length_field_offset = 1,
    .length_field_bytes = 1,
    .length_field_coding = ENCODE_BY_VARINT,
};

unpack_setting_t grpc_unpack_setting = {
    .mode = UNPACK_BY_LENGTH_FIELD,
    .package_max_length = DEFAULT_PACKAGE_MAX_LENGTH,
    .body_offset = 5,
    .length_field_offset = 1,
    .length_field_bytes = 4,
    .length_field_coding = ENCODE_BY_BIG_ENDIAN,
};
*/

//-----------------reconnect----------------------------------------
#define DEFAULT_RECONNECT_MIN_DELAY     1000  // ms
#define DEFAULT_RECONNECT_MAX_DELAY     60000 // ms
#define DEFAULT_RECONNECT_DELAY_POLICY  2     // exponential
#define DEFAULT_RECONNECT_MAX_RETRY_CNT INFINITE
typedef struct reconn_setting_s {
    uint32_t min_delay; // ms
    uint32_t max_delay; // ms
    uint32_t cur_delay; // ms
    /*
     * @delay_policy
     * 0: fixed
     * min_delay=3s => 3,3,3...
     * 1: linear
     * min_delay=3s max_delay=10s => 3,6,9,10,10...
     * other: exponential
     * min_delay=3s max_delay=60s delay_policy=2 => 3,6,12,24,48,60,60...
     */
    uint32_t delay_policy;
    uint32_t max_retry_cnt;
    uint32_t cur_retry_cnt;

} reconn_setting_t;

static void reconn_setting_init(reconn_setting_t* reconn) {
    reconn->min_delay = DEFAULT_RECONNECT_MIN_DELAY;
    reconn->max_delay = DEFAULT_RECONNECT_MAX_DELAY;
    reconn->cur_delay = 0;
    // 1,2,4,8,16,32,60,60...
    reconn->delay_policy = DEFAULT_RECONNECT_DELAY_POLICY;
    reconn->max_retry_cnt = DEFAULT_RECONNECT_MAX_RETRY_CNT;
    reconn->cur_retry_cnt = 0;
}

static void reconn_setting_reset(reconn_setting_t* reconn) {
    reconn->cur_delay = 0;
    reconn->cur_retry_cnt = 0;
}

static bool reconn_setting_can_retry(reconn_setting_t* reconn) {
    ++reconn->cur_retry_cnt;
    return reconn->max_retry_cnt == INFINITE || reconn->cur_retry_cnt < reconn->max_retry_cnt;
}

static uint32_t reconn_setting_calc_delay(reconn_setting_t* reconn) {
    if (reconn->delay_policy == 0) {
        // fixed
        reconn->cur_delay = reconn->min_delay;
    } else if (reconn->delay_policy == 1) {
        // linear
        reconn->cur_delay += reconn->min_delay;
    } else {
        // exponential
        reconn->cur_delay *= reconn->delay_policy;
    }
    reconn->cur_delay = MAX(reconn->cur_delay, reconn->min_delay);
    reconn->cur_delay = MIN(reconn->cur_delay, reconn->max_delay);
    return reconn->cur_delay;
}

//-----------------LoadBalance-------------------------------------
typedef enum {
    LB_RoundRobin,
    LB_Random,
    LB_LeastConnections,
    LB_IpHash,
    LB_UrlHash,
} load_balance_e;

#endif