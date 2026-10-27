/* calloc (docs/spec/heap.md), in every model: the block is zeroed even
 * where malloc left other contents, n * size overflowing size_t fails
 * with ENOMEM, and a zero count or size is malloc(0). Prints one line
 * per check (check.h). */

#include <errno.h>
#include <stdlib.h>
#include "check.h"

static int zeroed(const void *p, unsigned n)
{
    const unsigned char *b = p;

    while (n--)
        if (*b++)
            return 0;
    return 1;
}

static void zeroing(void)
{
    char *p = malloc(3000), *q;
    int dirty;

    /* Dirty the memory calloc will be given. */
    if (p)
        fill(p, 3000, 1);
    dirty = p && filled(p, 3000, 1);
    free(p);
    q = calloc(100, 30);
    check("calloc(100, 30) is zeroed", dirty && q == p && placed(q, 3000) && zeroed(q, 3000));
    free(q);
    q = calloc(1, 1);
    check("calloc(1, 1) is zeroed", placed(q, 1) && zeroed(q, 1));
    free(q);
}

static void zero(void)
{
    void *a = calloc(0, 10), *b = calloc(10, 0);

    check("calloc of 0 bytes is unique", placed(a, 0) && placed(b, 0) && a != b);
    free(a);
    free(b);
}

static void overflow(void)
{
    errno = 0;
    check("calloc(32768, 2) overflows with ENOMEM", !calloc(32768u, 2) && errno == ENOMEM);
    errno = 0;
    check("calloc(3, 21846) overflows with ENOMEM", !calloc(3, 21846u) && errno == ENOMEM);
    errno = 0;
    check("calloc(65535, 65535) overflows with ENOMEM", !calloc(65535u, 65535u) && errno == ENOMEM);
    errno = 0;
    check("calloc(1, 65535) fails with ENOMEM", !calloc(1, 65535u) && errno == ENOMEM);
}

int main(void)
{
    zeroing();
    zero();
    overflow();
    return failed;
}
