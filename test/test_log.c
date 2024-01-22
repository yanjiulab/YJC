#include "log.h"
#include "test.h"

void test_log() {
    int level = _LOG_TRACE;
    log_set_level(level);
    log_set_debug();
    FILE *fp = fopen("log/app.log", "w");
    int file_level = _LOG_WARN;
    log_add_fp(fp, file_level);

    log_trace("trace");
    log_debug("debug");
    log_info("info");
    log_warn("warn");
    log_error("error");
    log_fatal("fatal");
}