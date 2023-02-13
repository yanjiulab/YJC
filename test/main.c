#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "cmocka.h"

/* include here your files that contain test functions */
#include "test.h"

/**
 * Test runner function
 */
int main(void) {
    /**
     * Insert here your test functions
     */
    const struct CMUnitTest tests[] = {
        // utils
        cmocka_unit_test(test_log),
        cmocka_unit_test(test_ini),
        cmocka_unit_test(test_base64_md5_sha1),
        cmocka_unit_test(test_str),
        cmocka_unit_test(test_str_split),
        cmocka_unit_test(test_str_trim),
        cmocka_unit_test(test_list),
        cmocka_unit_test(test_map),
        cmocka_unit_test(test_heap),
        cmocka_unit_test(test_rbtree),
        cmocka_unit_test(test_container),
        cmocka_unit_test(test_json),
        // cmocka_unit_test(test_vector),
        // cmocka_unit_test(test_base),
        // cmocka_unit_test(null_test_success),
        // cmocka_unit_test(test_ifi),
        // cmocka_unit_test(test_timer),
        // cmocka_unit_test(test_name),
        // cmocka_unit_test(test_sock),
        // cmocka_unit_test(test_inet),
        // cmocka_unit_test(test_ev),
        // cmocka_unit_test(test_json),
        // cmocka_unit_test(test_shell),
        // cmocka_unit_test(test_ev_my),
        // cmocka_unit_test(test_epoll),
        // cmocka_unit_test(test_math),
        cmocka_unit_test(test_datetime),

    };

    /* Run the tests */
    return cmocka_run_group_tests(tests, NULL, NULL);
}