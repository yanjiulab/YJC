#include "str.h"

char **str_split(const char *in, size_t in_len, char delm, size_t *num_elm, size_t max) {
    char *parsestr;
    char **out;
    size_t cnt = 1;
    size_t i;

    if (in == NULL || in_len == 0 || num_elm == NULL) return NULL;

    parsestr = malloc(in_len + 1);
    memcpy(parsestr, in, in_len + 1);
    parsestr[in_len] = '\0';

    *num_elm = 1;
    for (i = 0; i < in_len; i++) {
        if (parsestr[i] == delm) (*num_elm)++;
        if (max > 0 && *num_elm == max) break;
    }

    out = malloc(*num_elm * sizeof(*out));
    out[0] = parsestr;
    for (i = 0; i < in_len && cnt < *num_elm; i++) {
        if (parsestr[i] != delm) continue;

        /* Add the pointer to the array of elements */
        parsestr[i] = '\0';
        out[cnt] = parsestr + i + 1;
        cnt++;
    }

    return out;
}

void str_split_free(char **in, size_t num_elm) {
    if (in == NULL) return;
    if (num_elm != 0) free(in[0]);
    free(in);
}

char *str_rtrim(char *string, char junk) {
    char *original = string + strlen(string);
    while (*--original == junk)
        ;
    *(original + 1) = '\0';
    return string;
}

char *str_ltrim(char *string, char junk) {
    char *original = string;
    char *p = original;
    int trimmed = 0;
    do {
        if (*original != junk || trimmed) {
            trimmed = 1;
            *p++ = *original;
        }
    } while (*original++ != '\0');
    return string;
}

char *str_trim(char *string, char junk) {
    str_ltrim(str_rtrim(string, junk), junk);
    return string;
}