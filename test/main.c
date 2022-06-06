#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "cmocka.h"
#include "hashmap.h"
#include "ini.h"
#include "linklist.h"
#include "log.h"
#include "vector.h"

/* include here your files that contain test functions */

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state) {
    /**
     * If you want to know how to use cmocka, please refer to:
     * https://api.cmocka.org/group__cmocka__asserts.html
     */
    (void)state; /* unused */
}

static void test_log() {
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
}

static void test_ini() {
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
}

void test_vec() {
    // create vec
    vector(int) *i_vec_p = vec_new(int);
    printf("new i_vec_p: %p\n", i_vec_p);
    vector(char *) str_vec;
    vec_init(&str_vec);
    printf("new str_vec: %p\n", &str_vec);

    // add
    vec_push(i_vec_p, 1);
    vec_push(i_vec_p, 1);
    vec_push(i_vec_p, 2);
    vec_push(i_vec_p, 3);

    vec_insert(&str_vec, 0, "a");
    vec_insert(&str_vec, 1, "bc");
    vec_insert(&str_vec, 2, "def");
    vec_insert(&str_vec, 3, "1a3b");
    vec_insert(&str_vec, 4, "bc");

    // foreach
    int var, iter;
    vec_foreach(i_vec_p, var, iter) { printf("%d ", var); }
    printf("\n");

    char *str;
    vec_foreach(&str_vec, str, iter) { printf("%s ", str); }
    printf("\n");

    // del
    vec_pop(i_vec_p);
    vec_dump(i_vec_p, "%d");

    vec_remove(&str_vec, "bc");
    vec_dump(&str_vec, "%s");

    // destroy vec
    vec_deinit(&str_vec);
    vec_free(i_vec_p);
}

void test_list() {
    list *l = list_new();
    listnode_add(l, 1);
    listnode_add(l, 11);
    listnode_add(l, 12);
    listnode_add(l, 133);   
    listnode_add(l, "abc"); 
    list_dump(l, "%d");
    listnode_delete(l, 1);
    list_dump(l, "%d");


}

void test_map() {}

/**
 * Test runner function
 */
int main(void) {
    /**
     * Insert here your test functions
     */
    const struct CMUnitTest tests[] = {
        // cmocka_unit_test(null_test_success),
        // cmocka_unit_test(test_log),
        // cmocka_unit_test(test_ini),
        // cmocka_unit_test(test_vec),
        cmocka_unit_test(test_list),
    };

    /* Run the tests */
    return cmocka_run_group_tests(tests, NULL, NULL);
}