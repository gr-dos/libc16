/* The compiler's 32- and 64-bit arithmetic helpers (docs/spec/arith.md),
 * on operands that exercise every path. Prints one line per operand pair
 * or shift, in hex.
 *
 * The same source compiles on the host, where C's own 32- and 64-bit
 * arithmetic gives the expected output:
 *
 *   cc -fwrapv -o arith-host tests/arith/arith.c
 *   ./arith-host > tests/arith/arith.expect
 *
 * Values are printed a byte at a time so that printing needs no helper. */

#ifdef __WATCOMC__
#include "testio.h"
typedef long i32;
typedef unsigned long u32;
typedef long long i64;
typedef unsigned long long u64;
#define NL "\r\n"
#else
#include <stdint.h>
#include <stdio.h>
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
#define NL "\n"
static void t_puts(const char *s) { fputs(s, stdout); }
#endif

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

static const u32 ops32[] = {
    0x00000000UL, 0x00000001UL, 0x00000002UL, 0x00000003UL, 0x00000007UL,
    0x0000000aUL, 0x0000ffffUL, 0x00010000UL, 0x0001ffffUL, 0x00123456UL,
    0x12345678UL, 0x7fffffffUL, 0x80000000UL, 0x80000001UL, 0xdeadbeefUL,
    0xffff0000UL, 0xfffffffdUL, 0xfffffffeUL, 0xffffffffUL
};

static const u64 ops64[] = {
    0x0000000000000000ULL, 0x0000000000000001ULL, 0x0000000000000002ULL,
    0x0000000000000007ULL, 0x000000000000000aULL, 0x000000000000ffffULL,
    0x0000000000010000ULL, 0x00000000ffffffffULL, 0x0000000100000000ULL,
    0x0000000123456789ULL, 0x0000ffff0000ffffULL, 0x0123456789abcdefULL,
    0x7fffffffffffffffULL, 0x8000000000000000ULL, 0x8000000000000001ULL,
    0xfedcba9876543210ULL, 0xffffffff00000000ULL, 0xfffffffffffffffdULL,
    0xffffffffffffffffULL
};

static const u64 shifted[] = {
    0x0000000000000001ULL, 0x0123456789abcdefULL, 0x8000000000000000ULL,
    0xfedcba9876543210ULL, 0xffffffffffffffffULL
};

static void hex(const void *p, unsigned n)
{
    static const char digits[] = "0123456789abcdef";
    const unsigned char *b = (const unsigned char *)p + n;
    char buf[18];
    char *q = buf;

    *q++ = ' ';
    while (n--) {
        --b;
        *q++ = digits[*b >> 4];
        *q++ = digits[*b & 15];
    }
    *q = '\0';
    t_puts(buf);
}

static void none(void)
{
    t_puts(" -");
}

static void test32(u32 a, u32 b)
{
    i32 sa = (i32)a, sb = (i32)b;
    u32 r;
    i32 s;

    t_puts("l");
    hex(&a, 4);
    hex(&b, 4);
    r = a * b;
    hex(&r, 4);
    s = sa * sb;
    hex(&s, 4);
    if (b == 0) {
        none(); none(); none(); none();
    } else {
        r = a / b;
        hex(&r, 4);
        r = a % b;
        hex(&r, 4);
        if (sa == (i32)0x80000000UL && sb == -1) {
            none(); none();
        } else {
            s = sa / sb;
            hex(&s, 4);
            s = sa % sb;
            hex(&s, 4);
        }
    }
    t_puts(NL);
}

static void test64(u64 a, u64 b)
{
    i64 sa = (i64)a, sb = (i64)b;
    u64 r;
    i64 s;

    t_puts("q");
    hex(&a, 8);
    hex(&b, 8);
    r = a * b;
    hex(&r, 8);
    s = sa * sb;
    hex(&s, 8);
    if (b == 0) {
        none(); none(); none(); none();
    } else {
        r = a / b;
        hex(&r, 8);
        r = a % b;
        hex(&r, 8);
        if (sa == (i64)0x8000000000000000ULL && sb == -1) {
            none(); none();
        } else {
            s = sa / sb;
            hex(&s, 8);
            s = sa % sb;
            hex(&s, 8);
        }
    }
    t_puts(NL);
}

static void test_shift(u64 a, unsigned n)
{
    i64 s = (i64)a;
    u64 r;
    unsigned char c = (unsigned char)n;

    t_puts("s");
    hex(&a, 8);
    hex(&c, 1);
    r = a << n;
    hex(&r, 8);
    s = (i64)a << n;
    hex(&s, 8);
    r = a >> n;
    hex(&r, 8);
    s = (i64)a >> n;
    hex(&s, 8);
    t_puts(NL);
}

/* Constant operands and compound assignment, which the compiler also
 * sends to the helpers. */
static void test_const(u64 a)
{
    u64 r;
    i64 s;
    u32 l = (u32)a;

    t_puts("c");
    hex(&a, 8);
    r = a * 1000000007ULL;
    hex(&r, 8);
    r = a / 1000000007ULL;
    hex(&r, 8);
    s = (i64)a;
    s %= -1000000007LL;
    hex(&s, 8);
    r = a;
    r <<= 13;
    hex(&r, 8);
    s = (i64)a >> 13;
    hex(&s, 8);
    l *= 100003UL;
    hex(&l, 4);
    l /= 10UL;
    hex(&l, 4);
    t_puts(NL);
}

int main(void)
{
    unsigned i, j;

    for (i = 0; i < COUNT(ops32); i++)
        for (j = 0; j < COUNT(ops32); j++)
            test32(ops32[i], ops32[j]);
    for (i = 0; i < COUNT(ops64); i++)
        for (j = 0; j < COUNT(ops64); j++)
            test64(ops64[i], ops64[j]);
    for (i = 0; i < COUNT(shifted); i++)
        for (j = 0; j < 64; j++)
            test_shift(shifted[i], j);
    for (i = 0; i < COUNT(ops64); i++)
        test_const(ops64[i]);
    return 0;
}
