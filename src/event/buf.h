#ifndef EV_BUF_H_
#define EV_BUF_H_

#include "base.h" // for EV_ALLOC, EV_FREE
#include "defs.h" // for MAX

typedef struct buf_s {
    char* base;
    size_t len;
} buf_t;

typedef struct offset_buf_s {
    char* base;
    size_t len;
    size_t offset;
} offset_buf_t;

typedef struct fifo_buf_s {
    char* base;
    size_t len;
    size_t head;
    size_t tail;
} fifo_buf_t;

#endif // EV_BUF_H_
