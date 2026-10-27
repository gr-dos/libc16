/* tzset - libc16 (docs/spec/time.md) */

#include <stdlib.h>
#include <time.h>
#include "time16.h"

static int is_letter(char c)
{
    return (unsigned)((c | 0x20) - 'a') < 26;
}

static int is_digit(char c)
{
    return (unsigned)(c - '0') < 10;
}

/* Copies a zone name of three or more letters from *p to name, cut to 15
 * letters, and advances *p past it; returns 0 if there are fewer than 3. */
static int name(const char **p, char *name)
{
    const char *s = *p;
    unsigned n = 0;

    while (is_letter(s[n]))
        n++;
    if (n < 3)
        return 0;
    *p = s + n;
    if (n > 15)
        n = 15;
    name[n] = '\0';
    while (n-- != 0)
        name[n] = s[n];
    return 1;
}

/* Reads a number of min to 2 digits, at most limit, from *p, advancing
 * it; returns -1 if there is none or it is out of range. */
static int number(const char **p, unsigned min, int limit)
{
    const char *s = *p;
    int v = 0;
    unsigned n = 0;

    while (n < 2 && is_digit(s[n]))
        v = v * 10 + (s[n++] - '0');
    if (n < min || v > limit)
        return -1;
    *p = s + n;
    return v;
}

static int parse(const char *p, char *std, char *dst, long *west, int *dl)
{
    int neg = 0, h, m = 0, s = 0;

    if (!name(&p, std))
        return 0;
    if (*p == '+' || *p == '-')
        neg = *p++ == '-';
    if ((h = number(&p, 1, 24)) < 0)
        return 0;
    if (*p == ':') {
        p++;
        if ((m = number(&p, 2, 59)) < 0)
            return 0;
        if (*p == ':') {
            p++;
            if ((s = number(&p, 2, 59)) < 0)
                return 0;
        }
    }
    *west = (long)h * 3600 + m * 60 + s;
    if (neg)
        *west = -*west;
    *dl = 0;
    dst[0] = '\0';
    if (*p != '\0') {
        if (!name(&p, dst) || *p != '\0')
            return 0;
        *dl = 1;
    }
    return 1;
}

void __cdecl tzset(void)
{
    const char *tz = getenv("TZ");
    char std[16], dst[16];
    long west;
    int dl, i;

    __libc16_tzdone = 1;
    if (tz == NULL || !parse(tz, std, dst, &west, &dl)) {
        std[0] = 'U', std[1] = 'T', std[2] = 'C', std[3] = '\0';
        dst[0] = '\0';
        west = 0;
        dl = 0;
    }
    for (i = 0; i < 16; i++) {
        __libc16_tzstd[i] = std[i];
        __libc16_tzdst[i] = dst[i];
    }
    timezone = west;
    daylight = dl;
}
