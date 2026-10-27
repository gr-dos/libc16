/* Shared by the heap test programs: one line per check, "ok" or
 * "FAILED", and the exit status 1 if any failed; patterned contents; and
 * where a block may be (docs/spec/heap.md). */

#ifndef CHECK_H
#define CHECK_H

#include <dos.h>
#include "testio.h"

extern char __cdecl __libc16_stack_top[];

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

/* FAR_DATA in the compact and large models, where the heap may pass
 * 64 KiB. A check that differs by model prints "near data: " or
 * "far data: " first, which the .expect files match with a pattern. */
#if defined(__COMPACT__) || defined(__LARGE__)
#define FAR_DATA 1
#define DATA "far data: "
#else
#define DATA "near data: "
#endif

static int failed;
static char last_static;

static void check(const char *what, int ok)
{
    t_puts(what);
    t_puts(ok ? " ok\r\n" : " FAILED\r\n");
    if (!ok)
        failed = 1;
}

static void fill(void *p, unsigned n, unsigned char v)
{
    unsigned char *b = p;

    while (n--)
        *b++ = v++;
}

static int filled(const void *p, unsigned n, unsigned char v)
{
    const unsigned char *b = p;

    while (n--)
        if (*b++ != v++)
            return 0;
    return 1;
}

/* p's linear address with far data, where pointers into different
 * segments compare by offset only; its offset in the small model. */
#ifdef FAR_DATA
static unsigned long linear(const void *p)
{
    return ((unsigned long)FP_SEG(p) << 4) + FP_OFF(p);
}

/* The segment of a pointer lvalue; 0 for all of DGROUP in the small
 * model. */
#define SEG(p) FP_SEG(p)
#else
#define linear(p) ((unsigned)(p))
#define SEG(p) 0
#endif

/* A block of n bytes is above static data and the stack, even, and
 * inside one segment: its offset plus n is at most 10000h. */
static int placed(const void *p, unsigned n)
{
    return p && !((unsigned)p & 1) && linear(p) >= linear(__libc16_stack_top)
        && linear(p) > linear(&last_static) && 0x10000ul - (unsigned)p >= n;
}

#endif
