/* libc16_ltoa (docs/spec/format.md): each value in each radix, then
 * radixes outside 2 to 36 with a negative value. Prints "VALUE RADIX
 * RESULT" per call, VALUE being the index into values[]. Each call must
 * return buf and write nothing past the terminator. */

#include <libc16.h>
#include "testio.h"

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

static const long values[] = {
    0L, 1L, -1L, 9L, -10L, 35L, -36L, 65535L, -65536L,
    1234567890L, -1234567890L, 0x7fffffffL, -0x7fffffffL - 1, -2L
};

static const int radixes[] = { 2, 3, 7, 8, 10, 16, 36 };
static const int bad_radixes[] = { -16, -1, 0, 1, 37, 100 };

/* With far data, buf is outside DGROUP, so DS is not SS while the
 * helper writes it (docs/ABI.md, "Memory models"). */
#if defined(__COMPACT__) || defined(__LARGE__)
static char __far buf[40];
#else
static char buf[40];
#endif
static int failed;

static void test(unsigned i, int radix)
{
    unsigned n;
    char *r;

    for (n = 0; n < sizeof buf; n++)
        buf[n] = '#';
    r = libc16_ltoa(values[i], buf, radix);
    t_putu(i);
    t_puts(" ");
    if (radix < 0) {
        t_puts("-");
        t_putu(-radix);
    } else {
        t_putu(radix);
    }
    t_puts(" ");
    t_puts(buf);
    for (n = 0; buf[n]; n++)
        ;
    if (r != buf || n > 33) {
        t_puts(" bad return or length");
        failed = 1;
    }
    for (n++; n < sizeof buf; n++)
        if (buf[n] != '#') {
            t_puts(" wrote past the end");
            failed = 1;
            break;
        }
    t_puts("\r\n");
}

int main(void)
{
    unsigned i, j;

    for (i = 0; i < COUNT(values); i++)
        for (j = 0; j < COUNT(radixes); j++)
            test(i, radixes[j]);
    for (j = 0; j < COUNT(bad_radixes); j++)
        test(COUNT(values) - 1, bad_radixes[j]);
    return failed;
}
