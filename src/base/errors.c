#include "errors.h"

static void err_doit(int, const char *, va_list);

/**
 * Nonfatal error related to system call
 * Print message and return
 * 与系统调用相关的非严重错误，打印信息并返回。
 */
void err_ret(const char *fmt, ...) {
    va_list params;

    va_start(params, fmt);
    err_doit(1, fmt, params);
    va_end(params);
    return;
}

/**
 * Fatal error related to system call
 * Print message and terminate
 * 与系统调用相关的严重错误，打印信息并终止程序。
 */
void err_sys(const char *fmt, ...) {
    va_list params;

    va_start(params, fmt);
    err_doit(1, fmt, params);
    va_end(params);
    exit(1);
}

/**
 * Nonfatal error unrelated to system call
 * Print message and return
 * 与系统调用不相关的非严重错误，打印信息并返回。
 */
void err_msg(const char *fmt, ...) {
    va_list params;

    va_start(params, fmt);
    err_doit(0, fmt, params);
    va_end(params);
    return;
}

/**
 * Fatal error unrelated to system call
 * Print message and terminate
 * 与系统调用不相关的严重错误，打印信息并终止程序。
 */
void err_quit(const char *fmt, ...) {
    va_list params;

    va_start(params, fmt);
    err_doit(0, fmt, params);
    va_end(params);
    exit(1);
}

static void err_doit(int errnoflag, const char *fmt, va_list params) {
    int errno_save;
    char buf[MAXLINE + 1];
    errno_save = errno; /* value caller might want printed */

    vsnprintf(buf, MAXLINE, fmt, params); /* safe */

    int n = strlen(buf);
    if (errnoflag) {
        snprintf(buf + n, MAXLINE - n, " (%s)", strerror(errno_save));
    }
    strcat(buf, "\n");

    fflush(stdout); /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(stderr);

    return;
}

// errcode => errmsg
const char *hv_strerror(int err) {
    if (err > 0 && err <= SYS_NERR) {
        return strerror(err);
    }

    switch (err) {
#define F(errcode, name, errmsg) \
    case errcode:                \
        return errmsg;
        FOREACH_ERR(F)
#undef F
    default:
        return "Undefined error";
    }
}

/* Wrapper around strerror to handle case where it returns NULL. */
const char *safe_strerror(int errnum)
{
	const char *s = strerror(errnum);
	return (s != NULL) ? s : "Unknown error";
}