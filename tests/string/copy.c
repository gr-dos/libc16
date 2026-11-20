/* memcpy, memmove, memset and _fmemcpy (docs/spec/string.md, "Copies and
 * fills"): every length up to 40 at every alignment of source and
 * destination, odd and even, with guard bytes around the destination;
 * memmove's overlaps in both directions; lengths above 32767; the
 * direction flag clear after each call; and, in the small model, DS kept. */

#include <string.h>
#include "check.h"

static unsigned get_flags(void);
#pragma aux get_flags = \
    "pushf" \
    "pop ax" \
    value [ax];

static unsigned get_ds(void);
#pragma aux get_ds = \
    "mov ax, ds" \
    value [ax];

static unsigned get_ss(void);
#pragma aux get_ss = \
    "mov ax, ss" \
    value [ax];

#define DF 0x0400u
#define GUARD 0xEE
#define MAXN 40
#define BUFN 64

/* More than 32767 bytes, so that n does not fit in an int */
#define BIG 40000u
static unsigned char big[BIG];
/* Outside DGROUP in every model */
static unsigned char __far fbig[BIG];
static unsigned char __far fbuf[BUFN];

static unsigned char src[BUFN], dst[BUFN], ref[BUFN];

static unsigned char pat(unsigned i)
{
    return (unsigned char)(i * 7 + 3);
}

/* the direction flag is clear and, in the small model, DS is DGROUP */
static int regs_ok(void)
{
#if defined(__SMALL__)
    if (get_ds() != get_ss())
        return 0;
#endif
    return (get_flags() & DF) == 0;
}

/* dst[d .. d+n) holds pat(s ..) and the rest of dst is GUARD */
static int copied(const unsigned char __far *b, unsigned s, unsigned d, unsigned n)
{
    unsigned i;

    for (i = 0; i < BUFN; i++)
        if (b[i] != (i >= d && i < d + n ? pat(i - d + s) : GUARD))
            return 0;
    return 1;
}

static void fill_src(unsigned char __far *b)
{
    unsigned i;

    for (i = 0; i < BUFN; i++)
        b[i] = pat(i);
}

static void fill_guard(unsigned char __far *b)
{
    unsigned i;

    for (i = 0; i < BUFN; i++)
        b[i] = GUARD;
}

static void check_memcpy(void)
{
    unsigned s, d, n;
    int bad = 0;

    fill_src(src);
    for (s = 0; s < 4; s++)
        for (d = 0; d < 4; d++)
            for (n = 0; n <= MAXN; n++) {
                fill_guard(dst);
                if (memcpy(dst + d, src + s, n) != dst + d || !regs_ok()
                    || !copied(dst, s, d, n))
                    bad++;
            }
    CHECK(bad == 0);
}

static void check_fmemcpy(void)
{
    unsigned s, d, n;
    int bad = 0;

    /* DGROUP to far, far to DGROUP and far to far */
    fill_src(src);
    for (s = 0; s < 4; s++)
        for (d = 0; d < 4; d++)
            for (n = 0; n <= MAXN; n++) {
                fill_guard(fbuf);
                if (_fmemcpy(fbuf + d, src + s, n) != fbuf + d || !regs_ok()
                    || !copied(fbuf, s, d, n))
                    bad++;
                fill_guard(dst);
                if (_fmemcpy(dst + d, fbuf + d, n) != (void __far *)(dst + d)
                    || !regs_ok() || !copied(dst, s, d, n))
                    bad++;
            }
    CHECK(bad == 0);
    fill_src(fbuf);
    for (s = 0; s < 4; s++)
        for (d = 0; d < 4; d++)
            for (n = 0; n <= MAXN; n++) {
                fill_guard(fbig);
                if (_fmemcpy(fbig + d, fbuf + s, n) != fbig + d || !regs_ok()
                    || !copied(fbig, s, d, n))
                    bad++;
            }
    CHECK(bad == 0);
}

static void check_memset(void)
{
    static const int cs[] = { 0, 0x41, 0x141, -1, 0x80, 0x7f00 };
    unsigned c, d, n, i;
    int bad = 0;

    for (c = 0; c < sizeof cs / sizeof cs[0]; c++)
        for (d = 0; d < 4; d++)
            for (n = 0; n <= MAXN; n++) {
                unsigned char v = (unsigned char)cs[c];

                fill_guard(dst);
                if (memset(dst + d, cs[c], n) != dst + d || !regs_ok())
                    bad++;
                for (i = 0; i < BUFN; i++)
                    if (dst[i] != (i >= d && i < d + n ? v : GUARD))
                        bad++;
            }
    CHECK(bad == 0);
}

/* memmove within one buffer: every source and destination offset up to
 * 7 apart, so every overlap in both directions */
static void check_memmove(void)
{
    unsigned s, d, n, i;
    int bad = 0;

    for (s = 0; s < 8; s++)
        for (d = 0; d < 8; d++)
            for (n = 0; n <= MAXN; n++) {
                fill_src(dst);
                fill_src(ref);
                for (i = 0; i < n; i++)
                    ref[d + i] = pat(s + i);
                if (memmove(dst + d, dst + s, n) != dst + d || !regs_ok())
                    bad++;
                for (i = 0; i < BUFN; i++)
                    if (dst[i] != ref[i])
                        bad++;
            }
    CHECK(bad == 0);
}

static unsigned bigpat(unsigned i)
{
    return (unsigned char)(i ^ (i >> 8));
}

/* lengths above 32767, odd and even */
static void check_big(void)
{
    unsigned i;
    int bad = 0;

    CHECK(memset(big, 0x5A, BIG) == big && regs_ok());
    for (i = 0; i < BIG; i++)
        if (big[i] != 0x5A)
            bad++;
    CHECK(bad == 0);

    for (i = 0; i < BIG; i++)
        big[i] = (unsigned char)bigpat(i);
    /* upwards by one, an odd count: copied downwards */
    CHECK(memmove(big + 1, big, BIG - 1) == big + 1 && regs_ok());
    bad = big[0] != bigpat(0);
    for (i = 1; i < BIG; i++)
        if (big[i] != bigpat(i - 1))
            bad++;
    CHECK(bad == 0);
    /* back down by one, an even count: copied upwards */
    CHECK(memmove(big, big + 1, BIG - 2) == big && regs_ok());
    for (i = 0; i < BIG - 2; i++)
        if (big[i] != bigpat(i))
            bad++;
    CHECK(bad == 0);

    for (i = 0; i < BIG; i++)
        big[i] = (unsigned char)bigpat(i);
    CHECK(_fmemcpy(fbig, big, BIG) == fbig && regs_ok());
    for (i = 0; i < BIG; i++)
        if (fbig[i] != bigpat(i))
            bad++;
    CHECK(bad == 0);
    memset(big, 0, BIG);
    CHECK(_fmemcpy(big + 1, fbig, BIG - 1) == (void __far *)(big + 1) && regs_ok());
    bad = big[0] != 0;
    for (i = 1; i < BIG; i++)
        if (big[i] != bigpat(i - 1))
            bad++;
    CHECK(bad == 0);

#if defined(__COMPACT__) || defined(__LARGE__)
    /* with far data, memcpy and memset outside DGROUP */
    memset(big, 0, BIG);
    CHECK(memcpy(big, fbig + 1, BIG - 1) == big && regs_ok());
    for (i = 0; i < BIG - 1; i++)
        if (big[i] != bigpat(i + 1))
            bad++;
    CHECK(bad == 0 && big[BIG - 1] == 0);
    CHECK(memset(fbig + 1, 0xA5, BIG - 2) == fbig + 1 && regs_ok());
    bad = fbig[0] != bigpat(0) || fbig[BIG - 1] != bigpat(BIG - 1);
    for (i = 1; i < BIG - 1; i++)
        if (fbig[i] != 0xA5)
            bad++;
    CHECK(bad == 0);
#endif
}

int main(void)
{
    check_memcpy();
    check_fmemcpy();
    check_memset();
    check_memmove();
    check_big();
    return done();
}
