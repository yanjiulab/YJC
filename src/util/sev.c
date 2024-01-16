#include "sev.h"

#include "log.h"
/* Definitions */
static int id = 0;

static struct timeval timeval_adjust(struct timeval a) {
    while (a.tv_usec >= TIMER_SECOND_MICRO) {
        a.tv_usec -= TIMER_SECOND_MICRO;
        a.tv_sec++;
    }
    while (a.tv_usec < 0) {
        a.tv_usec += TIMER_SECOND_MICRO;
        a.tv_sec--;
    }
    if (a.tv_sec < 0) /* Change negative timeouts to 0. */
        a.tv_sec = a.tv_usec = 0;
    return a;
}

static struct timeval timeval_subtract(struct timeval a, struct timeval b) {
    struct timeval ret;
    ret.tv_usec = a.tv_usec - b.tv_usec;
    ret.tv_sec = a.tv_sec - b.tv_sec;
    return timeval_adjust(ret);
}

// evtimer
void evtimer_add(evloop_t* loop, evtimer_cb cb, void* data, uint32_t etimeout_ms) {
    struct evtimer *ptr, *node, *prev;

    /* create a node */
    node = (struct evtimer*)calloc(1, sizeof(struct evtimer));
    if (!node) {
        printf("Failed calloc() in timer_settimer\n");
        return -1;
    }
    node->loop = loop;
    node->func = cb;
    node->data = data;
    node->time = etimeout_ms;
    node->next = 0;
    node->id = ++id;

    prev = ptr = loop->timers;

    if (node->id == 0)
        node->id = ++id;
    /* insert node in the queue */
    /* if the queue is empty, insert the node and return */
    if (!loop->timers)
        loop->timers = node;
    else {
        /* chase the pointer looking for the right place */
        while (ptr) {
            if (etimeout_ms < ptr->time) {
                /* right place */

                node->next = ptr;
                if (ptr == loop->timers)
                    loop->timers = node;
                else
                    prev->next = node;
                ptr->time -= node->time;
                return node->id;
            } else {
                /* keep moving */
                etimeout_ms -= ptr->time;
                node->time = etimeout_ms;
                prev = ptr;
                ptr = ptr->next;
            }
        }
        prev->next = node;
    }
    return node->id;
}

void evtimer_del(evloop_t* loop, int timer_id) {
    struct evtimer *ptr, *prev;

    if (!timer_id)
        return;

    prev = ptr = loop->timers;

    while (ptr) {
        if (ptr->id == timer_id) {
            /* got the right node */

            /* unlink it from the queue */
            if (ptr == loop->timers)
                loop->timers = loop->timers->next;
            else
                prev->next = ptr->next;

            /* increment next node if any */
            if (ptr->next != 0)
                (ptr->next)->time += ptr->time;

            free(ptr);
            return;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}

void evtimer_clean(evloop_t* loop) {
    struct evtimer* p;
    while (loop->timers) {
        p = loop->timers;
        loop->timers = loop->timers->next;
        free(p);
    }
}

// Return in how many ms evtimer_callout() would like to be called.
// Return -1 if there are no events pending.
int evtimer_next(evloop_t* loop) {
    if (loop->timers) {
        if (loop->timers->time < 0) {
            return 0;
        }
        return loop->timers->time;
    }
    return -1;
}

/* elapsed_time seconds have passed; perform all the events that should happen. */
void evtimer_callout(evloop_t* loop, int elapsed_time) {
    struct evtimer *ptr, *expQ;

    expQ = loop->timers;
    ptr = NULL;

    while (loop->timers) {
        if (loop->timers->time > elapsed_time) {
            loop->timers->time -= elapsed_time;
            if (ptr) {
                ptr->next = NULL;
                break;
            }
            return;
        } else {
            elapsed_time -= loop->timers->time;
            ptr = loop->timers;
            loop->timers = loop->timers->next;
        }
    }

    /* handle queue of expired timers */
    while (expQ) {
        ptr = expQ;
        if (ptr->func) {
            ptr->func(ptr, ptr->data);
        }
        expQ = expQ->next;
        free(ptr);
    }
}

/* returns the time until the timer is scheduled */
int evtimer_left(evloop_t* loop, int timer_id) {
    struct evtimer* ptr;
    int left = 0;

    if (!timer_id)
        return -1;

    for (ptr = loop->timers; ptr; ptr = ptr->next) {
        left += ptr->time;
        if (ptr->id == timer_id)
            return left;
    }
    return -1;
}

int evtimer_reset(evloop_t* loop, int timer_id, int delay) {
    struct evtimer *ptr, *node, *prev;
    struct evtimer* Q = loop->timers;
    if (!timer_id)
        return;

    prev = ptr = Q;
    while (ptr) {
        if (ptr->id == timer_id) {
            /* got the right node */
            ptr->time = delay;
            node = ptr;

            /* unlink it from the queue */
            if (ptr == Q)
                Q = Q->next;
            else
                prev->next = ptr->next;
            if (ptr->next != 0)
                (ptr->next)->time += ptr->time;
        }
        prev = ptr;
        ptr = ptr->next;
    }

    if (!node)
        return;

    // set
    prev = ptr = Q;
    /* insert node in the queue */

    /* if the queue is empty, insert the node and return */
    if (!Q)
        Q = node;
    else {
        /* chase the pointer looking for the right place */
        while (ptr) {
            if (delay < ptr->time) {
                /* right place */
                node->next = ptr;
                if (ptr == Q)
                    Q = node;
                else
                    prev->next = node;
                ptr->time -= node->time;
                return node->id;
            } else {
                /* keep moving */
                delay -= ptr->time;
                node->time = delay;
                prev = ptr;
                ptr = ptr->next;
            }
        }
        prev->next = node;
    }
    return node->id;
}

// evio
int evio_add(evloop_t* loop, int fd, evio_cb cb) {
    if (loop->nios >= loop->max_ios) {
        return -1;
    }
    loop->ios[loop->nios].fd = fd;
    loop->ios[loop->nios++].func = cb;

    FD_SET(loop->ios[loop->nios].fd, &loop->allset);
    if (loop->ios[loop->nios].fd >= loop->nfds) {
        loop->nfds = loop->ios[loop->nios].fd + 1;
    }

    return 0;
}

// evloop
evloop_t* evloop_new(int max) {
    evloop_t* loop = calloc(1, sizeof(evloop_t));
    loop->status = EVLOOP_STATUS_STOP;
    loop->max_ios = max;
    loop->ios = calloc(max, sizeof(struct evio));
    loop->nios = 0;
    FD_ZERO(&loop->allset);
    FD_ZERO(&loop->rfds);
    loop->timers = NULL;
    return loop;
}

void evloop_free(evloop_t** pp) {
    if (pp && *pp) {
        // clear timers
        evtimer_clean(*pp);
        // clear ios
        // evio_clean(*pp);
        free(*pp);
        *pp = NULL;
    }
}

int evloop_run(evloop_t* loop) {
    if (loop == NULL)
        return -1;
    if (loop->status == EVLOOP_STATUS_RUNNING)
        return -2;

    loop->status = EVLOOP_STATUS_RUNNING;

    struct timeval tv, difftime, curtime, lasttime, *timeout;
    int n, i, ms, diffms;
    difftime.tv_usec = 0;
    difftime.tv_sec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;

    /* Main loop */
    while (loop->status != EVLOOP_STATUS_STOP) {
        memcpy(&loop->rfds, &loop->allset, sizeof(loop->rfds));
        ms = evtimer_next(loop);

        if (ms == -1)
            timeout = NULL;
        else {
            timeout = &tv;
            timeout->tv_sec = ms / 1000;
            timeout->tv_usec = (ms % 1000) * 1000;
        }

        gettimeofday(&curtime, NULL);

        if ((n = select(loop->nfds, &loop->rfds, NULL, NULL, timeout)) < 0) {
            if (errno != EINTR) /* SIGALRM is expected */
                printf("select failed\n");
            continue;
        }

        do {
            /*
             * If the select timed out, then there's no other
             * activity to account for and we don't need to
             * call gettimeofday.
             */
            if (n == 0) {
                curtime.tv_sec = lasttime.tv_sec + (ms / 1000);
                curtime.tv_usec = lasttime.tv_usec + ((ms % 1000) * 1000);
                n = -1; /* don't do this next time through the loop */
            } else {
                gettimeofday(&curtime, NULL);
            }

            difftime = timeval_subtract(curtime, lasttime);
            diffms = difftime.tv_sec * 1000 + difftime.tv_usec / 1000;
            lasttime = curtime;

            if (ms == 0 || (diffms) > 0) {
                evtimer_callout(loop, diffms);
            }

            ms = -1;
        } while (diffms > 0);

        /* Handle sockets */
        if (n > 0) {
            for (i = 0; i < loop->nios; i++) {
                if (FD_ISSET(loop->ios[i].fd, &loop->rfds)) {
                    (*loop->ios[i].func)(loop->ios[i].fd, &loop->rfds);
                }
            }
        }

    } /* Main loop */

    loop->status = EVLOOP_STATUS_STOP;

    evloop_free(&loop);

    return 0;
}