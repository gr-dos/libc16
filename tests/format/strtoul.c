/* strtoul (docs/spec/format.md): each case's value, *endptr (as an offset
 * into the string) and errno, which is set to a marker first so that a
 * case leaving it unchanged is seen. Prints "not ok LINE" and the case's
 * index for each failure, then "ok" if there were none. */

#include <errno.h>
#include <stdlib.h>
#include "check.h"

#define COUNT(a) (sizeof (a) / sizeof (a)[0])
#define MARK 99                 /* errno before each call */

struct test {
    const char *s;
    int base;
    unsigned long value;
    unsigned end;
    int err;                    /* errno afterwards: MARK if unchanged */
};

static const struct test tests[] = {
    /* decimal, signs and white space */
    { "0", 10, 0, 1, MARK },
    { "123", 10, 123, 3, MARK },
    { " \t\n\v\f\r42xyz", 10, 42, 8, MARK },
    { "+7", 10, 7, 2, MARK },
    { "-1", 10, 0xffffffffUL, 2, MARK },
    { "-0", 10, 0, 2, MARK },
    { "  -4294967295", 10, 1, 13, MARK },
    { "65535", 10, 65535UL, 5, MARK },
    { "65536", 10, 65536UL, 5, MARK },
    { "305419896", 10, 0x12345678UL, 9, MARK },
    { "00000000000000000000000000001", 10, 1, 29, MARK },
    { "1e3", 10, 1, 1, MARK },
    /* overflow: every digit is read, and the result is ULONG_MAX */
    { "4294967295", 10, 0xffffffffUL, 10, MARK },
    { "4294967296", 10, 0xffffffffUL, 10, ERANGE },
    { "4294967300", 10, 0xffffffffUL, 10, ERANGE },
    { "-4294967296", 10, 0xffffffffUL, 11, ERANGE },
    { "99999999999999999999z", 10, 0xffffffffUL, 20, ERANGE },
    /* no number: 0, and *endptr is nptr */
    { "", 10, 0, 0, MARK },
    { "   ", 10, 0, 0, MARK },
    { "+", 10, 0, 0, MARK },
    { "  -  5", 10, 0, 0, MARK },
    { "z", 10, 0, 0, MARK },
    { "-x", 16, 0, 0, MARK },
    { "8", 8, 0, 0, MARK },
    /* other bases and letters of either case */
    { "1010102", 2, 42, 6, MARK },
    { "11111111111111111111111111111111", 2, 0xffffffffUL, 32, MARK },
    { "100000000000000000000000000000000", 2, 0xffffffffUL, 33, ERANGE },
    { "210", 3, 21, 3, MARK },
    { "777", 8, 511, 3, MARK },
    { "37777777777", 8, 0xffffffffUL, 11, MARK },
    { "40000000000", 8, 0xffffffffUL, 11, ERANGE },
    { "Az", 11, 10, 1, MARK },
    { "1f", 16, 31, 2, MARK },
    { "DeadBeef", 16, 0xdeadbeefUL, 8, MARK },
    { "ffffffff", 16, 0xffffffffUL, 8, MARK },
    { "100000000", 16, 0xffffffffUL, 9, ERANGE },
    { "zZ", 36, 1295, 2, MARK },
    { "1z141z3", 36, 0xffffffffUL, 7, MARK },
    { "1z141z4", 36, 0xffffffffUL, 7, ERANGE },
    { "@", 36, 0, 0, MARK },
    { "[", 36, 0, 0, MARK },
    { "`", 36, 0, 0, MARK },
    { "{", 36, 0, 0, MARK },
    { "\x80", 36, 0, 0, MARK },
    { "\xe1", 36, 0, 0, MARK },
    /* prefixes */
    { "0x1F", 0, 31, 4, MARK },
    { "0X1f", 16, 31, 4, MARK },
    { "0x1g", 0, 1, 3, MARK },
    { "-0x10", 0, 0xfffffff0UL, 5, MARK },
    { "0x", 0, 0, 1, MARK },
    { "0x", 16, 0, 1, MARK },
    { "0xg", 16, 0, 1, MARK },
    { "0X", 0, 0, 1, MARK },
    { "0x10", 10, 0, 1, MARK },
    { "0x10", 36, 42804UL, 4, MARK },
    { "017", 0, 15, 3, MARK },
    { "08", 0, 0, 1, MARK },
    { "0", 0, 0, 1, MARK },
    { "9", 0, 9, 1, MARK },
    { "+12", 0, 12, 3, MARK },
    { "0xffffffff", 0, 0xffffffffUL, 10, MARK },
    { "0x100000000", 0, 0xffffffffUL, 11, ERANGE },
    /* invalid bases: 0, *endptr is nptr, and EINVAL */
    { "12", 1, 0, 0, EINVAL },
    { " 12", -1, 0, 0, EINVAL },
    { "12", 37, 0, 0, EINVAL },
    { "12", 100, 0, 0, EINVAL },
    { "12", -32767 - 1, 0, 0, EINVAL }
};

#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data, a string outside DGROUP: strtoul then has DS at the
 * string's segment, not DGROUP, when it sets errno (docs/ABI.md, "Memory
 * models"). */
static char __far far_s[] = " -99999999999z";

static void far_string(void)
{
    char *end = 0;

    errno = MARK;
    CHECK(strtoul(far_s, &end, 0) == 0xffffffffUL);
    CHECK(end == far_s + 13 && errno == ERANGE);
    errno = MARK;
    CHECK(strtoul(far_s + 5, &end, 16) == 0x99999999UL);
    CHECK(end == far_s + 13 && errno == MARK);
}
#endif

int main(void)
{
    const struct test *t;
    char *end;
    unsigned long v;
    unsigned i;
    int ok;

#if defined(__COMPACT__) || defined(__LARGE__)
    far_string();
#endif

    for (i = 0; i < COUNT(tests); i++) {
        t = &tests[i];
        errno = MARK;
        end = 0;
        v = strtoul(t->s, &end, t->base);
        ok = v == t->value && end == t->s + t->end && errno == t->err;
        /* The same without endptr. */
        errno = MARK;
        v = strtoul(t->s, NULL, t->base);
        ok = ok && v == t->value && errno == t->err;
        CHECK(ok);
        if (!ok) {
            t_puts("case ");
            t_putu(i);
            t_puts("\r\n");
        }
    }
    return done();
}
