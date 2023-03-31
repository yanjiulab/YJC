#ifndef STR_H_
#define STR_H_

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <

#define STR_EQUAL(s1, s2) (strncmp((s1), (s2), strlen((s2))) == 0)
#define STR_SPLIT(in, out, sep) str_split(in, out, sizeof(out) / sizeof(out[0]), sep)

char *str(const char *fmtstr, ...);
int str2int(const char *string);
double str2double(const char *string);
uint32_t str_hash(const char *string);

int str_split(char *in, char **out, int outlen, const char *sep);
char *str_rtrim(char *str, char junk);
char *str_ltrim(char *str, char junk);
char *str_trim(char *str, char junk);
bool str_startswith(const char *str, const char *prefix);
bool str_endswith(const char *str, const char *suffix);
int all_digit(const char *str);
char *str_hex(char *buff, size_t bufsiz, const uint8_t *str, size_t num);

#endif