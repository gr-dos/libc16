/* atoi (docs/spec/format.md): each case's value, including libc16's wrap
 * to the low 16 bits of the value read modulo 2^32. Prints "not ok LINE"
 * and the case's index for each failure, then "ok" if there were none.
 * That errno is unchanged shows in tests/format/members: this program
 * does not use errno, and atoi does not link it. */

#include <stdlib.h>
#include "check.h"

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

struct test {
    const char *s;
    int value;
};

static const struct test tests[] = {
    { "0", 0 },
    { "42", 42 },
    { " \t\n\v\f\r7", 7 },
    { "  -42x", -42 },
    { "+17", 17 },
    { "-0", 0 },
    { "007", 7 },
    { "1e3", 1 },
    { "32767", 32767 },
    { "-32768", -32767 - 1 },
    /* the wrap */
    { "32768", -32767 - 1 },
    { "65535", -1 },
    { "65536", 0 },
    { "65537", 1 },
    { "-65537", -1 },
    { "2147483647", -1 },
    { "-2147483648", 0 },
    { "4294967295", -1 },
    { "4294967296", 0 },
    { "4294967297", 1 },
    { "99999999999", -6145 },
    /* no number */
    { "", 0 },
    { "   ", 0 },
    { "-", 0 },
    { "- 5", 0 },
    { "+-5", 0 },
    { "x12", 0 },
    { "0x10", 0 },
    { "\x80" "5", 0 }
};

#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data, a string outside DGROUP too. */
static char __far far_s[] = " -1234x";
#endif

int main(void)
{
    unsigned i;
    int ok;

#if defined(__COMPACT__) || defined(__LARGE__)
    CHECK(atoi(far_s) == -1234);
#endif

    for (i = 0; i < COUNT(tests); i++) {
        ok = atoi(tests[i].s) == tests[i].value;
        CHECK(ok);
        if (!ok) {
            t_puts("case ");
            t_putu(i);
            t_puts("\r\n");
        }
    }
    return done();
}
