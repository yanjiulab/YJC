#include "str.h"

// Prints a message to stderr and exits with a non-zero error code.
static void err(const char* msg) {
    fprintf(stderr, "Error: %s.\n", msg);
    exit(1);
}

char* str(const char* fmtstr, ...) {
    va_list args;
    va_start(args, fmtstr);

    int len = vsnprintf(NULL, 0, fmtstr, args);
    if (len < 0) {
        return NULL;
    }

    va_end(args);
    char* string = malloc(len + 1);

    if (string == NULL) {
        return NULL;
    }
    va_start(args, fmtstr);

    vsnprintf(string, len + 1, fmtstr, args);

    va_end(args);
    return string;
}

int str2int(const char* string) {
    char* endptr;
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

double str2double(const char* string) {
    char* endptr;
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

bool str2bool(const char* str) {
    if (str == NULL)
        return false;
    int len = strlen(str);
    if (len == 0)
        return false;
    switch (len) {
    case 1:
        return *str == '1' || *str == 'y' || *str == 'Y';
    case 2:
        return strcasecmp(str, "on") == 0;
    case 3:
        return strcasecmp(str, "yes") == 0;
    case 4:
        return strcasecmp(str, "true") == 0;
    case 6:
        return strcasecmp(str, "enable") == 0;
    default:
        return false;
    }
}

size_t str2size(const char* str) {
    size_t size = 0, n = 0;
    const char* p = str;
    char c;
    while ((c = *p) != '\0') {
        if (c >= '0' && c <= '9') {
            n = n * 10 + c - '0';
        } else {
            switch (c) {
            case 'K':
            case 'k':
                n <<= 10;
                break;
            case 'M':
            case 'm':
                n <<= 20;
                break;
            case 'G':
            case 'g':
                n <<= 30;
                break;
            case 'T':
            case 't':
                n <<= 40;
                break;
            default:
                break;
            }
            size += n;
            n = 0;
        }
        ++p;
    }
    return size + n;
}

time_t str2time(const char* str) {
    time_t time = 0, n = 0;
    const char* p = str;
    char c;
    while ((c = *p) != '\0') {
        if (c >= '0' && c <= '9') {
            n = n * 10 + c - '0';
        } else {
            switch (c) {
            case 's':
                break;
            case 'm':
                n *= 60;
                break;
            case 'h':
                n *= 60 * 60;
                break;
            case 'd':
                n *= 24 * 60 * 60;
                break;
            case 'w':
                n *= 7 * 24 * 60 * 60;
                break;
            default:
                break;
            }
            time += n;
            n = 0;
        }
        ++p;
    }
    return time + n;
}

uint32_t str_hash(const char* str) {
    uint32_t hash = 2166136261u;
    size_t length = strlen(str);
    for (size_t i = 0; i < length; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619;
    }
    return hash;
}

bool str_startswith(const char* str, const char* prefix) {
    if (!str || !prefix)
        return false;

    size_t lenstr = strlen(str);
    size_t lenprefix = strlen(prefix);

    if (lenprefix > lenstr)
        return false;

    return strncmp(str, prefix, lenprefix) == 0;
}

bool str_endswith(const char* str, const char* suffix) {
    if (!str || !suffix)
        return false;

    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr)
        return false;

    return strncmp(&str[lenstr - lensuffix], suffix, lensuffix) == 0;
}

bool str_contains(const char* str, const char* sub) {
    assert(str != NULL && sub != NULL);
    return strstr(str, sub) != NULL;
}

bool str_all_digit(const char* str) {
    for (; *str != '\0'; str++)
        if (!isdigit((unsigned char)*str))
            return false;
    return true;
}

/*-------------------------- new string operations ---------------------------------*/
char* str_replace(const char* str, const char* find, const char* replace) {
    char* ch;
    char* nustr = strdup(str);

    size_t findlen = strlen(find);
    size_t repllen = strlen(replace);

    while ((ch = strstr(nustr, find))) {
        if (repllen > findlen) {
            size_t nusz = strlen(nustr) + repllen - findlen + 1;
            nustr = realloc(nustr, nusz);
            ch = strstr(nustr, find);
        }

        size_t nustrlen = strlen(nustr);
        size_t taillen = (nustr + nustrlen) - (ch + findlen);

        memmove(ch + findlen + (repllen - findlen), ch + findlen, taillen + 1);
        memcpy(ch, replace, repllen);
    }

    return nustr;
}

/*-------------------------- in place modified operations ---------------------------------*/
char* str_upper(char* buf) {
    char* p = buf;
    while (*p != '\0') {
        if (*p >= 'a' && *p <= 'z') {
            *p &= ~0x20;
        }
        ++p;
    }
    return buf;
}

char* str_lower(char* buf) {
    char* p = buf;
    while (*p != '\0') {
        if (*p >= 'A' && *p <= 'Z') {
            *p |= 0x20;
        }
        ++p;
    }
    return buf;
}

char* str_reverse(char* buf) {
    if (buf == NULL)
        return NULL;
    char* b = buf;
    char* e = buf;
    while (*e) {
        ++e;
    }
    --e;
    char tmp;
    while (e > b) {
        tmp = *e;
        *e = *b;
        *b = tmp;
        --e;
        ++b;
    }
    return buf;
}

char* str_rtrim(char* buf, char junk) {
    char* original = buf + strlen(buf);
    while (*--original == junk)
        ;
    *(original + 1) = '\0';
    return buf;
}

char* str_ltrim(char* buf, char junk) {
    char* original = buf;
    char* p = original;
    int trimmed = 0;
    do {
        if (*original != junk || trimmed) {
            trimmed = 1;
            *p++ = *original;
        }
    } while (*original++ != '\0');
    return buf;
}

char* str_trim(char* buf, char junk) {
    str_ltrim(str_rtrim(buf, junk), junk);
    return buf;
}

int str_split(char* in, char** out, int outlen, const char* sep) {
    if (in == NULL || strlen(in) == 0)
        return 0;
    if (sep == NULL || strlen(sep) == 0) {
        *out = in;
        return 1;
    }

    int n;
    char *str, *token, *saveptr;
    for (n = 0, str = in;; n++, str = NULL) {
        if (n == outlen)
            break;
        token = strtok_r(str, sep, &saveptr);
        if (token == NULL)
            break;
        out[n] = token;
    }

    return n;
}

char* str_hex(char* buff, size_t bufsiz, const uint8_t* str, size_t num) {
    if (bufsiz == 0)
        return buff;

    char tmp[3];

    buff[0] = '\0';

    for (size_t i = 0; i < num; i++) {
        snprintf(tmp, sizeof(tmp), "%02x", (unsigned char)str[i]);
        strncat(buff, tmp, bufsiz);
    }

    return buff;
}


