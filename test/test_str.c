#include "str.h"
#include "test.h"

void test_str() {

    // str str;

    char *s = str("Hello %s\n", "World");
    printf(s);
    assert(str_hash("hello") == str_hash("hello"));
    printf("%s\n", str_replace(s, "World", "String!"));
    printf("%d\n", str2int("1234"));
    printf("%d\n", str2int(" 0x1234"));
    printf("%d\n", str2int(" 01234"));
    printf("%f\n", str2double("12.23"));
    printf("%e\n", str2double("0.0101"));
    printf("%d\n", str2bool("N"));
    printf("%d\n", str2bool("On"));
    printf("%d\n", str2bool("Enable"));
    printf("%d\n", str2bool("YES"));
    printf("%d\n", str2bool("TRUE"));
    printf("%d\n", str2bool("1"));
    printf("%d\n", str2size("1M100K"));
    printf("%d\n", str2time("1w2d3h4m5s"));

    // const char *abc = "abc";
    char abcd[] = "abcd";
    char *p = abcd;
    *p = 'e';
    // abc = "def";

    // printf("%s\n", str_upper("sdfSfdSIDFh"));
    // printf("%s\n", str_lower("dfWOJFDSS*FQ"));
    // printf("%s\n", str_reverse("sdfSfdSIDFh"));
}
