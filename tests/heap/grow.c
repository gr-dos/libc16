/* realloc (docs/spec/heap.md), in every model: from a null pointer,
 * growing, moving, shrinking, to 0, and failing with the old block
 * intact. Prints one line per check (check.h). */

#include <errno.h>
#include <stdlib.h>
#include "check.h"

/* As in heap.c: more than DGROUP holds, or, with far data, DOS memory. */
#ifdef FAR_DATA
#define BLOCK 30000u
#else
#define BLOCK 1000u
#endif

static void from_null(void)
{
    char *p;

    errno = 0;
    p = realloc(NULL, 100);
    if (p)
        fill(p, 100, 1);
    check("realloc(NULL, 100) is malloc(100)", placed(p, 100) && filled(p, 100, 1) && errno == 0);
    free(p);
    p = realloc(NULL, 0);
    check("realloc(NULL, 0) is malloc(0)", placed(p, 0));
    free(p);
}

/* a cannot grow where it is while b follows it, so it moves. */
static void grow(void)
{
    char *a = malloc(100), *b = malloc(100), *q, *c;

    fill(a, 100, 5);
    fill(b, 100, 9);
    q = realloc(a, 1000);
    check("realloc grows, keeping the contents", placed(q, 1000) && filled(q, 100, 5));
    if (q)
        fill(q, 1000, 7);
    check("the grown block is writable", q && filled(q, 1000, 7) && filled(b, 100, 9));
    /* Moved, the old block is free for the next request that fits. */
    c = malloc(100);
    check("realloc moves, freeing the old block", q != a && c == a);
    q = realloc(q, 2000);
    check("realloc moves again, keeping the contents", placed(q, 2000) && filled(q, 1000, 7));
    free(c);
    free(q);
    free(b);
}

static void shrink(void)
{
    char *p = malloc(1000), *q, *r;

    if (p)
        fill(p, 1000, 11);
    q = realloc(p, 1000);
    check("realloc to the same size stays", p && q == p && filled(q, 1000, 11));
    q = realloc(p, 10);
    check("realloc shrinks in place, keeping the contents", p && q == p && filled(q, 10, 11));
    /* What it gave back is free: the next request that fits goes there,
     * after the 10 bytes kept and before the old end. */
    r = malloc(500);
    check("shrinking frees the rest",
        r && linear(r) > linear(q) + 10 && linear(r) + 500 <= linear(q) + 1000);
    free(r);
    q = realloc(q, 0);
    r = malloc(0);
    check("realloc(p, 0) keeps a unique block", placed(q, 0) && placed(r, 0) && q != r);
    free(r);
    free(q);
}

static void fail(void)
{
    static void *blocks[80];
    char *p = malloc(100), *q;
    unsigned n, i;

    if (p)
        fill(p, 100, 13);
    errno = 0;
    q = realloc(p, 65535u);
    check("realloc(p, 65535) fails with ENOMEM", p && !q && errno == ENOMEM);
    check("failing keeps the old block", filled(p, 100, 13));

    /* With the heap exhausted, growing p fails, and p is still there. */
    for (n = 0; n < COUNT(blocks); n++)
        if (!(blocks[n] = malloc(BLOCK)))
            break;
    errno = 0;
    q = realloc(p, BLOCK);
    check("realloc fails with ENOMEM when the heap is full",
        n > 0 && n < COUNT(blocks) && !q && errno == ENOMEM);
    check("failing keeps the old block", filled(p, 100, 13));
    for (i = 0; i < n; i++)
        free(blocks[i]);
    q = realloc(p, BLOCK);
    check("realloc works after free, keeping the contents", placed(q, BLOCK) && filled(q, 100, 13));
    free(q);
}

int main(void)
{
    from_null();
    grow();
    shrink();
    fail();
    return failed;
}
