#ifndef STR_H
#define STR_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_EQUAL(s1, s2) (strncmp((s1), (s2), strlen((s2))) == 0)

char **str_split(const char *in, size_t in_len, char delm, size_t *num_elm, size_t max);
void str_split_free(char **in, size_t num_elm);

char *str_rtrim(char *str, char junk);
char *str_ltrim(char *str, char junk);
char *str_trim(char *str, char junk);

// str append
#endif