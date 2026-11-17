/* setjmp and longjmp (docs/spec/setjmp.md), with the assembly checks in
 * tests/setjmp/sjasm.asm */

#include <setjmp.h>
#include <stdlib.h>
#include "check.h"

int sj_regs(void);
void __cdecl sj_dirty(jmp_buf env, int val);

static unsigned get_sp(void);
#pragma aux get_sp = \
    "mov ax, sp" \
    value [ax];

static unsigned get_ds(void);
#pragma aux get_ds = \
    "mov ax, ds" \
    value [ax];

static unsigned get_ss(void);
#pragma aux get_ss = \
    "mov ax, ss" \
    value [ax];

/* The direction flag (bit 10 of the flags) */
static unsigned get_df(void);
#pragma aux get_df = \
    "pushf" \
    "pop ax" \
    "and ax, 400h" \
    value [ax];

static jmp_buf env;
#if defined(__COMPACT__) || defined(__LARGE__)
static jmp_buf __far fenv;      /* outside DGROUP */
#endif

static int depth;
static long counter;

/* The value setjmp returns when longjmp(e, val) is called */
static int value_of(int *e, int val)
{
    volatile int first = 1;
    int r = setjmp(e);

    if (r == 0) {
        if (!first)
            return -999;
        first = 0;
        longjmp(e, val);
        return -998;
    }
    return r;
}

/* 20 nested calls, each with a frame of its own, then a jump */
static void dive(int *e, int n)
{
    volatile char pad[40];

    pad[0] = (char)n;
    depth++;
    if (n == 0)
        longjmp(e, 42);
    dive(e, n - 1);
    depth += pad[0];            /* not reached */
}

static int calls;

static int __cdecl jump_cmp(const void *a, const void *b)
{
    (void)a;
    (void)b;
    calls++;
    longjmp(env, 3);
    return 0;
}

static void test_values(int *e)
{
    CHECK(value_of(e, 5) == 5);
    CHECK(value_of(e, -1) == -1);
    CHECK(value_of(e, 32767) == 32767);
    CHECK(value_of(e, 0) == 1);
    CHECK(value_of(e, 1) == 1);
}

static void test_nested(int *e)
{
    int r;

    depth = 0;
    r = setjmp(e);
    if (r == 0) {
        dive(e, 20);
        CHECK(0);
    } else {
        CHECK(r == 42 && depth == 21);
    }
}

static void test_locals(int *e)
{
    volatile int v = 1;
    volatile long lv = 100000L;
    int keep = depth * 3 + 7;   /* not changed after setjmp */
    int want = keep;
    int r;

    counter = 1;
    r = setjmp(e);
    if (r == 0) {
        v = 2;
        lv = -5L;
        counter = 77;
        longjmp(e, 9);
    }
    CHECK(r == 9 && v == 2 && lv == -5L && counter == 77 && keep == want);
}

static void test_repeat(int *e)
{
    static unsigned sp0;
    volatile unsigned n = 0;
    int r;

    r = setjmp(e);
    if (r == 0) {
        sp0 = get_sp();
    } else if (get_sp() != sp0) {
        CHECK(0);
        return;
    }
    if (++n <= 1000)
        longjmp(e, (int)n);
    CHECK(n == 1001 && r == 1000);
}

static void test_dirty(int *e)
{
    int r = setjmp(e);

    if (r == 0) {
        sj_dirty(e, 11);
        CHECK(0);
    }
    /* DS first, before other code of this function can load it */
    CHECK(get_ds() == get_ss());
    CHECK(get_df() == 0);
    CHECK(r == 11);
}

static void all(int *e)
{
    test_values(e);
    test_nested(e);
    test_locals(e);
    test_repeat(e);
    test_dirty(e);
}

int main(void)
{
    jmp_buf local;
    int a[4] = { 4, 3, 2, 1 };
    int r;

    CHECK(sizeof(jmp_buf) == 16);

    all(env);                   /* in DGROUP */
    all(local);                 /* on the stack */
#if defined(__COMPACT__) || defined(__LARGE__)
    all(fenv);                  /* in a far segment */
#endif

    /* out of qsort's comparison function */
    r = setjmp(env);
    if (r == 0) {
        qsort(a, 4, sizeof a[0], jump_cmp);
        CHECK(0);
    }
    CHECK(r == 3 && calls == 1);

    CHECK(sj_regs() == 0);
    return done();
}
