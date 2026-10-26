/* malloc and free (docs/spec/heap.md). Prints one line per check, "ok"
 * or what went wrong; the exit status is 1 if any failed. bigstk.c
 * includes this file to run the same checks with a larger stack. */

#include <errno.h>
#include <stdlib.h>
#include "testio.h"

extern char __cdecl __libc16_stack_top[];

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

/* The stack linked (option stack), and how much of it the checks use. */
#ifndef STACK
#define STACK 2048
#define DEPTH 1024
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

/* A block is above static data and the stack, and even. */
static int placed(const void *p)
{
    return p && !((unsigned)p & 1) && (const char *)p >= __libc16_stack_top
        && (const char *)p > &last_static;
}

/* Uses about n bytes of stack, below anything malloc returned. */
static unsigned deep(unsigned n)
{
    volatile char pad[64];

    pad[0] = (char)n;
    return n < 64 ? pad[0] : deep(n - 64) + pad[0];
}

static const unsigned sizes[] = { 1, 2, 3, 10, 64, 255, 256, 1000, 0, 7 };

static void distinct(void)
{
    void *p[COUNT(sizes)];
    unsigned i;
    int ok = 1;

    for (i = 0; i < COUNT(sizes); i++) {
        p[i] = malloc(sizes[i]);
        if (!placed(p[i]))
            ok = 0;
        else
            fill(p[i], sizes[i], (unsigned char)(i * 37));
    }
    check("allocations are placed", ok);
    deep(DEPTH);
    for (i = 0; i < COUNT(sizes); i++)
        if (ok && !filled(p[i], sizes[i], (unsigned char)(i * 37)))
            ok = 0;
    check("allocations keep their contents", ok);
    for (i = 0; i < COUNT(sizes); i++)
        free(p[i]);
}

static void reuse(void)
{
    char *a = malloc(100), *b = malloc(100), *c = malloc(100), *d;

    free(b);
    d = malloc(100);
    check("free and reuse", a && b && c && d == b);
    free(a);
    free(d);
    d = malloc(200);
    check("free neighbours merge", d == a);
    free(d);
    free(c);
    free(NULL);
}

static void zero(void)
{
    void *a = malloc(0), *b = malloc(0);

    check("malloc(0) is unique", placed(a) && placed(b) && a != b);
    free(a);
    free(b);
}

static void exhaust(void)
{
    static void *p[80];
    unsigned n, i;
    char *all;

    for (n = 0; n < COUNT(p); n++) {
        errno = 0;
        p[n] = malloc(1000);
        if (!p[n])
            break;
        fill(p[n], 1000, (unsigned char)n);
    }
    check("exhaustion returns NULL with ENOMEM", n > 0 && n < COUNT(p) && errno == ENOMEM);
    for (i = 0; i < n; i++)
        if (!filled(p[i], 1000, (unsigned char)i))
            break;
    check("exhausted heap keeps contents", i == n);
    errno = 0;
    check("malloc(65535) fails with ENOMEM", !malloc(65535u) && errno == ENOMEM);
    errno = 0;
    check("malloc(65533) fails with ENOMEM", !malloc(65533u) && errno == ENOMEM);
    free(p[n - 1]);
    p[n - 1] = malloc(1000);
    check("recovery after free", p[n - 1] != NULL);
    for (i = 0; i < n; i++)
        free(p[i]);
    all = malloc(n * 1000u);
    check("one block of the total after freeing all", placed(all));
    if (all)
        fill(all, n * 1000u, 1);
    free(all);
}

int main(void)
{
    check("the stack is all below the heap",
        (unsigned)__libc16_stack_top - (unsigned)&last_static >= STACK);
    distinct();
    reuse();
    zero();
    exhaust();
    distinct();
    return failed;
}
