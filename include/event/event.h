/* Asynchronous Event Notification */

#ifndef EVENT_H
#define EVENT_H
struct event_base {};

struct event {};

struct event_base *event_base_new();
struct event *event_new(struct event_base *, int);
void event_add(struct event *);
void event_base_dispatch(struct event_base base);
void event_loopbreak();
void event_loopexit();

#endif