#include "str.h"
#include "test.h"

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