/* strftime (docs/spec/time.md): every conversion on several dates, the
 * max boundary, an empty format, %Z with and without a daylight zone, and
 * the cases libc16 defines beyond C89. Prints "LABEL FORMAT -> N [S]" per
 * call (just "-> 0" when nothing fits), for strftime.expect, which
 * tests/time/mkexpect writes; and "wrote past max" if a call wrote a byte
 * at or after s[max]. tzname is set directly, so tzset is not linked. */

#include <time.h>
#include "testio.h"

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

/* tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday,
 * tm_isdst; tests/time/mkexpect has the same dates. */
static const struct tm dates[] = {
    {  0,  0,  0,  1,  0,  70, 4,   0, 0 },   /* 1970-01-01 Thursday */
    { 56, 34, 12, 29,  1, 100, 2,  59, 0 },   /* 2000-02-29, noon */
    { 59, 59, 23, 31, 11, 124, 2, 365, 0 },   /* 2024-12-31, a leap year's end */
    {  9,  5,  0,  4,  0, 126, 0,   3, 0 },   /* 2026-01-04, the first Sunday */
    {  8,  7, 13,  1,  0, 118, 1,   0, 0 },   /* 2018-01-01, a Monday */
    { 11, 10,  9, 31, 11,  99, 5, 364, 0 },   /* 1999-12-31 */
    {  0,  0, 18,  4,  6, 126, 6, 184, 1 },   /* 2026-07-04, daylight time */
    { 52, 45, 20, 13, 11,   1, 5, 346, 0 },   /* 1901-12-13 */
    {  1,  2,  3,  1,  0,   0, 1,   0, 0 }    /* 1900-01-01 */
};

static const char *const formats[] = {
    "%a", "%A", "%b", "%B", "%c", "%d", "%H", "%I", "%j", "%m", "%M",
    "%p", "%S", "%U", "%W", "%w", "%x", "%X", "%y", "%Y", "%%",
    "Week %U/%W, day %j: %A %d %B %Y %I:%M %p"
};

/* With far data, buf is outside DGROUP (docs/ABI.md, "Memory models"). */
#if defined(__COMPACT__) || defined(__LARGE__)
static char __far buf[80];
#else
static char buf[80];
#endif
static int failed;

static unsigned run(const char *format, const struct tm *tm, unsigned max)
{
    unsigned n;
    unsigned i;

    for (i = 0; i < sizeof buf; i++)
        buf[i] = '#';
    n = strftime(buf, max, format, tm);
    for (i = max; i < sizeof buf; i++)
        if (buf[i] != '#') {
            t_puts("wrote past max\r\n");
            failed = 1;
            break;
        }
    return n;
}

static void show(const char *label, const char *format, const struct tm *tm,
                 unsigned max)
{
    unsigned n = run(format, tm, max);

    t_puts(label);
    t_puts(" ");
    t_puts(format);
    t_puts(" -> ");
    t_putu(n);
    if (n) {
        t_puts(" [");
        t_puts(buf);
        t_puts("]");
    }
    t_puts("\r\n");
}

/* Every conversion on members out of their ranges: the text is
 * unspecified, but must be what was counted, within max. */
static void out_of_range(void)
{
    static const char all[] = "%a%A%b%B%c%d%H%I%j%m%M%p%S%U%W%w%x%X%y%Y%Z";
    static const struct tm odd[] = {
        { 99, -1, 24, 99, 12, -2000, 7, -5, 2 },
        { -1, 60, -1,  0, -1,  9999, -1, 999, -1 },
        { 32767, -32767 - 1, 32767, -32767 - 1, 32767, 32767, 32767, 32767, 32767 }
    };
    unsigned i, max, n, len;

    for (i = 0; i < COUNT(odd); i++)
        for (max = 0; max <= sizeof buf; max++) {
            n = run(all, &odd[i], max);
            for (len = 0; len < max && buf[len]; len++)
                ;
            if (n && (n >= max || len != n)) {
                t_puts("bad count\r\n");
                failed = 1;
            }
        }
}

int main(void)
{
    static char label[] = "0";
    struct tm tm;
    unsigned i, j;

    for (i = 0; i < COUNT(dates); i++) {
        label[0] = (char)('0' + i);
        for (j = 0; j < COUNT(formats); j++)
            show(label, formats[j], &dates[i], sizeof buf);
    }

    /* The boundary: the text and its terminator fit in max, or 0. */
    show("max11", "%Y-%m-%d", &dates[0], 11);
    show("max10", "%Y-%m-%d", &dates[0], 10);
    show("max25", "%c", &dates[0], 25);
    show("max24", "%c", &dates[0], 24);
    show("max9", "%B", &dates[2], 9);
    show("max8", "%B", &dates[2], 8);
    show("max2", "x", &dates[0], 2);
    show("max1", "x", &dates[0], 1);
    show("max0", "x", &dates[0], 0);

    /* An empty format stores just the terminator, when max has room. */
    show("empty1", "", &dates[0], 1);
    t_puts(buf[0] == '\0' ? "empty1 terminated\r\n" : "empty1 not terminated\r\n");
    show("empty0", "", &dates[0], 0);

    /* %Z: first the variables a program has before any tzset (UTC, no
     * daylight zone), then a zone with one. */
    tm = dates[0];
    show("utc", "[%Z]", &tm, sizeof buf);
    tm.tm_isdst = 1;
    show("utcdst", "[%Z]", &tm, sizeof buf);
    tzname[0] = "EST";
    tzname[1] = "EDT";
    tm.tm_isdst = 0;
    show("est", "[%Z]", &tm, sizeof buf);
    tm.tm_isdst = 1;
    show("edt", "[%Z]", &tm, sizeof buf);
    tm.tm_isdst = 2;
    show("edt2", "[%Z]", &tm, sizeof buf);
    tm.tm_isdst = -1;
    show("unknown", "[%Z]", &tm, sizeof buf);

    /* libc16's choices where C89 leaves it undefined, and a leap second. */
    show("other", "%q%E%5", &dates[0], sizeof buf);
    show("end", "100%", &dates[0], sizeof buf);
    show("percent", "100%%", &dates[0], sizeof buf);
    tm = dates[2];
    tm.tm_sec = 60;
    show("leap", "%S", &tm, sizeof buf);

    out_of_range();
    return failed;
}
