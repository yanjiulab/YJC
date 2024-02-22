#ifndef PROC_H_
#define PROC_H_

#include "platform.h"

typedef void (*procedure_t)(void* userdata);

typedef struct proc_ctx_s {
    pid_t pid;
    time_t start_time;
    int spawn_cnt;
    procedure_t init;
    void* init_userdata;
    procedure_t proc;
    void* proc_userdata;
    procedure_t exit;
    void* exit_userdata;
} proc_ctx_t;

static inline void proc_run(proc_ctx_t* ctx) {
    if (ctx->init) {
        ctx->init(ctx->init_userdata);
    }
    if (ctx->proc) {
        ctx->proc(ctx->proc_userdata);
    }
    if (ctx->exit) {
        ctx->exit(ctx->exit_userdata);
    }
}

static inline int proc_spawn(proc_ctx_t* ctx) {
    ++ctx->spawn_cnt;
    ctx->start_time = time(NULL);
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        // child process
        ctx->pid = getpid();
        proc_run(ctx);
        exit(0);
    } else if (pid > 0) {
        // parent process
        ctx->pid = pid;
    }
    return pid;
}

#endif // PROC_H_
