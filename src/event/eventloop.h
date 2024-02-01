#ifndef EVENTELOOP_H_
#define EVENTELOOP_H_

#include "defs.h"
#include "export.h"
#include "platform.h"

typedef struct eloop_s eloop_t;
typedef struct event_s event_t;

// NOTE: The following structures are subclasses of event_t,
// inheriting event_t data members and function members.
typedef struct eidle_s eidle_t;
typedef struct etimer_s etimer_t;
typedef struct etimeout_s etimeout_t;
typedef struct eperiod_s eperiod_t;
typedef struct eio_s eio_t;

typedef void (*event_cb)(event_t* ev);
typedef void (*eidle_cb)(eidle_t* idle);
typedef void (*etimer_cb)(etimer_t* timer);
typedef void (*eio_cb)(eio_t* io);

typedef void (*accept_cb)(eio_t* io);
typedef void (*connect_cb)(eio_t* io);
typedef void (*read_cb)(eio_t* io, void* buf, int readbytes);
typedef void (*write_cb)(eio_t* io, const void* buf, int writebytes);
typedef void (*close_cb)(eio_t* io);

typedef enum { ELOOP_STATUS_STOP,
               ELOOP_STATUS_RUNNING,
               ELOOP_STATUS_PAUSE } eloop_status_e;

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
    eloop_t* loop;                \
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
} eio_type_e;

typedef enum {
    EIO_SERVER_SIDE = 0,
    EIO_CLIENT_SIDE = 1,
} eio_side_e;

#define EIO_DEFAULT_CONNECT_TIMEOUT           10000 // ms
#define EIO_DEFAULT_CLOSE_TIMEOUT             60000 // ms
#define EIO_DEFAULT_KEEPALIVE_TIMEOUT         75000 // ms
#define EIO_DEFAULT_HEARTBEAT_INTERVAL        10000 // ms

// loop
#define ELOOP_FLAG_RUN_ONCE                   0x00000001
#define ELOOP_FLAG_AUTO_FREE                  0x00000002
#define ELOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS 0x00000004
eloop_t* eloop_new(int flags DEFAULT(ELOOP_FLAG_AUTO_FREE));

// WARN: Forbid to call eloop_free if eloop_FLAG_AUTO_FREE set.
void eloop_free(eloop_t** pp);

// NOTE: when no active events, loop will quit if eloop_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS set.
int eloop_run(eloop_t* loop);
// NOTE: eloop_stop called in loop-thread just set flag to quit in next loop,
// if called in other thread, it will wakeup loop-thread from blocking poll system call,
// then you should join loop thread to safely exit loop thread.
int eloop_stop(eloop_t* loop);
int eloop_pause(eloop_t* loop);
int eloop_resume(eloop_t* loop);
int eloop_wakeup(eloop_t* loop);
eloop_status_e eloop_status(eloop_t* loop);

void eloop_update_time(eloop_t* loop);
uint64_t eloop_now(eloop_t* loop);        // s
uint64_t eloop_now_ms(eloop_t* loop);     // ms
uint64_t eloop_now_us(eloop_t* loop);     // us
uint64_t eloop_now_hrtime(eloop_t* loop); // us

// export some loop's members
// @return pid of eloop_run
long eloop_pid(eloop_t* loop);
// @return tid of eloop_run
long eloop_tid(eloop_t* loop);
// @return count of loop
uint64_t eloop_count(eloop_t* loop);
// @return number of ios
uint32_t eloop_nios(eloop_t* loop);
// @return number of timers
uint32_t eloop_ntimers(eloop_t* loop);
// @return number of idles
uint32_t eloop_nidles(eloop_t* loop);
// @return number of active events
uint32_t eloop_nactives(eloop_t* loop);

// userdata
void eloop_set_userdata(eloop_t* loop, void* userdata);
void* eloop_userdata(eloop_t* loop);

// custom_event
/*
 * event_t ev;
 * memset(&ev, 0, sizeof(event_t));
 * ev.event_type = (event_type_e)(EVENT_TYPE_CUSTOM + 1);
 * ev.cb = custom_event_cb;
 * ev.userdata = userdata;
 * eloop_post_event(loop, &ev);
 */
// NOTE: eloop_post_event is thread-safe, used to post event from other thread to loop thread.
void eloop_post_event(eloop_t* loop, event_t* ev);

// idle
eidle_t* eidle_add(eloop_t* loop, eidle_cb cb, uint32_t repeat DEFAULT(INFINITE));
void eidle_del(eidle_t* idle);

// timer
etimer_t* etimer_add(eloop_t* loop, etimer_cb cb, uint32_t etimeout_ms, uint32_t repeat DEFAULT(INFINITE));
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
etimer_t* etimer_add_period(eloop_t* loop, etimer_cb cb, int8_t minute DEFAULT(0), int8_t hour DEFAULT(-1),
                            int8_t day DEFAULT(-1), int8_t week DEFAULT(-1), int8_t month DEFAULT(-1),
                            uint32_t repeat DEFAULT(INFINITE));

void etimer_del(etimer_t* timer);
void etimer_reset(etimer_t* timer, uint32_t etimeout_ms DEFAULT(0));

// io
//-----------------------low-level apis---------------------------------------
#define EV_READ  0x0001
#define EV_WRITE 0x0004
#define EV_RDWR  (EV_READ | EV_WRITE)
/*
const char* eio_engine() {
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
const char* eio_engine();

eio_t* eio_get(eloop_t* loop, int fd);
int eio_add(eio_t* io, eio_cb cb, int events DEFAULT(EV_READ));
int eio_del(eio_t* io, int events DEFAULT(EV_RDWR));

// NOTE: io detach from old loop and attach to new loop
/* @see examples/multi-thread/one-acceptor-multi-workers.c
void new_conn_event(event_t* ev) {
    eloop_t* loop = ev->loop;
    eio_t* io = (eio_t*)event_userdata(ev);
    eio_attach(loop, io);
}

void on_accpet(eio_t* io) {
    eio_detach(io);

    eloop_t* worker_loop = get_one_loop();
    event_t ev;
    memset(&ev, 0, sizeof(ev));
    ev.loop = worker_loop;
    ev.cb = new_conn_event;
    ev.userdata = io;
    eloop_post_event(worker_loop, &ev);
}
 */
void eio_detach(/*eloop_t* loop,*/ eio_t* io);
void eio_attach(eloop_t* loop, eio_t* io);
bool eio_exists(eloop_t* loop, int fd);

// eio_t fields
// NOTE: fd cannot be used as unique identifier, so we provide an id.
uint32_t eio_id(eio_t* io);
int eio_fd(eio_t* io);
int eio_error(eio_t* io);
int eio_events(eio_t* io);
int eio_revents(eio_t* io);
eio_type_e eio_type(eio_t* io);
struct sockaddr* eio_localaddr(eio_t* io);
struct sockaddr* eio_peeraddr(eio_t* io);
void eio_set_context(eio_t* io, void* ctx);
void* eio_context(eio_t* io);
bool eio_is_opened(eio_t* io);
bool eio_is_connected(eio_t* io);
bool eio_is_closed(eio_t* io);

// iobuf
// #include "hbuf.h"
typedef struct fifo_buf_s eio_readbuf_t;
// NOTE: One loop per thread, one readbuf per loop.
// But you can pass in your own readbuf instead of the default readbuf to avoid memcopy.
void eio_set_readbuf(eio_t* io, void* buf, size_t len);
eio_readbuf_t* eio_get_readbuf(eio_t* io);
void eio_set_max_read_bufsize(eio_t* io, uint32_t size);
void eio_set_max_write_bufsize(eio_t* io, uint32_t size);
// NOTE: eio_write is non-blocking, so there is a write queue inside eio_t to cache unwritten data and wait for writable.
// @return current buffer size of write queue.
size_t eio_write_bufsize(eio_t* io);
#define eio_write_is_complete(io) (eio_write_bufsize(io) == 0)

uint64_t eio_last_read_time(eio_t* io);  // ms
uint64_t eio_last_write_time(eio_t* io); // ms

// set callbacks
void eio_setcb_accept(eio_t* io, accept_cb accept_cb);
void eio_setcb_connect(eio_t* io, connect_cb connect_cb);
void eio_setcb_read(eio_t* io, read_cb read_cb);
void eio_setcb_write(eio_t* io, write_cb write_cb);
void eio_setcb_close(eio_t* io, close_cb close_cb);
// get callbacks
accept_cb eio_getcb_accept(eio_t* io);
connect_cb eio_getcb_connect(eio_t* io);
read_cb eio_getcb_read(eio_t* io);
write_cb eio_getcb_write(eio_t* io);
close_cb eio_getcb_close(eio_t* io);

// Enable SSL/TLS is so easy :)
// int eio_enable_ssl(eio_t* io);
// bool eio_is_ssl(eio_t* io);
// int eio_set_ssl(eio_t* io, hssl_t ssl);
// int eio_set_ssl_ctx(eio_t* io, hssl_ctx_t ssl_ctx);
// // hssl_ctx_new(opt) -> eio_set_ssl_ctx
// int eio_new_ssl_ctx(eio_t* io, hssl_ctx_opt_t* opt);
// hssl_t eio_get_ssl(eio_t* io);
// hssl_ctx_t eio_get_ssl_ctx(eio_t* io);
// // for hssl_set_sni_hostname
// int eio_set_hostname(eio_t* io, const char* hostname);
// const char* eio_get_hostname(eio_t* io);

// connect timeout => close_cb
void eio_set_connect_timeout(eio_t* io, int etimeout_ms DEFAULT(EIO_DEFAULT_CONNECT_TIMEOUT));
// close timeout => close_cb
void eio_set_close_timeout(eio_t* io, int etimeout_ms DEFAULT(EIO_DEFAULT_CLOSE_TIMEOUT));
// read timeout => close_cb
void eio_set_read_timeout(eio_t* io, int etimeout_ms);
// write timeout => close_cb
void eio_set_write_timeout(eio_t* io, int etimeout_ms);
// keepalive timeout => close_cb
void eio_set_keepalive_timeout(eio_t* io, int etimeout_ms DEFAULT(EIO_DEFAULT_KEEPALIVE_TIMEOUT));
/*
void send_heartbeat(eio_t* io) {
    static char buf[] = "PING\r\n";
    eio_write(io, buf, 6);
}
eio_set_heartbeat(io, 3000, send_heartbeat);
*/
typedef void (*eio_send_heartbeat_fn)(eio_t* io);
// heartbeat interval => eio_send_heartbeat_fn
void eio_set_heartbeat(eio_t* io, int interval_ms, eio_send_heartbeat_fn fn);

// Nonblocking, poll IO events in the loop to call corresponding callback.
// eio_add(io, EV_READ) => accept => accept_cb
int eio_accept(eio_t* io);

// connect => eio_add(io, EV_WRITE) => connect_cb
int eio_connect(eio_t* io);

// eio_add(io, EV_READ) => read => read_cb
int eio_read(eio_t* io);
#define eio_read_start(io) eio_read(io)
#define eio_read_stop(io)  eio_del(io, EV_READ)

// eio_read_start => read_cb => eio_read_stop
int eio_read_once(eio_t* io);
// eio_read_once => read_cb(len)
int eio_read_until_length(eio_t* io, unsigned int len);
// eio_read_once => read_cb(...delim)
int eio_read_until_delim(eio_t* io, unsigned char delim);
int eio_read_remain(eio_t* io);
// @see examples/tinyhttpd.c examples/tinyproxyd.c
#define eio_readline(io)        eio_read_until_delim(io, '\n')
#define eio_readstring(io)      eio_read_until_delim(io, '\0')
#define eio_readbytes(io, len)  eio_read_until_length(io, len)
#define eio_read_until(io, len) eio_read_until_length(io, len)

// NOTE: eio_write is thread-safe, locked by recursive_mutex, allow to be called by other threads.
// eio_try_write => eio_add(io, EV_WRITE) => write => write_cb
int eio_write(eio_t* io, const void* buf, size_t len);

// NOTE: eio_close is thread-safe, eio_close_async will be called actually in other thread.
// eio_del(io, EV_RDWR) => close => close_cb
int eio_close(eio_t* io);
// NOTE: eloop_post_event(eio_close_event)
int eio_close_async(eio_t* io);

//------------------high-level apis-------------------------------------------
// eio_get -> eio_set_readbuf -> eio_setcb_read -> eio_read
eio_t* hread(eloop_t* loop, int fd, void* buf, size_t len, read_cb read_cb);
// eio_get -> eio_setcb_write -> eio_write
eio_t* hwrite(eloop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));
// eio_get -> eio_close
void hclose(eloop_t* loop, int fd);

// tcp
// eio_get -> eio_setcb_accept -> eio_accept
eio_t* haccept(eloop_t* loop, int listenfd, accept_cb accept_cb);
// eio_get -> eio_setcb_connect -> eio_connect
eio_t* hconnect(eloop_t* loop, int connfd, connect_cb connect_cb);
// eio_get -> eio_set_readbuf -> eio_setcb_read -> eio_read
eio_t* hrecv(eloop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb);
// eio_get -> eio_setcb_write -> eio_write
eio_t* hsend(eloop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));

// udp
void eio_set_type(eio_t* io, eio_type_e type);
void eio_set_localaddr(eio_t* io, struct sockaddr* addr, int addrlen);
void eio_set_peeraddr(eio_t* io, struct sockaddr* addr, int addrlen);
// NOTE: must call eio_set_peeraddr before hrecvfrom/hsendto
// eio_get -> eio_set_readbuf -> eio_setcb_read -> eio_read
eio_t* hrecvfrom(eloop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb);
// eio_get -> eio_setcb_write -> eio_write
eio_t* hsendto(eloop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb DEFAULT(NULL));

//-----------------top-level apis---------------------------------------------
// @eio_create_socket: socket -> bind -> listen
// sockaddr_set_ipport -> socket -> eio_get(loop, sockfd) ->
// side == EIO_SERVER_SIDE ? bind ->
// type & EIO_TYPE_SOCK_STREAM ? listen ->
eio_t* eio_create_socket(eloop_t* loop, const char* host, int port, eio_type_e type DEFAULT(EIO_TYPE_TCP),
                         eio_side_e side DEFAULT(EIO_SERVER_SIDE));

// @tcp_server: eio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_SERVER_SIDE) -> eio_setcb_accept -> eio_accept
// @see examples/tcp_echo_server.c
eio_t* eloop_create_tcp_server(eloop_t* loop, const char* host, int port, accept_cb accept_cb);

// @tcp_client: eio_create_socket(loop, host, port, EIO_TYPE_TCP, EIO_CLIENT_SIDE) -> eio_setcb_connect -> eio_setcb_close ->
// eio_connect
// @see examples/nc.c
eio_t* eloop_create_tcp_client(eloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb);

// @ssl_server: eio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_SERVER_SIDE) -> eio_setcb_accept -> eio_accept
// @see examples/tcp_echo_server.c => #define TEST_SSL 1
eio_t* eloop_create_ssl_server(eloop_t* loop, const char* host, int port, accept_cb accept_cb);

// @ssl_client: eio_create_socket(loop, host, port, EIO_TYPE_SSL, EIO_CLIENT_SIDE) -> eio_setcb_connect -> eio_setcb_close ->
// eio_connect
// @see examples/nc.c => #define TEST_SSL 1
eio_t* eloop_create_ssl_client(eloop_t* loop, const char* host, int port, connect_cb connect_cb, close_cb close_cb);

// @udp_server: eio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_SERVER_SIDE)
// @see examples/udp_echo_server.c
eio_t* eloop_create_udp_server(eloop_t* loop, const char* host, int port);

// @udp_server: eio_create_socket(loop, host, port, EIO_TYPE_UDP, EIO_CLIENT_SIDE)
// @see examples/nc.c
eio_t* eloop_create_udp_client(eloop_t* loop, const char* host, int port);

//-----------------upstream---------------------------------------------
// eio_read(io)
// eio_read(io->upstream_io)
void eio_read_upstream(eio_t* io);
// on_write(io) -> eio_write_is_complete(io) -> eio_read(io->upstream_io)
void eio_read_upstream_on_write_complete(eio_t* io, const void* buf, int writebytes);
// eio_write(io->upstream_io, buf, bytes)
void eio_write_upstream(eio_t* io, void* buf, int bytes);
// eio_close(io->upstream_io)
void eio_close_upstream(eio_t* io);

// io1->upstream_io = io2;
// io2->upstream_io = io1;
// @see examples/socks5_proxy_server.c
void eio_setup_upstream(eio_t* io1, eio_t* io2);

// @return io->upstream_io
eio_t* eio_get_upstream(eio_t* io);

// @tcp_upstream: eio_create_socket -> eio_setup_upstream -> eio_connect -> on_connect -> eio_read_upstream
// @return upstream_io
// @see examples/tcp_proxy_server.c
eio_t* eio_setup_tcp_upstream(eio_t* io, const char* host, int port, int ssl DEFAULT(0));
#define eio_setup_ssl_upstream(io, host, port) eio_setup_tcp_upstream(io, host, port, 1)

// @udp_upstream: eio_create_socket -> eio_setup_upstream -> eio_read_upstream
// @return upstream_io
// @see examples/udp_proxy_server.c
eio_t* eio_setup_udp_upstream(eio_t* io, const char* host, int port);

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
 *       so only the pointer of unpack_setting_t is stored in eio_t,
 *       the life time of unpack_setting_t shoud be guaranteed by caller.
 */
void eio_set_unpack(eio_t* io, unpack_setting_t* setting);
void eio_unset_unpack(eio_t* io);

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