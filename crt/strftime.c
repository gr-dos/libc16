/* strftime - libc16 (docs/spec/time.md; C89 7.12.3.5)
 *
 * The "C" locale only. %Z reads tzname, which links the member holding the
 * time zone variables but not tzset (the spec), and nothing here links
 * localtime or stdio. */

#include <time.h>

/* The output so far: p is the next byte of s, and left the characters
 * that still fit before the terminator. full is set when one did not. */
struct out {
    char *p;
    size_t left;
    char full;
};

/* The "C" locale's names, each abbreviated to its first three letters. */
static const char days[] =
    "Sunday\0Monday\0Tuesday\0Wednesday\0Thursday\0Friday\0Saturday";
static const char months[] =
    "January\0February\0March\0April\0May\0June\0July\0August\0September\0"
    "October\0November\0December";

static void conv(struct out *o, const char *format, const struct tm *tm);

static void put(struct out *o, char c)
{
    if (o->left) {
        *o->p++ = c;
        o->left--;
    } else {
        o->full = 1;
    }
}

static void put_string(struct out *o, const char *s)
{
    while (*s)
        put(o, *s++);
}

/* v in decimal, with leading zeros to at least width digits. */
static void number(struct out *o, unsigned v, unsigned width)
{
    char buf[5];                /* 65535 */
    unsigned n = 0;

    do {
        buf[n++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (width > n) {
        put(o, '0');
        width--;
    }
    while (n)
        put(o, buf[--n]);
}

/* Name i of the count in names, in full or as its first three letters.
 * An i out of range gives "?" rather than read past the table. */
static void name(struct out *o, const char *names, int i, int count, char full)
{
    int n;

    if (i < 0 || i >= count) {
        put(o, '?');
        return;
    }
    while (i--)
        while (*names++)
            ;
    for (n = 0; names[n] && (full || n < 3); n++)
        put(o, names[n]);
}

/* Writes the conversion %c. */
static void one(struct out *o, char c, const struct tm *tm)
{
    unsigned u;
    long year;
    const char *zone;

    switch (c) {
    case 'a':
    case 'A':
        name(o, days, tm->tm_wday, 7, c == 'A');
        break;
    case 'b':
    case 'B':
        name(o, months, tm->tm_mon, 12, c == 'B');
        break;
    case 'c':
        conv(o, "%a %b %d %H:%M:%S %Y", tm);
        break;
    case 'd':
        number(o, tm->tm_mday, 2);
        break;
    case 'H':
        number(o, tm->tm_hour, 2);
        break;
    case 'I':
        u = (unsigned)tm->tm_hour % 12;
        number(o, u ? u : 12, 2);
        break;
    case 'j':
        number(o, (unsigned)tm->tm_yday + 1, 3);
        break;
    case 'm':
        number(o, (unsigned)tm->tm_mon + 1, 2);
        break;
    case 'M':
        number(o, tm->tm_min, 2);
        break;
    case 'p':
        put_string(o, tm->tm_hour < 12 ? "AM" : "PM");
        break;
    case 'S':
        number(o, tm->tm_sec, 2);
        break;
    case 'U':
        /* Days before the first Sunday are week 0. Unsigned arithmetic
         * keeps out-of-range members from overflowing. */
        number(o, ((unsigned)tm->tm_yday + 7 - (unsigned)tm->tm_wday) / 7, 2);
        break;
    case 'W':
        /* As %U, with Monday the first day: its weekday counted from
         * Monday is (tm_wday + 6) % 7. */
        number(o, ((unsigned)tm->tm_yday + 7
                   - ((unsigned)tm->tm_wday + 6) % 7) / 7, 2);
        break;
    case 'w':
        number(o, tm->tm_wday, 1);
        break;
    case 'x':
        conv(o, "%m/%d/%y", tm);
        break;
    case 'X':
        conv(o, "%H:%M:%S", tm);
        break;
    case 'y':
        /* tm_year counts from 1900, a whole century. */
        u = (unsigned)(tm->tm_year % 100 + 100) % 100;
        number(o, u, 2);
        break;
    case 'Y':
        year = tm->tm_year + 1900L;
        if (year < 0) {
            put(o, '-');
            year = -year;
        }
        number(o, (unsigned)year, 1);
        break;
    case 'Z':
        /* Nothing for an empty name (the spec), or a null one. */
        zone = tzname[tm->tm_isdst > 0];
        if (zone)
            put_string(o, zone);
        break;
    case '%':
        put(o, '%');
        break;
    default:
        /* Undefined in C89: libc16 copies the % and the character. */
        put(o, '%');
        put(o, c);
        break;
    }
}

/* Copies format to o, replacing each conversion. */
static void conv(struct out *o, const char *format, const struct tm *tm)
{
    while (*format) {
        if (*format != '%') {
            put(o, *format++);
        } else if (*++format) {
            one(o, *format++, tm);
        } else {
            put(o, '%');        /* a % at the end is copied (the spec) */
        }
    }
}

size_t __cdecl strftime(char *s, size_t max, const char *format, const struct tm *tm)
{
    struct out o;

    if (max == 0)
        return 0;
    o.p = s;
    o.left = max - 1;           /* the terminator's byte */
    o.full = 0;
    conv(&o, format, tm);
    if (o.full)
        return 0;
    *o.p = '\0';
    return o.p - s;
}
