#include "datetime.h"
#include "test.h"

// time_t int

/* A time value that is accurate to the nearest
   microsecond but also has a range of years.  */
// struct timeval
// {
//   __time_t tv_sec;		/* Seconds.  */
//   __suseconds_t tv_usec;	/* Microseconds.  */
// };

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
// struct timespec
// {
//   __time_t tv_sec;		/* Seconds.  */
//   __syscall_slong_t tv_nsec;	/* Nanoseconds.  */
// };

/* ISO C `broken-down time' structure.  */
// struct tm
// {
//   int tm_sec;			/* Seconds.	[0-60] (1 leap second) */
//   int tm_min;			/* Minutes.	[0-59] */
//   int tm_hour;			/* Hours.	[0-23] */
//   int tm_mday;			/* Day.		[1-31] */
//   int tm_mon;			/* Month.	[0-11] */
//   int tm_year;			/* Year	- 1900.  */
//   int tm_wday;			/* Day of week.	[0-6] */
//   int tm_yday;			/* Days in year.[0-365]	*/
//   int tm_isdst;			/* DST.		[-1/0/1]*/
// # ifdef	__USE_MISC
//   long int tm_gmtoff;		/* Seconds east of UTC.  */
//   const char *tm_zone;		/* Timezone abbreviation.  */
// # else
//   long int __tm_gmtoff;		/* Seconds east of UTC.  */
//   const char *__tm_zone;	/* Timezone abbreviation.  */
// # endif
// };

void test_datetime() {
    datetime_t dt;
    char time_str[DATETIME_FMT_BUFLEN] = {0};

    // 自 1970 年开始时间
    printf("%ld s since 1970.1.1 00:00 UTC+0\n", time(NULL));
    printf("%lld ms since 1970.1.1 00:00 UTC+0\n", gettimeofday_ms());
    printf("%lld us since 1970.1.1 00:00 UTC+0\n", gettimeofday_us());

    // 时钟时间，系统启动时清零
    printf("%d ms since system boot\n", gettick_ms());
    printf("%lld us since system boot\n", gethrtime_us());
    printf("%s elapsed since system boot\n", duration_fmt((int)gettick_ms() / 1000, time_str));

    // 本地时间
    dt = datetime_now();
    printf("local time: %s\n", datetime_fmt(&dt, time_str));
    printf("local time (ISO fmt): %s\n", datetime_fmt_iso(&dt, time_str));
    printf("UTC/GMT time: %s\n", gmtime_fmt(datetime_mktime(&dt), time_str));

    dt = compile_datetime();
    printf("compile time: %s\n", datetime_fmt(&dt, time_str));

    timezone_t tz = timezone_now();
    char tz_str[TZ_FMT_BUFLEN] = {0};
    timezone_fmt(&tz, tz_str);
    printf("gmoff: %ld\n", tz.gmtoff);
    printf("%s %s\n", time_str, tz_str);
}