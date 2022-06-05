#include <net/ethernet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ini.h"
#include "log.h"
#include "str.h"
#include "timer.h"
#include "ifi.h"

void test_log() {
    printf("===== %s start =====\n", __func__);
    int level = LOG_TRACE;
    log_set_level(level);

    FILE *fp = fopen("log/app.log", "w");
    int file_level = LOG_WARN;
    log_add_fp(fp, file_level);

    log_trace("trace");
    log_debug("debug");
    log_info("info");
    log_warn("warn");
    log_error("error");
    log_fatal("fatal");

    printf("===== %s end =====\n", __func__);
}

void test_ini() {
    printf("===== %s start =====\n", __func__);
    ini_t *config = ini_load("config.ini");

    const char *name = ini_get(config, "owner", "name");
    if (name) {
        printf("name: %s\n", name);
    }

    const char *server = "default";
    int port = 80;

    ini_sget(config, "database", "server", NULL, &server);
    ini_sget(config, "database", "port", "%d", &port);

    printf("server: %s:%d\n", server, port);

    ini_free(config);

    printf("===== %s end =====\n", __func__);
}

void test_str_split() {
    printf("===== %s start =====\n", __func__);

    char *a = "abc def ijk z";
    size_t n;
    char **rst = str_split(a, strlen(a), ' ', &n, 10);

    for (size_t i = 0; i < n; i++) {
        printf("%s\n", rst[i]);
    }
    str_split_free(rst, n);

    printf("===== %s end =====\n", __func__);
}

void test_str_trim() {
    printf("===== %s start =====\n", __func__);
    char testStr1[] = "     We like helping out people          ";
    char testStr2[] = "     We like helping out people          ";
    char testStr3[] = "     We like helping out people          ";
    printf("|%s|\n", testStr1);
    printf("|%s|\n", str_ltrim(testStr1, ' '));
    printf("|%s|\n", testStr2);
    printf("|%s|\n", str_rtrim(testStr2, ' '));
    printf("|%s|\n", testStr3);
    printf("|%s|\n", str_trim(testStr3, ' '));

    printf("===== %s end =====\n", __func__);
}

void work() {
    printf("Its my time to work!\n");
}

void test_timer() {
    printf("===== %s start =====\n", __func__);
    timer_set(10, work, 0);
    timer_set(3, work, 0);
    timer_set(5, work, 0);
    timer_set(9, work, 0);

    timer_print();

    timer_left(4);
    printf("timer_left(4): %d\n", timer_left(4));
    timer_elapse(5);
    timer_print();
    printf("next event will happen after %d seconds\n", timer_next());
    timer_clear(4);
    timer_print();
    timer_free();
    printf("===== %s end =====\n", __func__);
}


void test_net_ipa() {


}

void test_net_ifi() {
    //struct ifi_info *ifi;

}
int main(int argc, char *argv[]) {
    printf("Hello YJC\n");

    for (int i = 0; i < argc; i++) {
        printf("args(%d/%d): %s\n", i, argc, argv[i]);
    }

    // test_log();
    // test_ini();
    // test_str_split();
    // test_timer();
    test_net_ifi();

    return 0;
}