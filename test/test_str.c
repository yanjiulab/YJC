#include "str.h"
#include "test.h"

// void test_str_split() {
//     printf("===== %s start =====\n", __func__);

//     char *a = "abc def ijk z";
//     size_t n;
//     char **rst = str_split(a, strlen(a), ' ', &n, 10);

//     for (size_t i = 0; i < n; i++) {
//         printf("%s\n", rst[i]);
//     }
//     str_split_free(rst, n);

//     printf("===== %s end =====\n", __func__);
// }

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

void test_str() {
    char *s = str("Hello %s\n", "World");
    printf(s);
    assert(str_hash("hello") == str_hash("hello"));

    char *in = "www.baidu.com";
    char *out[10];
    int n = str_split(in, out, ".");

    for (int i = 0; i < n; i++)
    {
        printf("%s\n", out[i]);
    }
    

    printf("%d\n", str2int("1234"));
    printf("%x\n", str2int(" 0x1234"));
    printf("%f\n", str2double("12.23"));
    printf("%e\n", str2double("0.0101"));
}