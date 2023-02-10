#include "str.h"
#include "test.h"

void test_str() {
    char *s = str("Hello %s\n", "World");
    printf(s);
    assert(str_hash("hello") == str_hash("hello"));

    printf("%d\n", str2int("1234"));
    printf("%x\n", str2int(" 0x1234"));
    printf("%f\n", str2double("12.23"));
    printf("%e\n", str2double("0.0101"));
}

void test_str_split() {
    char in[] = "www.baidu...com.www.baidu.com.www.baidu.com.www.baidu.com.www.baidu.com";
    char *out[40];
    int n = STR_SPLIT(in, out, ".");
    printf("split number %d\n", n);

    for (int i = 0; i < n; i++) {
        printf("%d:%s\n", i, out[i]);
    }
}

void test_str_trim() {
    char testStr1[] = "     We like helping out people          ";
    char testStr2[] = "     We like helping out people          ";
    char testStr3[] = "     We like helping out people          ";
    printf("|%s|\n", testStr1);
    printf("|%s|\n", str_ltrim(testStr1, ' '));
    printf("|%s|\n", testStr2);
    printf("|%s|\n", str_rtrim(testStr2, ' '));
    printf("|%s|\n", testStr3);
    printf("|%s|\n", str_trim(testStr3, ' '));
}

