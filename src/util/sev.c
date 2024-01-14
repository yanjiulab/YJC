#include "sev.h"

/* Definitions */
static int sev_nhandlers = 0;

static struct sev_handler sev_handlers[NHANDLERS];

// static struct sev_handler {
//     int fd;        /* File descriptor				 */
//     sev_cb func; /* Function to call with &fd_set */
// } sev_handlers[NHANDLERS];

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

/* the code below implements a callout queue */
static int id = 0;
static struct timeout_q *Q = 0; /* pointer to the beginning of timeout queue */

struct timeout_q {
    struct timeout_q *next; /* next event */
    int id;
    cfunc_t func; /* function to call */
    void *data;   /* func's data */
    int time;     /* time offset to next event*/
};

#if 0
#define CALLOUT_DEBUG 1
#endif /* 0 */
// static void print_Q(void);

void callout_init() { Q = (struct timeout_q *)0; }

void free_all_callouts() {
    struct timeout_q *p;

    while (Q) {
        p = Q;
        Q = Q->next;
        free(p);
    }
}

/*
 * elapsed_time seconds have passed; perform all the events that should
 * happen.
 */
void age_callout_queue(int elapsed_time) {
    struct timeout_q *ptr, *expQ;

#ifdef CALLOUT_DEBUG
    // log_debug("aging queue (elapsed time %d):", elapsed_time);
    print_Q();
#endif

    expQ = Q;
    ptr = NULL;

    while (Q) {
        if (Q->time > elapsed_time) {
            Q->time -= elapsed_time;
            if (ptr) {
                ptr->next = NULL;
                break;
            }
            return;
        } else {
            elapsed_time -= Q->time;
            ptr = Q;
            Q = Q->next;
        }
    }

    /* handle queue of expired timers */
    while (expQ) {
        ptr = expQ;
        if (ptr->func)
            ptr->func(ptr->data);
        expQ = expQ->next;
        free(ptr);
    }
}

/*
 * Return in how many seconds age_callout_queue() would like to be called.
 * Return -1 if there are no events pending.
 */
int timer_nextTimer() {
    if (Q) {
        if (Q->time < 0) {
            // log_warn("timer_nextTimer top of queue says %d", Q->time);
            return 0;
        }
        return Q->time;
    }
    return -1;
}

/*
 * sets the timer
 */
int timer_setTimer(int delay, cfunc_t action, void *data) {
    struct timeout_q *ptr, *node, *prev;

#ifdef CALLOUT_DEBUG
    // log_debug("setting timer:");
    print_Q();
#endif

    /* create a node */
    node = (struct timeout_q *)calloc(1, sizeof(struct timeout_q));
    if (!node) {
        // log_error("Failed calloc() in timer_settimer\n");
        return -1;
    }
    node->func = action;
    node->data = data;
    node->time = delay;
    node->next = 0;
    node->id = ++id;

    prev = ptr = Q;
    /*2013-0618-1225::bug fixed by dongfh*/
    if (node->id == 0)
        node->id = ++id;
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
#if 0
		print_Q();
#endif
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
#if 0
    print_Q();
#endif
    return node->id;
}

/* returns the time until the timer is scheduled */
int timer_leftTimer(int timer_id) {
    struct timeout_q *ptr;
    int left = 0;

    if (!timer_id)
        return -1;

    for (ptr = Q; ptr; ptr = ptr->next) {
        left += ptr->time;
        if (ptr->id == timer_id)
            return left;
    }
    return -1;
}

/* clears the associated timer */
void timer_clearTimer(int timer_id) {
    struct timeout_q *ptr, *prev;

    if (!timer_id)
        return;

    prev = ptr = Q;
#if 0        
    print_Q();
#endif
    while (ptr) {
        if (ptr->id == timer_id) {
            /* got the right node */

            /* unlink it from the queue */
            if (ptr == Q)
                Q = Q->next;
            else
                prev->next = ptr->next;

            /* increment next node if any */
            if (ptr->next != 0)
                (ptr->next)->time += ptr->time;
            /*
                    if (ptr->data)    /已经改为unsigned int 类型 ，无须释放/
                    free(ptr->data);
            */
            free(ptr);
#if 0
	    print_Q();
#endif
            return;
        }
        prev = ptr;
        ptr = ptr->next;
    }
#if 0
    print_Q();
#endif
}

int timer_resetTimer(int timer_id, int delay) {
    struct timeout_q *ptr, *node, *prev;

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

void print_Q() {
    struct timeout_q *ptr;

    // for (ptr = Q; ptr; ptr = ptr->next) log_debug("(%d,%d) ", ptr->id, ptr->time);
}

int ser_register(int fd, sev_cb func) {
    if (sev_nhandlers >= NHANDLERS) {
        return -1;
    }

    sev_handlers[sev_nhandlers].fd = fd;
    sev_handlers[sev_nhandlers++].func = func;
    return 0;
}

void sev_runloop() {
    struct timeval tv, difftime, curtime, lasttime, *timeout;
    fd_set rfds, allset;
    int nfds = 0, n, i, secs, ch;
    char *tmp;

    difftime.tv_usec = 0;
    gettimeofday(&curtime, NULL);
    lasttime = curtime;

    /* Main loop */
    while (1) {
        memcpy(&rfds, &allset, sizeof(rfds));
        secs = timer_nextTimer();
        // log_trace("[TIMEOUT: %2d secs]  > ", secs);

        if (secs == -1)
            timeout = NULL;
        else {
            timeout = &tv;
            timeout->tv_sec = secs;
            timeout->tv_usec = 0;
        }

        gettimeofday(&curtime, NULL);

        if ((n = select(nfds, &rfds, NULL, NULL, timeout)) < 0) {
            // printf("select<0\n");
            if (errno != EINTR) /* SIGALRM is expected */
                // log_warn("select failed");
                continue;
        }

        do {
            /*
             * If the select timed out, then there's no other
             * activity to account for and we don't need to
             * call gettimeofday.
             */
            if (n == 0) {
                curtime.tv_sec = lasttime.tv_sec + secs;
                curtime.tv_usec = lasttime.tv_usec;
                n = -1; /* don't do this next time through the loop */
            } else {
                gettimeofday(&curtime, NULL);
            }

            difftime.tv_sec = curtime.tv_sec - lasttime.tv_sec;
            difftime.tv_usec += curtime.tv_usec - lasttime.tv_usec;

            while (difftime.tv_usec > 1000000) {
                difftime.tv_sec++;
                difftime.tv_usec -= 1000000;
            }

            if (difftime.tv_usec < 0) {
                difftime.tv_sec--;
                difftime.tv_usec += 1000000;
            }

            lasttime = curtime;

            if (secs == 0 || difftime.tv_sec > 0) {
                age_callout_queue(difftime.tv_sec);
            }

            secs = -1;
        } while (difftime.tv_sec > 0);

        /* Handle sockets */
        if (n > 0) {
            for (i = 0; i < sev_nhandlers; i++) {
                if (FD_ISSET(sev_handlers[i].fd, &rfds)) {
                    (*sev_handlers[i].func)(sev_handlers[i].fd, &rfds);
                }
            }
        }

    } /* Main loop */
}