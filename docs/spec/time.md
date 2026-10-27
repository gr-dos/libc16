# Spec: time

**Header:** `<time.h>`  
**Members:** `tzset` (with `timezone`, `daylight` and `tzname`), `localtime`, `strftime`, and the private conversions they and `docs/spec/files.md` share  
**Tests:** `tests/time` (run by `tests/run-dos`, in every model)

This is the behavioural spec an implementation is written from (decision 6). It does not describe any existing implementation. `strftime` is formatting code, so a separate session writes it (decision 6); the rest is ordinary library code.

## Sources

- C89 7.12 (date and time): `time_t`, `struct tm`, `localtime` (7.12.3.4), `strftime` (7.12.3.5), and the "C" locale's names of days and months.
- Microsoft C 5.1 run-time library reference: `tzset`, `timezone`, `daylight`, `tzname` and the `TZ` variable.
- The Gregorian calendar; DOS stores file times as local time (Ralf Brown's Interrupt List, INT 21h function 57h).

## Types

- `time_t` is `long`: seconds since 1970-01-01 00:00:00 UTC, ignoring leap seconds. Negative values are before 1970.
- `struct tm` has C89's members, all `int`, in the order `<time.h>` declares them.

## `void tzset(void)`

Reads `TZ` from the environment (`getenv`, `docs/spec/dos.md`) and sets `timezone`, `daylight` and `tzname`. `TZ` has Microsoft's form `tzn[+|-]hh[:mm[:ss]][dzn]`:

- `tzn`, the standard zone's name, is three or more letters; `tzname[0]` points to a copy of it.
- The offset is the time to add to local standard time to get UTC, so `EST5` is five hours west: `timezone` = 18000. The hours are one or two digits, 0 to 24; minutes and seconds, if present, are two digits each, 0 to 59. A `-` makes it negative (east of UTC); `+` or no sign makes it positive.
- `dzn`, if present, is three or more letters naming the daylight saving zone: `daylight` is then 1 and `tzname[1]` points to a copy of it. Otherwise `daylight` is 0 and `tzname[1]` is `""`.
- Names longer than 15 letters are cut to 15.

If `TZ` is not set, or does not have this form, the zone is UTC with no daylight saving: `timezone` = 0, `daylight` = 0, `tzname[0]` = `"UTC"` and `tzname[1]` = `""`. (Microsoft C uses Pacific time when `TZ` is not set; libc16 does not.) Before the first call, and in a program that never calls `tzset`, `localtime`, `stat` or `utime`, the variables are the same as for an unset `TZ`.

**Daylight saving time** applies when `daylight` is 1, from 2:00 local standard time on the second Sunday in March to 2:00 local daylight time on the first Sunday in November (the United States rules since 2007, in every year), and is one hour ahead of standard time. Microsoft C 5.1 used the earlier United States rules; libc16 uses the current ones.

## `struct tm *localtime(const time_t *t)`

Converts `*t` to local time: `*t` minus `timezone`, plus an hour when daylight saving time applies at that instant. Fills every member of a static `struct tm` (`tm_isdst` 1 or 0), which the next call overwrites, and returns a pointer to it. If `tzset` has not been called, `localtime` calls it first. Every `time_t` from 1970-01-01 (and earlier back to 1901-12-13, the most negative `long`) up to 2038-01-19 has a result; the result is not affected by the range of DOS dates.

## `size_t strftime(char *s, size_t max, const char *format, const struct tm *tm)`

C89 7.12.3.5 in the "C" locale. Copies `format` to `s`, replacing each conversion with the text below and copying other characters unchanged, and null-terminates it. Returns the number of characters stored, not counting the terminator, if they and the terminator fit in `max` bytes; otherwise returns 0 and the contents of `s` are indeterminate.

| Conversion | Replaced by |
|---|---|
| `%a`, `%A` | abbreviated and full weekday name: `Sun` … `Sat`, `Sunday` … `Saturday` |
| `%b`, `%B` | abbreviated and full month name: `Jan` … `Dec`, `January` … `December` |
| `%c` | date and time, as `%a %b %d %H:%M:%S %Y` |
| `%d` | day of the month, `01` to `31` |
| `%H`, `%I` | hour, `00` to `23`, and `01` to `12` |
| `%j` | day of the year, `001` to `366` |
| `%m` | month, `01` to `12` |
| `%M`, `%S` | minute `00` to `59`, second `00` to `60` |
| `%p` | `AM` or `PM` |
| `%U`, `%W` | week of the year, `00` to `53`, with the first Sunday (`%U`) or Monday (`%W`) as the first day of week 1 |
| `%w` | weekday, `0` (Sunday) to `6` |
| `%x`, `%X` | date as `%m/%d/%y`, time as `%H:%M:%S` |
| `%y`, `%Y` | year without century, `00` to `99`, and with century |
| `%Z` | `tzname[0]`, or `tzname[1]` when `tm_isdst` is positive; nothing if that name is empty |
| `%%` | `%` |

The members used are those C89 lists for each conversion; values outside their normal ranges give unspecified text but never a write past `max` bytes. Any other character after `%` is undefined in C89; libc16 copies the `%` and the character unchanged. A `%` at the end of `format` is copied.

`strftime` does not call `tzset`, and does not link it: `%Z`'s reference to `tzname` links only the member that holds the time zone variables.

## Constraints

- All are `__cdecl` (`docs/ABI.md`). A program links only the members it calls: `strftime` does not link `localtime`, and neither links the stdio functions.
- 8086 instructions only (`scripts/check-8086`).

## Tests

`tests/time` covers: `tzset` with `TZ` unset, `UTC0`, `EST5EDT`, `CET-1`, `NST3:30` and invalid values; `localtime` at 1970, the year 2000, a leap day, the end of a leap year, 2038-01-19 03:14:07 UTC, a negative `time_t`, and either side of both daylight saving transitions; `strftime` with every conversion on several dates, the exact `max` boundary (fits, and one byte short), an empty format, and `%Z` with and without a daylight zone.
