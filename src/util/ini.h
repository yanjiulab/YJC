/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `ini.c` for details.
 */

#ifndef INI_H_
#define INI_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INI_VERSION "0.1.1"

typedef struct ini_t ini_t;

ini_t *ini_load(char *filename);
void ini_free(ini_t *ini);
const char *ini_get(ini_t *ini, const char *section, const char *key);
int ini_sget(ini_t *ini, const char *section, const char *key, const char *scanfmt, void *dst);
int ini_set(ini_t *ini, const char *section, const char *key, char *val);
#define ini_sset(ini, section, key, scanfmt, src) \
    do {                                          \
        char new_val[128] = {0};                  \
        sprintf(new_val, scanfmt, src);           \
        ini_set(ini, section, key, new_val);      \
    } while (0);

#endif
