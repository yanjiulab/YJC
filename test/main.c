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
        // cmocka_unit_test(null_test_success),
        // cmocka_unit_test(test_log),
        // cmocka_unit_test(test_ini),
        // cmocka_unit_test(test_vector),
        // cmocka_unit_test(test_list),
        // cmocka_unit_test(test_map),
        // cmocka_unit_test(test_ifi),
        // cmocka_unit_test(test_str_split),
        // cmocka_unit_test(test_str_trim),
        // cmocka_unit_test(test_timer),
        // cmocka_unit_test(test_name),
        // cmocka_unit_test(test_sock),
        cmocka_unit_test(test_ev),
        // cmocka_unit_test(test_json),
    };

    /* Run the tests */
    return cmocka_run_group_tests(tests, NULL, NULL);
}