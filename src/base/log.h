/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H_
#define LOG_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define LOG_VERSION "0.1.0"

typedef struct {
    va_list ap;
    const char* fmt;
    const char* file;
    struct tm* time;
    void* udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event* ev);
typedef void (*log_LockFn)(bool lock, void* udata);

enum { _LOG_TRACE,
       _LOG_DEBUG,
       _LOG_INFO,
       _LOG_WARN,
       _LOG_ERROR,
       _LOG_FATAL };

#define log_trace(...) log_log(_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) log_log(_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) log_log(_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#define log_set_trace() log_set_level(_LOG_TRACE)
#define log_set_debug() log_set_level(_LOG_DEBUG)
#define log_set_info() log_set_level(_LOG_INFO)
#define log_set_warn() log_set_level(_LOG_WARN)
#define log_set_error() log_set_level(_LOG_ERROR)
#define log_set_fatal() log_set_level(_LOG_FATAL)

const char* log_level_string(int level);
void log_set_lock(log_LockFn fn, void* udata);
void log_set_level(int level);
void log_set_quiet(bool enable);
int log_add_callback(log_LogFn fn, void* udata, int level);
int log_add_fp(FILE* fp, int level);
void log_log(int level, const char* file, int line, const char* fmt, ...);

/* Message structure used by user */
struct message {
    int key;
    const char* str;
};

const char* lookup_msg(const struct message* mz, int kz);

#endif