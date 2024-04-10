#include "event.h"

#include "base.h"
#include "err.h"
#include "log.h"
#include "socket.h"
#include "unpack.h"
#include "sockunion.h"
#include "sockopt.h"

uint64_t evloop_next_event_id() {
    static atomic_long s_id = ATOMIC_VAR_INIT(0);
    return ++s_id;
}

uint32_t evio_next_id() {
    static atomic_long s_id = ATOMIC_VAR_INIT(0);
    return ++s_id;
}

static void fill_io_type(evio_t* io) {
    int type = 0;
    socklen_t optlen = sizeof(int);
    int ret = getsockopt(io->fd, SOL_SOCKET, SO_TYPE, (char*)&type, &optlen);
    printd("getsockopt SO_TYPE fd=%d ret=%d type=%d errno=%d\n", io->fd, ret, type, socket_errno());
    if (ret == 0) {
        switch (type) {
        case SOCK_STREAM:
            io->io_type = EIO_TYPE_TCP;
            break;
        case SOCK_DGRAM:
            io->io_type = EIO_TYPE_UDP;
            break;
        case SOCK_RAW:
            io->io_type = EIO_TYPE_IP;
            break;
        default:
            io->io_type = EIO_TYPE_SOCKET;
            break;
        }
    } else if (socket_errno() == ENOTSOCK) {
        switch (io->fd) {
        case 0:
            io->io_type = EIO_TYPE_STDIN;
            break;
        case 1:
            io->io_type = EIO_TYPE_STDOUT;
            break;
        case 2:
            io->io_type = EIO_TYPE_STDERR;
            break;
        default:
            io->io_type = EIO_TYPE_FILE;
            break;
        }
    } else {
        io->io_type = EIO_TYPE_TCP;
    }
}

static void evio_socket_init(evio_t* io) {
    if ((io->io_type & EIO_TYPE_SOCK_DGRAM) || (io->io_type & EIO_TYPE_SOCK_RAW)) {
        // NOTE: sendto multiple peeraddr cannot use io->write_queue
        blocking(io->fd);
    } else {
        nonblocking(io->fd);
    }
    // fill io->localaddr io->peeraddr
    if (io->localaddr == NULL) {
        EV_ALLOC(io->localaddr, sizeof(sockaddr_u));
    }
    if (io->peeraddr == NULL) {
        EV_ALLOC(io->peeraddr, sizeof(sockaddr_u));
    }
    socklen_t addrlen = sizeof(sockaddr_u);
    int ret = getsockname(io->fd, io->localaddr, &addrlen);
    printd("getsockname fd=%d ret=%d errno=%d\n", io->fd, ret, socket_errno());
    // NOTE: udp peeraddr set by recvfrom/sendto
    if (io->io_type & EIO_TYPE_SOCK_STREAM) {
        addrlen = sizeof(sockaddr_u);
        ret = getpeername(io->fd, io->peeraddr, &addrlen);
        printd("getpeername fd=%d ret=%d errno=%d\n", io->fd, ret, socket_errno());
    }
}

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
    io->connected = 0;
    io->closed = 0;
    io->accept = io->connect = io->connectex = 0;
    io->recv = io->send = 0;
    io->recvfrom = io->sendto = 0;
    io->close = 0;
    // public:
    io->id = evio_next_id();
    io->io_type = EIO_TYPE_UNKNOWN;
    io->error = 0;
    io->events = io->revents = 0;
    io->last_read_hrtime = io->last_write_hrtime = io->loop->cur_hrtime;
    // readbuf
    io->alloced_readbuf = 0;
    io->readbuf.base = io->loop->readbuf.base;
    io->readbuf.len = io->loop->readbuf.len;
    io->readbuf.head = io->readbuf.tail = 0;
    io->read_flags = 0;
    io->read_until_length = 0;
    io->max_read_bufsize = MAX_READ_BUFSIZE;
    io->small_readbytes_cnt = 0;
    // write_queue
    io->write_bufsize = 0;
    io->max_write_bufsize = MAX_WRITE_BUFSIZE;
    // callbacks
    io->read_cb = NULL;
    io->write_cb = NULL;
    io->close_cb = NULL;
    io->accept_cb = NULL;
    io->connect_cb = NULL;
    // timers
    io->connect_timeout = 0;
    io->connect_timer = NULL;
    io->close_timeout = 0;
    io->close_timer = NULL;
    io->read_timeout = 0;
    io->read_timer = NULL;
    io->write_timeout = 0;
    io->write_timer = NULL;
    io->keepalive_timeout = 0;
    io->keepalive_timer = NULL;
    io->heartbeat_interval = 0;
    io->heartbeat_fn = NULL;
    io->heartbeat_timer = NULL;
    // upstream
    io->upstream_io = NULL;
    // unpack
    io->unpack_setting = NULL;
    // ssl
    io->ssl = NULL;
    io->ssl_ctx = NULL;
    io->alloced_ssl_ctx = 0;
    io->hostname = NULL;
    // context
    io->ctx = NULL;
    // private:
#if defined(EVENT_POLL) || defined(EVENT_KQUEUE)
    io->event_index[0] = io->event_index[1] = -1;
#endif
#ifdef EVENT_IOCP
    io->hovlp = NULL;
#endif

    // io_type
    fill_io_type(io);
    if (io->io_type & EIO_TYPE_SOCKET) {
        evio_socket_init(io);
    }
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

uint32_t evio_id(evio_t* io) {
    return io->id;
}

int evio_fd(evio_t* io) {
    return io->fd;
}

evio_type_e evio_type(evio_t* io) {
    return io->io_type;
}

int evio_error(evio_t* io) {
    return io->error;
}

int evio_events(evio_t* io) {
    return io->events;
}

int evio_revents(evio_t* io) {
    return io->revents;
}

struct sockaddr* evio_localaddr(evio_t* io) {
    return io->localaddr;
}

struct sockaddr* evio_peeraddr(evio_t* io) {
    return io->peeraddr;
}

void evio_set_context(evio_t* io, void* ctx) {
    io->ctx = ctx;
}

void* evio_context(evio_t* io) {
    return io->ctx;
}

accept_cb evio_getcb_accept(evio_t* io) {
    return io->accept_cb;
}

connect_cb evio_getcb_connect(evio_t* io) {
    return io->connect_cb;
}

read_cb evio_getcb_read(evio_t* io) {
    return io->read_cb;
}

write_cb evio_getcb_write(evio_t* io) {
    return io->write_cb;
}

close_cb evio_getcb_close(evio_t* io) {
    return io->close_cb;
}

void evio_setcb_accept(evio_t* io, accept_cb accept_cb) {
    io->accept_cb = accept_cb;
}

void evio_setcb_connect(evio_t* io, connect_cb connect_cb) {
    io->connect_cb = connect_cb;
}

void evio_setcb_read(evio_t* io, read_cb read_cb) {
    io->read_cb = read_cb;
}

void evio_setcb_write(evio_t* io, write_cb write_cb) {
    io->write_cb = write_cb;
}

void evio_setcb_close(evio_t* io, close_cb close_cb) {
    io->close_cb = close_cb;
}

void evio_accept_cb(evio_t* io) {
    /*
    char localaddrstr[SU_ADDRSTRLEN] = {0};
    char peeraddrstr[SU_ADDRSTRLEN] = {0};
    printd("accept connfd=%d [%s] <= [%s]\n", io->fd,
            SU_ADDRSTR(io->localaddr, localaddrstr),
            SU_ADDRSTR(io->peeraddr, peeraddrstr));
    */
    if (io->accept_cb) {
        // printd("accept_cb------\n");
        io->accept_cb(io);
        // printd("accept_cb======\n");
    }
}

void evio_connect_cb(evio_t* io) {
    /*
    char localaddrstr[SU_ADDRSTRLEN] = {0};
    char peeraddrstr[SU_ADDRSTRLEN] = {0};
    printd("connect connfd=%d [%s] => [%s]\n", io->fd,
            SU_ADDRSTR(io->localaddr, localaddrstr),
            SU_ADDRSTR(io->peeraddr, peeraddrstr));
    */
    io->connected = 1;
    if (io->connect_cb) {
        // printd("connect_cb------\n");
        io->connect_cb(io);
        // printd("connect_cb======\n");
    }
}

void evio_handle_read(evio_t* io, void* buf, int readbytes) {
    if (io->unpack_setting) {
        // evio_set_unpack
        evio_unpack(io, buf, readbytes);
    } else {
        const unsigned char* sp = (const unsigned char*)io->readbuf.base + io->readbuf.head;
        const unsigned char* ep = (const unsigned char*)buf + readbytes;
        if (io->read_flags & EIO_READ_UNTIL_LENGTH) {
            // evio_read_until_length
            if (ep - sp >= io->read_until_length) {
                io->readbuf.head += io->read_until_length;
                if (io->readbuf.head == io->readbuf.tail) {
                    io->readbuf.head = io->readbuf.tail = 0;
                }
                io->read_flags &= ~EIO_READ_UNTIL_LENGTH;
                evio_read_cb(io, (void*)sp, io->read_until_length);
            }
        } else if (io->read_flags & EIO_READ_UNTIL_DELIM) {
            // evio_read_until_delim
            const unsigned char* p = (const unsigned char*)buf;
            for (int i = 0; i < readbytes; ++i, ++p) {
                if (*p == io->read_until_delim) {
                    int len = p - sp + 1;
                    io->readbuf.head += len;
                    if (io->readbuf.head == io->readbuf.tail) {
                        io->readbuf.head = io->readbuf.tail = 0;
                    }
                    io->read_flags &= ~EIO_READ_UNTIL_DELIM;
                    evio_read_cb(io, (void*)sp, len);
                    return;
                }
            }
        } else {
            // evio_read
            io->readbuf.head = io->readbuf.tail = 0;
            evio_read_cb(io, (void*)sp, ep - sp);
        }
    }

    if (io->readbuf.head == io->readbuf.tail) {
        io->readbuf.head = io->readbuf.tail = 0;
    }
    // readbuf autosize
    if (io->readbuf.tail == io->readbuf.len) {
        if (io->readbuf.head == 0) {
            // scale up * 2
            evio_alloc_readbuf(io, io->readbuf.len * 2);
        } else {
            evio_memmove_readbuf(io);
        }
    } else {
        size_t small_size = io->readbuf.len / 2;
        if (io->readbuf.tail < small_size && io->small_readbytes_cnt >= 3) {
            // scale down / 2
            evio_alloc_readbuf(io, small_size);
        }
    }
}

void evio_read_cb(evio_t* io, void* buf, int len) {
    if (io->read_flags & EIO_READ_ONCE) {
        io->read_flags &= ~EIO_READ_ONCE;
        evio_read_stop(io);
    }

    if (io->read_cb) {
        // printd("read_cb------\n");
        io->read_cb(io, buf, len);
        // printd("read_cb======\n");
    }

    // for readbuf autosize
    if (evio_is_alloced_readbuf(io) && io->readbuf.len > READ_BUFSIZE_HIGH_WATER) {
        size_t small_size = io->readbuf.len / 2;
        if (len < small_size) {
            ++io->small_readbytes_cnt;
        } else {
            io->small_readbytes_cnt = 0;
        }
    }
}

void evio_write_cb(evio_t* io, const void* buf, int len) {
    if (io->write_cb) {
        // printd("write_cb------\n");
        io->write_cb(io, buf, len);
        // printd("write_cb======\n");
    }
}

void evio_close_cb(evio_t* io) {
    io->connected = 0;
    io->closed = 1;
    if (io->close_cb) {
        // printd("close_cb------\n");
        io->close_cb(io);
        // printd("close_cb======\n");
    }
}

void evio_set_type(evio_t* io, evio_type_e type) {
    io->io_type = type;
}

void evio_set_localaddr(evio_t* io, struct sockaddr* addr, int addrlen) {
    if (io->localaddr == NULL) {
        EV_ALLOC(io->localaddr, sizeof(sockaddr_u));
    }
    memcpy(io->localaddr, addr, addrlen);
}

void evio_set_peeraddr(evio_t* io, struct sockaddr* addr, int addrlen) {
    if (io->peeraddr == NULL) {
        EV_ALLOC(io->peeraddr, sizeof(sockaddr_u));
    }
    memcpy(io->peeraddr, addr, addrlen);
}

int evio_set_hostname(evio_t* io, const char* hostname) {
    SAFE_FREE(io->hostname);
    io->hostname = strdup(hostname);
    return 0;
}

const char* evio_get_hostname(evio_t* io) {
    return io->hostname;
}

void evio_del_connect_timer(evio_t* io) {
    if (io->connect_timer) {
        evtimer_del(io->connect_timer);
        io->connect_timer = NULL;
        io->connect_timeout = 0;
    }
}

void evio_del_close_timer(evio_t* io) {
    if (io->close_timer) {
        evtimer_del(io->close_timer);
        io->close_timer = NULL;
        io->close_timeout = 0;
    }
}

void evio_del_read_timer(evio_t* io) {
    if (io->read_timer) {
        evtimer_del(io->read_timer);
        io->read_timer = NULL;
        io->read_timeout = 0;
    }
}

void evio_del_write_timer(evio_t* io) {
    if (io->write_timer) {
        evtimer_del(io->write_timer);
        io->write_timer = NULL;
        io->write_timeout = 0;
    }
}

void evio_del_keepalive_timer(evio_t* io) {
    if (io->keepalive_timer) {
        evtimer_del(io->keepalive_timer);
        io->keepalive_timer = NULL;
        io->keepalive_timeout = 0;
    }
}

void evio_del_heartbeat_timer(evio_t* io) {
    if (io->heartbeat_timer) {
        evtimer_del(io->heartbeat_timer);
        io->heartbeat_timer = NULL;
        io->heartbeat_interval = 0;
        io->heartbeat_fn = NULL;
    }
}

void evio_set_connect_timeout(evio_t* io, int timeout_ms) {
    io->connect_timeout = timeout_ms;
}

void evio_set_close_timeout(evio_t* io, int timeout_ms) {
    io->close_timeout = timeout_ms;
}

static void __read_timeout_cb(evtimer_t* timer) {
    evio_t* io = (evio_t*)timer->privdata;
    uint64_t inactive_ms = (io->loop->cur_hrtime - io->last_read_hrtime) / 1000;
    if (inactive_ms + 100 < io->read_timeout) {
        evtimer_reset(io->read_timer, io->read_timeout - inactive_ms);
    } else {
        if (io->io_type & EIO_TYPE_SOCKET) {
            char localaddrstr[SU_ADDRSTRLEN] = {0};
            char peeraddrstr[SU_ADDRSTRLEN] = {0};
            log_warn("read timeout [%s] <=> [%s]", SU_ADDRSTR(io->localaddr, localaddrstr),
                     SU_ADDRSTR(io->peeraddr, peeraddrstr));
        }
        io->error = ETIMEDOUT;
        evio_close(io);
    }
}

void evio_set_read_timeout(evio_t* io, int timeout_ms) {
    if (timeout_ms <= 0) {
        // del
        evio_del_read_timer(io);
        return;
    }

    if (io->read_timer) {
        // reset
        evtimer_reset(io->read_timer, timeout_ms);
    } else {
        // add
        io->read_timer = evtimer_add(io->loop, __read_timeout_cb, timeout_ms, 1);
        io->read_timer->privdata = io;
    }
    io->read_timeout = timeout_ms;
}

static void __write_timeout_cb(evtimer_t* timer) {
    evio_t* io = (evio_t*)timer->privdata;
    uint64_t inactive_ms = (io->loop->cur_hrtime - io->last_write_hrtime) / 1000;
    if (inactive_ms + 100 < io->write_timeout) {
        evtimer_reset(io->write_timer, io->write_timeout - inactive_ms);
    } else {
        if (io->io_type & EIO_TYPE_SOCKET) {
            char localaddrstr[SU_ADDRSTRLEN] = {0};
            char peeraddrstr[SU_ADDRSTRLEN] = {0};
            log_warn("write timeout [%s] <=> [%s]", SU_ADDRSTR(io->localaddr, localaddrstr),
                     SU_ADDRSTR(io->peeraddr, peeraddrstr));
        }
        io->error = ETIMEDOUT;
        evio_close(io);
    }
}

void evio_set_write_timeout(evio_t* io, int timeout_ms) {
    if (timeout_ms <= 0) {
        // del
        evio_del_write_timer(io);
        return;
    }

    if (io->write_timer) {
        // reset
        evtimer_reset(io->write_timer, timeout_ms);
    } else {
        // add
        io->write_timer = evtimer_add(io->loop, __write_timeout_cb, timeout_ms, 1);
        io->write_timer->privdata = io;
    }
    io->write_timeout = timeout_ms;
}

static void __keepalive_timeout_cb(evtimer_t* timer) {
    evio_t* io = (evio_t*)timer->privdata;
    uint64_t last_rw_hrtime = MAX(io->last_read_hrtime, io->last_write_hrtime);
    uint64_t inactive_ms = (io->loop->cur_hrtime - last_rw_hrtime) / 1000;
    if (inactive_ms + 100 < io->keepalive_timeout) {
        evtimer_reset(io->keepalive_timer, io->keepalive_timeout - inactive_ms);
    } else {
        if (io->io_type & EIO_TYPE_SOCKET) {
            char localaddrstr[SU_ADDRSTRLEN] = {0};
            char peeraddrstr[SU_ADDRSTRLEN] = {0};
            log_warn("keepalive timeout [%s] <=> [%s]", SU_ADDRSTR(io->localaddr, localaddrstr),
                     SU_ADDRSTR(io->peeraddr, peeraddrstr));
        }
        io->error = ETIMEDOUT;
        evio_close(io);
    }
}

void evio_set_keepalive_timeout(evio_t* io, int timeout_ms) {
    if (timeout_ms <= 0) {
        // del
        evio_del_keepalive_timer(io);
        return;
    }

    if (io->keepalive_timer) {
        // reset
        evtimer_reset(io->keepalive_timer, timeout_ms);
    } else {
        // add
        io->keepalive_timer = evtimer_add(io->loop, __keepalive_timeout_cb, timeout_ms, 1);
        io->keepalive_timer->privdata = io;
    }
    io->keepalive_timeout = timeout_ms;
}

static void __heartbeat_timer_cb(evtimer_t* timer) {
    evio_t* io = (evio_t*)timer->privdata;
    if (io && io->heartbeat_fn) {
        io->heartbeat_fn(io);
    }
}

void evio_set_heartbeat(evio_t* io, int interval_ms, evio_send_heartbeat_fn fn) {
    if (interval_ms <= 0) {
        // del
        evio_del_heartbeat_timer(io);
        return;
    }

    if (io->heartbeat_timer) {
        // reset
        evtimer_reset(io->heartbeat_timer, interval_ms);
    } else {
        // add
        io->heartbeat_timer = evtimer_add(io->loop, __heartbeat_timer_cb, interval_ms, INFINITE);
        io->heartbeat_timer->privdata = io;
    }
    io->heartbeat_interval = interval_ms;
    io->heartbeat_fn = fn;
}

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

evio_readbuf_t* evio_get_readbuf(evio_t* io) {
    return &io->readbuf;
}

void evio_set_max_read_bufsize(evio_t* io, uint32_t size) {
    io->max_read_bufsize = size;
}

void evio_set_max_write_bufsize(evio_t* io, uint32_t size) {
    io->max_write_bufsize = size;
}

size_t evio_write_bufsize(evio_t* io) {
    return io->write_bufsize;
}

int evio_read_once(evio_t* io) {
    io->read_flags |= EIO_READ_ONCE;
    return evio_read_start(io);
}

int evio_read_until_length(evio_t* io, unsigned int len) {
    if (len == 0)
        return 0;
    if (io->readbuf.tail - io->readbuf.head >= len) {
        void* buf = io->readbuf.base + io->readbuf.head;
        io->readbuf.head += len;
        if (io->readbuf.head == io->readbuf.tail) {
            io->readbuf.head = io->readbuf.tail = 0;
        }
        evio_read_cb(io, buf, len);
        return len;
    }
    io->read_flags = EIO_READ_UNTIL_LENGTH;
    io->read_until_length = len;
    if (io->readbuf.head > 1024 || io->readbuf.tail - io->readbuf.head < 1024) {
        evio_memmove_readbuf(io);
    }
    // NOTE: prepare readbuf
    int need_len = io->readbuf.head + len;
    if (evio_is_loop_readbuf(io) || io->readbuf.len < need_len) {
        evio_alloc_readbuf(io, need_len);
    }
    return evio_read_once(io);
}

int evio_read_until_delim(evio_t* io, unsigned char delim) {
    if (io->readbuf.tail - io->readbuf.head > 0) {
        const unsigned char* sp = (const unsigned char*)io->readbuf.base + io->readbuf.head;
        const unsigned char* ep = (const unsigned char*)io->readbuf.base + io->readbuf.tail;
        const unsigned char* p = sp;
        while (p <= ep) {
            if (*p == delim) {
                int len = p - sp + 1;
                io->readbuf.head += len;
                if (io->readbuf.head == io->readbuf.tail) {
                    io->readbuf.head = io->readbuf.tail = 0;
                }
                evio_read_cb(io, (void*)sp, len);
                return len;
            }
            ++p;
        }
    }
    io->read_flags = EIO_READ_UNTIL_DELIM;
    io->read_until_length = delim;
    // NOTE: prepare readbuf
    if (evio_is_loop_readbuf(io) || io->readbuf.len < EVLOOP_READ_BUFSIZE) {
        evio_alloc_readbuf(io, EVLOOP_READ_BUFSIZE);
    }
    return evio_read_once(io);
}

int evio_read_remain(evio_t* io) {
    int remain = io->readbuf.tail - io->readbuf.head;
    if (remain > 0) {
        void* buf = io->readbuf.base + io->readbuf.head;
        io->readbuf.head = io->readbuf.tail = 0;
        evio_read_cb(io, buf, remain);
    }
    return remain;
}

evio_t* evio_read_raw(evloop_t* loop, int fd, evio_cb read_cb) {
    evio_t* io = evio_get(loop, fd);
    assert(io != NULL);
    evio_add(io, read_cb, EV_READ);
    return io;
}

//-----------------unpack---------------------------------------------
void evio_set_unpack(evio_t* io, unpack_setting_t* setting) {
    evio_unset_unpack(io);
    if (setting == NULL)
        return;

    io->unpack_setting = setting;
    if (io->unpack_setting->package_max_length == 0) {
        io->unpack_setting->package_max_length = DEFAULT_PACKAGE_MAX_LENGTH;
    }
    if (io->unpack_setting->mode == UNPACK_BY_FIXED_LENGTH) {
        assert(io->unpack_setting->fixed_length != 0 &&
               io->unpack_setting->fixed_length <= io->unpack_setting->package_max_length);
    } else if (io->unpack_setting->mode == UNPACK_BY_DELIMITER) {
        if (io->unpack_setting->delimiter_bytes == 0) {
            io->unpack_setting->delimiter_bytes = strlen((char*)io->unpack_setting->delimiter);
        }
    } else if (io->unpack_setting->mode == UNPACK_BY_LENGTH_FIELD) {
        assert(io->unpack_setting->body_offset >=
               io->unpack_setting->length_field_offset + io->unpack_setting->length_field_bytes);
    }

    // NOTE: unpack must have own readbuf
    if (io->unpack_setting->mode == UNPACK_BY_FIXED_LENGTH) {
        io->readbuf.len = io->unpack_setting->fixed_length;
    } else {
        io->readbuf.len = MIN(EVLOOP_READ_BUFSIZE, io->unpack_setting->package_max_length);
    }
    io->max_read_bufsize = io->unpack_setting->package_max_length;
    evio_alloc_readbuf(io, io->readbuf.len);
}

void evio_unset_unpack(evio_t* io) {
    if (io->unpack_setting) {
        io->unpack_setting = NULL;
        // NOTE: unpack has own readbuf
        evio_free_readbuf(io);
    }
}

//-----------------upstream---------------------------------------------
void evio_read_upstream(evio_t* io) {
    evio_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        evio_read(io);
        evio_read(upstream_io);
    }
}

void evio_read_upstream_on_write_complete(evio_t* io, const void* buf, int writebytes) {
    evio_t* upstream_io = io->upstream_io;
    if (upstream_io && evio_write_is_complete(io)) {
        evio_setcb_write(io, NULL);
        evio_read(upstream_io);
    }
}

void evio_write_upstream(evio_t* io, void* buf, int bytes) {
    evio_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        int nwrite = evio_write(upstream_io, buf, bytes);
        // if (!evio_write_is_complete(upstream_io)) {
        if (nwrite >= 0 && nwrite < bytes) {
            evio_read_stop(io);
            evio_setcb_write(upstream_io, evio_read_upstream_on_write_complete);
        }
    }
}

void evio_close_upstream(evio_t* io) {
    evio_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        evio_close(upstream_io);
    }
}

void evio_setup_upstream(evio_t* io1, evio_t* io2) {
    io1->upstream_io = io2;
    io2->upstream_io = io1;
}

evio_t* evio_get_upstream(evio_t* io) {
    return io->upstream_io;
}

evio_t* evio_setup_tcp_upstream(evio_t* io, const char* host, int port, int ssl) {
    evio_t* upstream_io = evio_create_socket(io->loop, host, port, EIO_TYPE_TCP, EIO_CLIENT_SIDE);
    if (upstream_io == NULL)
        return NULL;
    if (ssl)
        // evio_enable_ssl(upstream_io);
        evio_setup_upstream(io, upstream_io);
    evio_setcb_read(io, evio_write_upstream);
    evio_setcb_read(upstream_io, evio_write_upstream);
    evio_setcb_close(io, evio_close_upstream);
    evio_setcb_close(upstream_io, evio_close_upstream);
    evio_setcb_connect(upstream_io, evio_read_upstream);
    evio_connect(upstream_io);
    return upstream_io;
}

evio_t* evio_setup_udp_upstream(evio_t* io, const char* host, int port) {
    evio_t* upstream_io = evio_create_socket(io->loop, host, port, EIO_TYPE_UDP, EIO_CLIENT_SIDE);
    if (upstream_io == NULL)
        return NULL;
    evio_setup_upstream(io, upstream_io);
    evio_setcb_read(io, evio_write_upstream);
    evio_setcb_read(upstream_io, evio_write_upstream);
    evio_read_upstream(io);
    return upstream_io;
}
