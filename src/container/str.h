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
#include <sys/time.h>

#define strmatch(s1, s2) (strncmp((s1), (s2), strlen((s2))) == 0)
#define STR_SPLIT(in, out, sep) str_split(in, out, sizeof(out) / sizeof(out[0]), sep)

// Prints to an automatically-allocated string. Returns NULL if an encoding error occurs or if sufficient memory cannot
// be allocated.
char *str(const char *fmtstr, ...);
// Attempts to parse a string as an integer value, exiting on failure.
int str2int(const char *string);
// Attempts to parse a string as a double value, exiting on failure.
double str2double(const char *string);
// 1 y on yes true enable => true
bool str2bool(const char *str);
// 1T2G3M4K5B => ?B
size_t str2size(const char *str);
// 1w2d3h4m5s => ?s
time_t str2time(const char *str);
// Hashes a string using the FNV-1a algorithm.
uint32_t str_hash(const char *string);

bool str_startswith(const char *str, const char *prefix);
bool str_endswith(const char *str, const char *suffix);
bool str_all_digit(const char *str);
bool str_contains(const char *str, const char *sub);

/*-------------------------- new string operations ---------------------------------*/
// Use REPLACE string to replace the FIND string in STR.
char *str_replace(const char *str, const char *find, const char *replace);

/*-------------------------- in place modified operations ---------------------------------*/
// Converts a string to uppercase in place.
char *str_upper(char *str);
// Converts a string to lowercase in place.
char *str_lower(char *str);
// Reverses the string in place.
char *str_reverse(char *str);
// Use SEP to split the IN string and store the pointers in OUT array, return the split number.
int str_split(char *in, char **out, int outlen, const char *sep);
char *str_rtrim(char *buf, char junk);
char *str_ltrim(char *buf, char junk);
char *str_trim(char *buf, char junk);
char *str_hex(char *buff, size_t bufsiz, const uint8_t *str, size_t num);
// Generates a random string of len in heap if buf is NULL, otherwise in buf.
char *str_random(char *buf, int len);

#endif