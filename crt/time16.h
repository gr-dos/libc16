/* Private to libc16: calendar arithmetic and the time zone, shared by the
 * time and file members (docs/spec/time.md, docs/spec/files.md). */

#ifndef LIBC16_TIME16_H
#define LIBC16_TIME16_H

#include <time.h>

/* The zone names tzname points to, and whether tzset has run */
extern char __libc16_tzstd[16];
extern char __libc16_tzdst[16];
extern char __libc16_tzdone;

/* Days in year y (which it evaluates more than once), and days from 1970-01-01 to January 1 of year y */
#define YEAR_DAYS(y) (((y) % 4 == 0 && ((y) % 100 != 0 || (y) % 400 == 0)) ? 366 : 365)
long __libc16_year_start(int y);

/* Day of the year (0 is January 1) of month mon (0 to 11), day mday */
int __libc16_yday(int y, int mon, int mday);

/* Whether daylight saving time applies at the local standard time tm,
 * when tzset found a daylight zone */
int __libc16_isdst(const struct tm *tm);

/* Fills tm, all but tm_isdst, with the time adjust seconds (a few days at
 * most, either way) after t seconds since 1970, without overflow */
void __libc16_civil(long t, long adjust, struct tm *tm);

/* The time_t of a local wall-clock time, given as seconds since 1970 as
 * if it were UTC */
time_t __libc16_from_local(long local);

/* A DOS file date and time (local time) as a time_t, and a time_t as a
 * DOS date and time; __libc16_to_dos returns -1 if the year is outside
 * 1980 to 2107. */
time_t __libc16_from_dos(unsigned date, unsigned time);
int __libc16_to_dos(time_t t, unsigned *date, unsigned *time);

#endif
