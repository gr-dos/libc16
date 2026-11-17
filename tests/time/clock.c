/* time (docs/spec/time.md), from DOS dates and times set with INT 21h 2Bh
 * and 2Dh. "clock utc" runs with TZ unset and "clock est" with EST5EDT;
 * the expected values were computed on the host. Also checks that gmtime
 * and localtime keep separate results. The DOS clock is set back at the
 * end. Each time is set at .50 seconds: DOSBox-X reads a time set at .00
 * back a few hundredths early, in the second before. */

#include <string.h>
#include <time.h>
#include "check.h"

static unsigned char set_date(unsigned year, unsigned mon_day);
#pragma aux set_date = \
    "mov ah, 2bh" \
    "int 21h" \
    parm [cx] [dx] value [al] modify exact [ax];

static unsigned char set_time(unsigned hour_min, unsigned sec_hund);
#pragma aux set_time = \
    "mov ah, 2dh" \
    "int 21h" \
    parm [cx] [dx] value [al] modify exact [ax];

/* DX = year, AX = month << 8 | day */
static unsigned long get_date(void);
#pragma aux get_date = \
    "mov ah, 2ah" \
    "int 21h" \
    "mov ax, dx" \
    "mov dx, cx" \
    value [dx ax] modify exact [ax cx dx];

/* DX = hour << 8 | minute, AX = second << 8 | hundredths */
static unsigned long get_time(void);
#pragma aux get_time = \
    "mov ah, 2ch" \
    "int 21h" \
    "mov ax, dx" \
    "mov dx, cx" \
    value [dx ax] modify exact [ax cx dx];

struct ccase {
    int year, mon, mday, hour, min, sec;
    time_t want;
    int exact;          /* 0 if a second passing gives want + 1 */
};

static const struct ccase utc[] = {
    { 1980, 1, 1, 0, 0, 0, 315532800L, 0 },
    { 2024, 3, 10, 1, 59, 58, 1710035998L, 0 },
    { 2024, 3, 10, 3, 0, 0, 1710039600L, 0 },
    { 2024, 11, 3, 0, 59, 59, 1730595599L, 0 },
    { 2024, 11, 3, 1, 30, 0, 1730597400L, 0 },
    { 2024, 11, 3, 2, 0, 0, 1730599200L, 0 },
    { 2024, 2, 29, 23, 59, 59, 1709251199L, 0 },
    { 2000, 1, 1, 0, 0, 0, 946684800L, 0 },
    { 2024, 7, 4, 12, 0, 0, 1720094400L, 0 },
    { 2038, 1, 19, 3, 14, 7, 2147483647L, 1 },
    { 2038, 1, 19, 3, 14, 8, -1L, 1 },
    { 2099, 12, 31, 23, 59, 58, -1L, 1 },
};

static const struct ccase est[] = {
    { 1980, 1, 1, 0, 0, 0, 315550800L, 0 },
    { 2024, 3, 10, 1, 59, 58, 1710053998L, 0 },     /* EST */
    { 2024, 3, 10, 3, 0, 0, 1710054000L, 0 },       /* EDT */
    { 2024, 11, 3, 0, 59, 59, 1730609999L, 0 },     /* EDT */
    { 2024, 11, 3, 1, 30, 0, 1730611800L, 0 },      /* repeated: EDT */
    { 2024, 11, 3, 2, 0, 0, 1730617200L, 0 },       /* EST */
    { 2024, 2, 29, 23, 59, 59, 1709269199L, 0 },
    { 2000, 1, 1, 0, 0, 0, 946702800L, 0 },
    { 2024, 7, 4, 12, 0, 0, 1720108800L, 0 },
    { 2038, 1, 18, 22, 14, 7, 2147483647L, 1 },
    { 2038, 1, 18, 22, 14, 8, -1L, 1 },
    { 2099, 12, 31, 23, 59, 58, -1L, 1 },
};

#define NCASES (sizeof utc / sizeof utc[0])

static void set_clock(int year, int mon, int mday, int hour, int min, int sec, int hund)
{
    CHECK(set_date(year, mon << 8 | mday) == 0);
    CHECK(set_time(hour << 8 | min, sec << 8 | hund) == 0);
}

static void fail_case(const char *what, unsigned i)
{
    t_puts("not ok ");
    t_puts(what);
    t_puts(" ");
    t_putu(i);
    t_puts("\r\n");
    failures++;
}

int main(int argc, char **argv)
{
    unsigned long d0 = get_date(), t0 = get_time();
    const struct ccase *cases;
    struct tm *g, *l, lsave;
    time_t r, x, prev, want;
    int is_est;
    unsigned i, n;

    if (argc != 2) {
        t_puts("usage: clock utc|est\r\n");
        return 2;
    }
    is_est = strcmp(argv[1], "est") == 0;
    cases = is_est ? est : utc;

    /* The first call of time calls tzset. */
    set_clock(2024, 7, 4, 12, 0, 0, 50);
    r = time(NULL);
    CHECK(r == cases[8].want || r == cases[8].want + 1);
    CHECK(daylight == is_est && timezone == (is_est ? 18000L : 0L));

    for (i = 0; i < NCASES; i++) {
        const struct ccase *c = &cases[i];

        set_clock(c->year, c->mon, c->mday, c->hour, c->min, c->sec, 50);
        x = 12345;
        r = time(&x);
        if (x != r)
            fail_case("stored", i);
        if (r != c->want && (c->exact || r != c->want + 1))
            fail_case("time", i);
        /* localtime gives the DOS date and time back */
        if (r == c->want && r != -1) {
            l = localtime(&r);
            if (l->tm_year + 1900 != c->year || l->tm_mon + 1 != c->mon || l->tm_mday != c->mday
                || l->tm_hour != c->hour || l->tm_min != c->min || l->tm_sec != c->sec)
                fail_case("localtime", i);
        }
    }
    CHECK(time(NULL) == -1);

    /* Across midnight: never a day off, and never backwards */
    set_clock(2024, 12, 31, 23, 59, 59, 90);
    want = is_est ? 1735707599L : 1735689599L;  /* 2024-12-31 23:59:59 local */
    prev = want;
    for (n = 0; n < 2000; n++) {
        r = time(NULL);
        if (r < prev || r > want + 2) {
            fail_case("midnight", n);
            break;
        }
        prev = r;
        if (r >= want + 2)
            break;
    }
    CHECK(prev > want);

    /* gmtime and localtime have separate results. In July 2024 EST5EDT is
     * in daylight time. */
    r = 1720094400L;                            /* 2024-07-04 12:00:00 UTC */
    g = gmtime(&r);
    l = localtime(&r);
    CHECK(g != l);
    CHECK(g->tm_hour == 12 && g->tm_isdst == 0);
    CHECK(l->tm_hour == (is_est ? 8 : 12) && l->tm_isdst == is_est);
    lsave = *l;
    r = 0;
    g = gmtime(&r);
    CHECK(memcmp(l, &lsave, sizeof lsave) == 0);
    r = 86400L * 365;
    l = localtime(&r);
    CHECK(g->tm_year == 70 && g->tm_yday == 0 && g->tm_hour == 0);

    /* the clock back where it was */
    CHECK(set_date((unsigned)(d0 >> 16), (unsigned)d0) == 0);
    CHECK(set_time((unsigned)(t0 >> 16), (unsigned)t0) == 0);
    return done();
}
