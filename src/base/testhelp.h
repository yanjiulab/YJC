#ifndef __TESTHELP_H
#define __TESTHELP_H

static int __failed_tests = 0;
static int __test_num = 0;

#define test_cond(descr, _c)                                                \
    do {                                                                    \
        __test_num++;                                                       \
        printf("%d - %s: ", __test_num, descr);                             \
        if (_c)                                                             \
            printf("\x1b[32mPASSED\x1b[0m\n");                              \
        else {                                                              \
            printf("\x1b[31mFAILED\x1b[0m at %s:%d\n", __FILE__, __LINE__); \
            __failed_tests++;                                               \
        }                                                                   \
    } while (0);

#define test_report()                                                 \
    do {                                                              \
        printf("%d test(s) run, %d passed, %d failed\n", __test_num,  \
               __test_num - __failed_tests, __failed_tests);          \
        if (__failed_tests) {                                         \
            printf("=== WARNING === We have failed tests here...\n"); \
        }                                                             \
    } while (0);

#endif