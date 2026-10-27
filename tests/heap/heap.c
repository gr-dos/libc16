/* malloc and free (docs/spec/heap.md), in every model. Prints one line
 * per check (check.h); the exit status is 1 if any failed. bigstk.c
 * includes this file to run the same checks with a larger stack. */

#include <errno.h>
#include <stdlib.h>
#include "check.h"

/* The stack linked (option stack), and how much of it the checks use. */
#ifndef STACK
#define STACK 2048
#define DEPTH 1024
#endif

/* The size exhaust allocates until the heap runs out: 80 of them are
 * more than DGROUP holds, or, with far data, than DOS memory. */
#ifdef FAR_DATA
#define BLOCK 30000u
#else
#define BLOCK 1000u
#endif

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
        if (!placed(p[i], sizes[i]))
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

    check("malloc(0) is unique", placed(a, 0) && placed(b, 0) && a != b);
    free(a);
    free(b);
}

/* The largest request with far data, 65,530 bytes (docs/spec/heap.md),
 * and one byte more. */
static void limit(void)
{
    char *p;

    errno = 0;
    p = malloc(65530u);
#ifdef FAR_DATA
    if (p)
        fill(p, 65530u, 3);
    check(DATA "malloc(65530) is placed and keeps its contents",
        placed(p, 65530u) && filled(p, 65530u, 3));
#else
    check(DATA "malloc(65530) fails with ENOMEM", !p && errno == ENOMEM);
#endif
    free(p);
    errno = 0;
    check("malloc(65531) fails with ENOMEM", !malloc(65531u) && errno == ENOMEM);
}

static void exhaust(void)
{
    static void *p[80];
    unsigned n, i, total;
    char *all;

    for (n = 0; n < COUNT(p); n++) {
        errno = 0;
        p[n] = malloc(BLOCK);
        if (!p[n])
            break;
        fill(p[n], BLOCK, (unsigned char)n);
    }
    check("exhaustion returns NULL with ENOMEM", n > 0 && n < COUNT(p) && errno == ENOMEM);
    for (i = 0; i < n; i++)
        if (!placed(p[i], BLOCK) || !filled(p[i], BLOCK, (unsigned char)i))
            break;
    check("exhausted heap keeps contents", i == n);
#ifdef FAR_DATA
    check(DATA "more than 64 KiB in total", n > 0x10000ul / BLOCK);
#else
    check(DATA "the heap ends within DGROUP", n < 0x10000ul / BLOCK);
#endif
    errno = 0;
    check("malloc(65535) fails with ENOMEM", !malloc(65535u) && errno == ENOMEM);
    errno = 0;
    check("malloc(65533) fails with ENOMEM", !malloc(65533u) && errno == ENOMEM);
    free(p[n - 1]);
    p[n - 1] = malloc(BLOCK);
    check("recovery after free", p[n - 1] != NULL);

    /* The total of the blocks that share the first one's segment: all of
     * them in the small model, and with far data those that fit in one
     * segment, which freeing everything must let one request have. */
    total = 0;
    for (i = 0; i < n; i++) {
        if (SEG(p[i]) == SEG(p[0]))
            total += BLOCK;
        free(p[i]);
    }
    all = malloc(total);
    check("one block of the total after freeing all", placed(all, total));
    if (all)
        fill(all, total, 1);
    free(all);
}

int main(void)
{
    check("the stack is all below the heap",
        (unsigned)__libc16_stack_top - (unsigned)&last_static >= STACK);
    distinct();
    reuse();
    zero();
    limit();
    exhaust();
    distinct();
    return failed;
}
