#include "str.h"

// Prints a message to stderr and exits with a non-zero error code.
static void err(const char *msg) {
    fprintf(stderr, "Error: %s.\n", msg);
    exit(1);
}

// Prints to an automatically-allocated string. Returns NULL if an encoding
// error occurs or if sufficient memory cannot be allocated.
char *str(const char *fmtstr, ...) {
    va_list args;

    va_start(args, fmtstr);
    int len = vsnprintf(NULL, 0, fmtstr, args);
    if (len < 0) {
        return NULL;
    }
    va_end(args);

    char *string = malloc(len + 1);
    if (string == NULL) {
        return NULL;
    }

    va_start(args, fmtstr);
    vsnprintf(string, len + 1, fmtstr, args);
    va_end(args);

    return string;
}

// Attempts to parse a string as an integer value, exiting on failure.
int str2int(const char *string) {
    char *endptr;
    errno = 0;
    long result = strtol(string, &endptr, 0);
    if (errno == ERANGE || result > INT_MAX || result < INT_MIN) {
        err(str("'%s' is out of range", string));
    }
    if (*endptr != '\0') {
        err(str("cannot parse '%s' as an integer", string));
    }
    return (int)result;
}

// Attempts to parse a string as a double value, exiting on failure.
double str2double(const char *string) {
    char *endptr;
    errno = 0;
    double result = strtod(string, &endptr);
    if (errno == ERANGE) {
        err(str("'%s' is out of range", string));
    }
    if (*endptr != '\0') {
        err(str("cannot parse '%s' as a floating-point value", string));
    }
    return result;
}

// Hashes a string using the FNV-1a algorithm.
uint32_t str_hash(const char *string) {
    uint32_t hash = 2166136261u;
    size_t length = strlen(string);
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)string[i];
        hash *= 16777619;
    }
    return hash;
}

int str_split(char *in, char **out, int outlen, const char *sep) {
    if (in == NULL || strlen(in) == 0) return 0;
    if (sep == NULL || strlen(sep) == 0) {
        *out = in;
        return 1;
    }

    int n;
    char *str, *token, *saveptr;
    for (n = 0, str = in;; n++, str = NULL) {
        if (n == outlen) break;
        token = strtok_r(str, sep, &saveptr);
        if (token == NULL) break;
        out[n] = token;
    }

    return n;
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