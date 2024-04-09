#ifndef IO_WATCHER_H_
#define IO_WATCHER_H_

#include "eventloop.h"

#if !defined(EVENT_SELECT) && !defined(EVENT_POLL) && !defined(EVENT_EPOLL) && !defined(EVENT_KQUEUE)
#ifdef OS_LINUX
#define EVENT_EPOLL
#elif defined(OS_MAC)
#define EVENT_KQUEUE
#elif defined(OS_BSD)
#define EVENT_KQUEUE
#else
#define EVENT_SELECT
#endif
#endif

int iowatcher_init(evloop_t* loop);
int iowatcher_cleanup(evloop_t* loop);
int iowatcher_add_event(evloop_t* loop, int fd, int events);
int iowatcher_del_event(evloop_t* loop, int fd, int events);
int iowatcher_poll_events(evloop_t* loop, int timeout);

#endif
