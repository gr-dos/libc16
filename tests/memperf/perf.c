/* The time of memcpy, memmove (upwards and downwards), memset and
 * _fmemcpy (docs/spec/string.md, "Copies and fills"), in every model:
 * each moves or fills 20,000 bytes REPS times, and so does a byte loop
 * compiled here, the way libc16 0.18's members copied (gr-dos/libc16#42).
 * The check passes when the member takes at most half the loop's time.
 *
 * Time is the BIOS tick count (18.2 per second) of the emulated machine
 * (tests/bufperf/perf.c). Each line prints the member's ticks and the
 * loop's; the .expect file matches the numbers with a pattern. */

#include <dos.h>
#include <string.h>
#include "check.h"

#define N 20000u
#define REPS 20

static unsigned char buf[2 * N];
static unsigned char __far fbuf[N];

static unsigned long ticks(void)
{
    volatile unsigned long __far *t = MK_FP(0x40, 0x6C);
    unsigned long a, b;

    do {
        a = *t;
        b = *t;
    } while (a != b);
    return a;
}

/* a tick boundary, so that each run starts at the beginning of a tick */
static unsigned long tick_edge(void)
{
    unsigned long t = ticks(), u;

    while ((u = ticks()) == t)
        ;
    return u;
}

static void byte_copy(unsigned char *d, const unsigned char *s, unsigned n)
{
    while (n-- != 0)
        *d++ = *s++;
}

static void byte_fcopy(unsigned char __far *d, const unsigned char __far *s, unsigned n)
{
    while (n-- != 0)
        *d++ = *s++;
}

static void byte_down(unsigned char *d, const unsigned char *s, unsigned n)
{
    d += n;
    s += n;
    while (n-- != 0)
        *--d = *--s;
}

static void byte_fill(unsigned char *d, int c, unsigned n)
{
    while (n-- != 0)
        *d++ = (unsigned char)c;
}

static void report(const char *name, unsigned long tm, unsigned long tl)
{
    t_puts(name);
    t_puts(": ");
    t_putu((unsigned)tm);
    t_puts(" ticks, byte loop ");
    t_putu((unsigned)tl);
    t_puts(" ticks\r\n");
    CHECK(tl >= 4 && tm * 2 <= tl);
}

int main(void)
{
    unsigned long t0, tm, tl;
    int r;

    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        memcpy(buf, buf + N, N);
    tm = ticks() - t0;
    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        byte_copy(buf, buf + N, N);
    tl = ticks() - t0;
    report("memcpy", tm, tl);

    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        memmove(buf, buf + 1, N);
    tm = ticks() - t0;
    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        byte_copy(buf, buf + 1, N);
    tl = ticks() - t0;
    report("memmove up", tm, tl);

    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        memmove(buf + 1, buf, N);
    tm = ticks() - t0;
    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        byte_down(buf + 1, buf, N);
    tl = ticks() - t0;
    report("memmove down", tm, tl);

    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        memset(buf, r, N);
    tm = ticks() - t0;
    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        byte_fill(buf, r, N);
    tl = ticks() - t0;
    report("memset", tm, tl);

    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        _fmemcpy(fbuf, buf, N);
    tm = ticks() - t0;
    t0 = tick_edge();
    for (r = 0; r < REPS; r++)
        byte_fcopy(fbuf, buf, N);
    tl = ticks() - t0;
    report("_fmemcpy", tm, tl);
    return done();
}
