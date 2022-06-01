#include "timer.h"

/* the code below implements a callout queue */
static int id = 0;
static struct timeout_q *Q = 0; /* pointer to the beginning of timeout queue */

struct timeout_q {
    struct timeout_q *next; /* next event */
    int id;
    cfunc_t func;      /* function to call */
    unsigned int data; /* func's data */
    int time;          /* time offset to next event*/
};

/*
 * elapsed_time seconds have passed; perform all the events that should
 * happen.
 */
void timer_elapse(int elapsed_time) {
    struct timeout_q *ptr, *expQ;

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
 * Return in how many seconds timer_elapse() would like to be called.
 * Return -1 if there are no events pending.
 */
int timer_next() {
    if (Q) {
        if (Q->time < 0) {
            printf("timer_next top of queue says %d\n", Q->time);
            return 0;
        }
        return Q->time;
    }
    return -1;
}

/*
 * sets the timer
 */
int timer_set(int delay, cfunc_t action, unsigned int data) {
    struct timeout_q *ptr, *node, *prev;

    /* create a node */
    node = (struct timeout_q *)calloc(1, sizeof(struct timeout_q));
    if (!node) {
        printf("Failed calloc() in timer_set\n");
        return -1;
    }
    node->func = action;
    node->data = data;
    node->time = delay;
    node->next = 0;
    node->id = ++id;

    prev = ptr = Q;

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

/* returns the time until the timer is scheduled */
int timer_left(int timer_id) {
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
void timer_clear(int timer_id) {
    struct timeout_q *ptr, *prev;

    if (!timer_id)
        return;

    prev = ptr = Q;

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

            return;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}

void timer_print() {
    struct timeout_q *ptr;

    for (ptr = Q; ptr; ptr = ptr->next)
        printf("(%d,%d) ", ptr->id, ptr->time);

    printf("\n");
}

void timer_free() {
    struct timeout_q *p;

    while (Q) {
        p = Q;
        Q = Q->next;
        free(p);
    }
}