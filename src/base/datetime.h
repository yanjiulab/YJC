#ifndef DATETIME_H
#define DATETIME_H

/**
1. struct timeval:  A time value that is accurate to the nearest microsecond
                    but also has a range of years.
    struct timeval {
        __time_t tv_sec;       // Seconds.
        __suseconds_t tv_usec; // Microseconds.
    };

    get current time: gettimeofday(&tv, NULL);

2. struct timezone
3. struct tm
4. time_t:
5. clock_t:
*/

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR   3600
#define SECONDS_PER_DAY    86400  // 24*3600
#define SECONDS_PER_WEEK   604800 // 7*24*3600

#define IS_LEAP_YEAR(year) (((year) % 4 == 0 && (year) % 100 != 0) || (year) % 400 == 0)

typedef struct datetime_s {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
    int ms;
} datetime_t;

typedef struct timezone_s {
    long gmtoff;
    uint8_t hour;
    uint8_t min;
    const char* timezone;
} timezone_t;

extern unsigned int gettick_ms();
static inline unsigned long long gettimeofday_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000 + tv.tv_usec / 1000;
}
static inline unsigned long long gettimeofday_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (unsigned long long)1000000 + tv.tv_usec;
}
extern unsigned long long gethrtime_us();

extern datetime_t datetime_now();
extern datetime_t datetime_localtime(time_t seconds);

extern time_t datetime_mktime(datetime_t* dt);

extern datetime_t* datetime_past(datetime_t* dt, int days);
extern datetime_t* datetime_future(datetime_t* dt, int days);

#define TIME_FMT        "%02d:%02d:%02d"
#define TIME_FMT_BUFLEN 12
extern char* duration_fmt(int sec, char* buf);

#define DATETIME_FMT        "%04d-%02d-%02d %02d:%02d:%02d"
#define DATETIME_FMT_ISO    "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"
#define DATETIME_FMT_BUFLEN 30
extern char* datetime_fmt(datetime_t* dt, char* buf);
extern char* datetime_fmt_iso(datetime_t* dt, char* buf);

#define GMTIME_FMT        "%.3s, %02d %.3s %04d %02d:%02d:%02d GMT"
#define GMTIME_FMT_BUFLEN 30
extern char* gmtime_fmt(time_t time, char* buf);

extern int days_of_month(int month, int year);

extern int month_atoi(const char* month);
extern const char* month_itoa(int month);

extern int weekday_atoi(const char* weekday);
extern const char* weekday_itoa(int weekday);

extern datetime_t compile_datetime();

/*
 * minute   hour    day     week    month       action
 * 0~59     0~23    1~31    0~6     1~12
 *  -1      -1      -1      -1      -1          cron.minutely
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       0      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
extern time_t cron_next_timeout(int minute, int hour, int day, int week, int month);

extern timezone_t timezone_now();
#define TIMEZONE_FMT  "%s(UTC%s%02d:%02d)"
#define TZ_FMT_BUFLEN 20
extern char* timezone_fmt(timezone_t* tz, char* buf);

void tv_sub(struct timeval* out, struct timeval* in);
#endif