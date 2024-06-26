#include "iowatcher.h"

#ifdef EVENT_EPOLL
#include <sys/epoll.h>

#include "array.h"
#include "defs.h"
#include "event.h"
#include "platform.h"
#define EVENTS_INIT_SIZE 64

ARRAY_DECL(struct epoll_event, events);

typedef struct epoll_ctx_s {
    int epfd;
    struct events events;
} epoll_ctx_t;

int iowatcher_init(evloop_t* loop) {
    if (loop->iowatcher)
        return 0;
    epoll_ctx_t* epoll_ctx;
    EV_ALLOC_SIZEOF(epoll_ctx);
    epoll_ctx->epfd = epoll_create(EVENTS_INIT_SIZE);
    events_init(&epoll_ctx->events, EVENTS_INIT_SIZE);
    loop->iowatcher = epoll_ctx;
    return 0;
}

int iowatcher_cleanup(evloop_t* loop) {
    if (loop->iowatcher == NULL)
        return 0;
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    close(epoll_ctx->epfd);
    events_cleanup(&epoll_ctx->events);
    EV_FREE(loop->iowatcher);
    return 0;
}

int iowatcher_add_event(evloop_t* loop, int fd, int events) {
    if (loop->iowatcher == NULL) {
        iowatcher_init(loop);
    }
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    evio_t* io = loop->ios.ptr[fd];

    struct epoll_event ee;
    memset(&ee, 0, sizeof(ee));
    ee.data.fd = fd;
    // pre events
    if (io->events & EV_READ) {
        ee.events |= EPOLLIN;
    }
    if (io->events & EV_WRITE) {
        ee.events |= EPOLLOUT;
    }
    // now events
    if (events & EV_READ) {
        ee.events |= EPOLLIN;
    }
    if (events & EV_WRITE) {
        ee.events |= EPOLLOUT;
    }
    int op = io->events == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    epoll_ctl(epoll_ctx->epfd, op, fd, &ee);
    if (op == EPOLL_CTL_ADD) {
        if (epoll_ctx->events.size == epoll_ctx->events.maxsize) {
            events_double_resize(&epoll_ctx->events);
        }
        epoll_ctx->events.size++;
    }
    return 0;
}

int iowatcher_del_event(evloop_t* loop, int fd, int events) {
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    if (epoll_ctx == NULL)
        return 0;
    evio_t* io = loop->ios.ptr[fd];

    struct epoll_event ee;
    memset(&ee, 0, sizeof(ee));
    ee.data.fd = fd;
    // pre events
    if (io->events & EV_READ) {
        ee.events |= EPOLLIN;
    }
    if (io->events & EV_WRITE) {
        ee.events |= EPOLLOUT;
    }
    // now events
    if (events & EV_READ) {
        ee.events &= ~EPOLLIN;
    }
    if (events & EV_WRITE) {
        ee.events &= ~EPOLLOUT;
    }
    int op = ee.events == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    epoll_ctl(epoll_ctx->epfd, op, fd, &ee);
    if (op == EPOLL_CTL_DEL) {
        epoll_ctx->events.size--;
    }
    return 0;
}

int iowatcher_poll_events(evloop_t* loop, int timeout) {
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    if (epoll_ctx == NULL)
        return 0;
    if (epoll_ctx->events.size == 0)
        return 0;
    int nepoll = epoll_wait(epoll_ctx->epfd, epoll_ctx->events.ptr, epoll_ctx->events.size, timeout);
    if (nepoll < 0) {
        if (errno == EINTR) {
            return 0;
        }
        perror("epoll");
        return nepoll;
    }
    if (nepoll == 0)
        return 0;
    int nevents = 0;
    for (int i = 0; i < epoll_ctx->events.size; ++i) {
        struct epoll_event* ee = epoll_ctx->events.ptr + i;
        int fd = ee->data.fd;
        uint32_t revents = ee->events;
        if (revents) {
            ++nevents;
            evio_t* io = loop->ios.ptr[fd];
            if (io) {
                if (revents & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
                    io->revents |= EV_READ;
                }
                if (revents & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    io->revents |= EV_WRITE;
                }
                EVENT_PENDING(io);
            }
        }
        if (nevents == nepoll)
            break;
    }
    return nevents;
}
#endif
