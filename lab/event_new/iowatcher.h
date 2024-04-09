#ifndef IO_WATCHER_H_
#define IO_WATCHER_H_

#include "sev.h"

int iowatcher_init(evloop_t* loop);
int iowatcher_cleanup(evloop_t* loop);
int iowatcher_add_event(evloop_t* loop, int fd, int events);
int iowatcher_del_event(evloop_t* loop, int fd, int events);
int iowatcher_poll_events(evloop_t* loop, int timeout);

#endif
