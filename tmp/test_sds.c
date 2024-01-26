#include <stdio.h>

#include "limits.h"
#include "sds.h"
#include "test.h"
#include "testhelp.h"

#define UNUSED(x) (void)(x)
void test_sds() {
    printf("%d\n", sizeof(size_t));
    sds x = sdsnew("foo"), y;

    test_cond("Create a string and obtain the length",
              sdslen(x) == 3 && memcmp(x, "foo\0", 4) == 0);

    sdsfree(x);
    x = sdsnewlen("foo", 2);
    test_cond("Create a string with specified length",
              sdslen(x) == 2 && memcmp(x, "fo\0", 3) == 0);

    x = sdscat(x, "bar");
    test_cond("Strings concatenation",
              sdslen(x) == 5 && memcmp(x, "fobar\0", 6) == 0);

    x = sdscpy(x, "a");
    test_cond("sdscpy() against an originally longer string",
              sdslen(x) == 1 && memcmp(x, "a\0", 2) == 0);

    x = sdscpy(x, "xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
    test_cond("sdscpy() against an originally shorter string",
              sdslen(x) == 33 &&
                  memcmp(x, "xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0", 33) == 0);

    sdsfree(x);
    x = sdscatprintf(sdsempty(), "%d", 123);
    test_cond("sdscatprintf() seems working in the base case",
              sdslen(x) == 3 && memcmp(x, "123\0", 4) == 0);

    sdsfree(x);
    x = sdscatprintf(sdsempty(), "a%cb", 0);
    test_cond("sdscatprintf() seems working with \\0 inside of result",
              sdslen(x) == 3 && memcmp(x,
                                       "a\0"
                                       "b\0",
                                       4) == 0);

    {
        sdsfree(x);
        char etalon[1024 * 1024];
        for (size_t i = 0; i < sizeof(etalon); i++) {
            etalon[i] = '0';
        }
        x = sdscatprintf(sdsempty(), "%0*d", (int)sizeof(etalon), 0);
        test_cond("sdscatprintf() can print 1MB",
                  sdslen(x) == sizeof(etalon) && memcmp(x, etalon, sizeof(etalon)) == 0);
    }

    sdsfree(x);
    x = sdsnew("--");
    x = sdscatfmt(x, "Hello %s World %I,%I--", "Hi!", LLONG_MIN, LLONG_MAX);
    test_cond("sdscatfmt() seems working in the base case",
              sdslen(x) == 60 &&
                  memcmp(x,
                         "--Hello Hi! World -9223372036854775808,"
                         "9223372036854775807--",
                         60) == 0);
    printf("[%s]\n", x);

    sdsfree(x);
    x = sdsnew("--");
    x = sdscatfmt(x, "%u,%U--", UINT_MAX, ULLONG_MAX);
    test_cond("sdscatfmt() seems working with unsigned numbers",
              sdslen(x) == 35 &&
                  memcmp(x, "--4294967295,18446744073709551615--", 35) == 0);

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " x");
    test_cond("sdstrim() works when all chars match",
              sdslen(x) == 0);

    sdsfree(x);
    x = sdsnew(" x ");
    sdstrim(x, " ");
    test_cond("sdstrim() works when a single char remains",
              sdslen(x) == 0 && x[0] == 'x');

    sdsfree(x);
    x = sdsnew("xxciaoyyy");
    sdstrim(x, "xy");
    test_cond("sdstrim() correctly trims characters",
              sdslen(x) == 4 && memcmp(x, "ciao\0", 5) == 0);

    y = sdsdup(x);
    sdsrange(y, 1, 1);
    test_cond("sdsrange(...,1,1)",
              sdslen(y) == 1 && memcmp(y, "i\0", 2) == 0);

    sdsfree(y);
    y = sdsdup(x);
    sdsrange(y, 1, -1);
    test_cond("sdsrange(...,1,-1)",
              sdslen(y) == 3 && memcmp(y, "iao\0", 4) == 0);

    sdsfree(y);
    y = sdsdup(x);
    sdsrange(y, -2, -1);
    test_cond("sdsrange(...,-2,-1)",
              sdslen(y) == 2 && memcmp(y, "ao\0", 3) == 0);

    sdsfree(y);
    y = sdsdup(x);
    sdsrange(y, 2, 1);
    test_cond("sdsrange(...,2,1)",
              sdslen(y) == 0 && memcmp(y, "\0", 1) == 0);

    sdsfree(y);
    y = sdsdup(x);
    sdsrange(y, 1, 100);
    test_cond("sdsrange(...,1,100)",
              sdslen(y) == 3 && memcmp(y, "iao\0", 4) == 0);

    sdsfree(y);
    y = sdsdup(x);
    sdsrange(y, 100, 100);
    test_cond("sdsrange(...,100,100)",
              sdslen(y) == 0 && memcmp(y, "\0", 1) == 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("foo");
    y = sdsnew("foa");
    test_cond("sdscmp(foo,foa)", sdscmp(x, y) > 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("bar");
    y = sdsnew("bar");
    test_cond("sdscmp(bar,bar)", sdscmp(x, y) == 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnew("aar");
    y = sdsnew("bar");
    test_cond("sdscmp(bar,bar)", sdscmp(x, y) < 0);

    sdsfree(y);
    sdsfree(x);
    x = sdsnewlen("\a\n\0foo\r", 7);
    y = sdscatrepr(sdsempty(), x, sdslen(x));
    test_cond("sdscatrepr(...data...)",
              memcmp(y, "\"\\a\\n\\x00foo\\r\"", 15) == 0);

    {
        char* p;
        int step = 10, j, i;

        sdsfree(x);
        sdsfree(y);
        x = sdsnew("0");
        test_cond("sdsnew() free/len buffers", sdslen(x) == 1 && sdsavail(x) == 0);
        ;

        /* Run the test a few times in order to hit the first two
         * SDS header types. */
        for (i = 0; i < 10; i++) {
            int oldlen = sdslen(x);
            x = sdsMakeRoomFor(x, step);
            int type = x[-1] & SDS_TYPE_MASK;

            test_cond("sdsMakeRoomFor() len", sdslen(x) == oldlen);
            if (type != SDS_TYPE_5) {
                test_cond("sdsMakeRoomFor() free", sdsavail(x) >= step);
            }
            p = x + oldlen;
            for (j = 0; j < step; j++) {
                p[j] = 'A' + j;
            }
            sdsIncrLen(x, step);
        }
        test_cond("sdsMakeRoomFor() content",
                  memcmp("0ABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJABCDEFGHIJ", x, 101) == 0);
        test_cond("sdsMakeRoomFor() final length", sdslen(x) == 101);

        sdsfree(x);
    }

    test_report();
    return 0;
}