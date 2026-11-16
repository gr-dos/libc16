/* strtol (docs/spec/format.md): each case's value, *endptr (as an offset
 * into the string) and errno, which is set to a marker first so that a
 * case leaving it unchanged is seen. Prints "not ok LINE" and the case's
 * index for each failure, then "ok" if there were none. */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "check.h"

#define COUNT(a) (sizeof (a) / sizeof (a)[0])
#define MARK 99                 /* errno before each call */

struct test {
    const char *s;
    int base;
    long value;
    unsigned end;
    int err;                    /* errno afterwards: MARK if unchanged */
};

static const struct test tests[] = {
    /* decimal, signs and white space */
    { "0", 10, 0, 1, MARK },
    { "123", 10, 123, 3, MARK },
    { " \t\n\v\f\r42xyz", 10, 42, 8, MARK },
    { "+7", 10, 7, 2, MARK },
    { "-1", 10, -1L, 2, MARK },
    { "-0", 10, 0, 2, MARK },
    { "32767", 10, 32767L, 5, MARK },
    { "32768", 10, 32768L, 5, MARK },
    { "-32768", 10, -32768L, 6, MARK },
    { "65536", 10, 65536L, 5, MARK },
    { "-305419896", 10, -0x12345678L, 10, MARK },
    { "00000000000000000000000000001", 10, 1, 29, MARK },
    { "1e3", 10, 1, 1, MARK },
    /* the limits, and overflow: every digit is read */
    { "2147483647", 10, LONG_MAX, 10, MARK },
    { "2147483648", 10, LONG_MAX, 10, ERANGE },
    { "-2147483647", 10, -LONG_MAX, 11, MARK },
    { "-2147483648", 10, LONG_MIN, 11, MARK },
    { "-2147483649", 10, LONG_MIN, 11, ERANGE },
    { "4294967295", 10, LONG_MAX, 10, ERANGE },
    { "4294967296", 10, LONG_MAX, 10, ERANGE },
    { "-4294967296", 10, LONG_MIN, 11, ERANGE },
    { "99999999999999999999z", 10, LONG_MAX, 20, ERANGE },
    { "  -99999999999999999999z", 10, LONG_MIN, 23, ERANGE },
    { "+2147483648", 10, LONG_MAX, 11, ERANGE },
    /* no number: 0, and *endptr is nptr */
    { "", 10, 0, 0, MARK },
    { "   ", 10, 0, 0, MARK },
    { "+", 10, 0, 0, MARK },
    { "-", 10, 0, 0, MARK },
    { "  -  5", 10, 0, 0, MARK },
    { "+-5", 10, 0, 0, MARK },
    { "--5", 10, 0, 0, MARK },
    { "z", 10, 0, 0, MARK },
    { "-x", 16, 0, 0, MARK },
    { "8", 8, 0, 0, MARK },
    /* other bases and letters of either case */
    { "1010102", 2, 42, 6, MARK },
    { "-101", 2, -5, 4, MARK },
    { "1111111111111111111111111111111", 2, LONG_MAX, 31, MARK },
    { "10000000000000000000000000000000", 2, LONG_MAX, 32, ERANGE },
    { "-10000000000000000000000000000000", 2, LONG_MIN, 33, MARK },
    { "-10000000000000000000000000000001", 2, LONG_MIN, 33, ERANGE },
    { "777", 8, 511, 3, MARK },
    { "17777777777", 8, LONG_MAX, 11, MARK },
    { "20000000000", 8, LONG_MAX, 11, ERANGE },
    { "-20000000000", 8, LONG_MIN, 12, MARK },
    { "Az", 11, 10, 1, MARK },
    { "DeadBee", 16, 0xdeadbeeL, 7, MARK },
    { "7fffffff", 16, LONG_MAX, 8, MARK },
    { "80000000", 16, LONG_MAX, 8, ERANGE },
    { "-80000000", 16, LONG_MIN, 9, MARK },
    { "-80000001", 16, LONG_MIN, 9, ERANGE },
    { "ffffffff", 16, LONG_MAX, 8, ERANGE },
    { "zZ", 36, 1295, 2, MARK },
    { "-zik0zk", 36, LONG_MIN, 7, MARK },
    { "zik0zk", 36, LONG_MAX, 6, ERANGE },
    { "@", 36, 0, 0, MARK },
    { "[", 36, 0, 0, MARK },
    { "`", 36, 0, 0, MARK },
    { "{", 36, 0, 0, MARK },
    { "\x80", 36, 0, 0, MARK },
    /* prefixes */
    { "0x1F", 0, 31, 4, MARK },
    { "0X1f", 16, 31, 4, MARK },
    { "0x1g", 0, 1, 3, MARK },
    { "-0x10", 0, -16, 5, MARK },
    { "-0x80000000", 0, LONG_MIN, 11, MARK },
    { "0x80000000", 0, LONG_MAX, 10, ERANGE },
    { "0x", 0, 0, 1, MARK },
    { "-0x", 16, 0, 2, MARK },
    { "0xg", 16, 0, 1, MARK },
    { "0x10", 10, 0, 1, MARK },
    { "017", 0, 15, 3, MARK },
    { "-017", 0, -15, 4, MARK },
    { "08", 0, 0, 1, MARK },
    { "9", 0, 9, 1, MARK },
    { "+12", 0, 12, 3, MARK },
    /* invalid bases: 0, *endptr is nptr, and EINVAL */
    { "12", 1, 0, 0, EINVAL },
    { " 12", -1, 0, 0, EINVAL },
    { "12", 37, 0, 0, EINVAL },
    { "-12", -32767 - 1, 0, 0, EINVAL }
};

#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data, a string outside DGROUP: strtol then has DS at the
 * string's segment, not DGROUP, when it sets errno (docs/ABI.md, "Memory
 * models"). */
static char __far far_s[] = " -99999999999z";

static void far_string(void)
{
    char *end = 0;

    errno = MARK;
    CHECK(strtol(far_s, &end, 0) == LONG_MIN);
    CHECK(end == far_s + 13 && errno == ERANGE);
    errno = MARK;
    CHECK(strtol(far_s + 6, &end, 16) == 0x9999999L);
    CHECK(end == far_s + 13 && errno == MARK);
}
#endif

int main(void)
{
    const struct test *t;
    char *end;
    long v;
    unsigned i;
    int ok;

#if defined(__COMPACT__) || defined(__LARGE__)
    far_string();
#endif

    for (i = 0; i < COUNT(tests); i++) {
        t = &tests[i];
        errno = MARK;
        end = 0;
        v = strtol(t->s, &end, t->base);
        ok = v == t->value && end == t->s + t->end && errno == t->err;
        /* The same without endptr. */
        errno = MARK;
        v = strtol(t->s, NULL, t->base);
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
