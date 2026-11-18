/* malloc, realloc and free in a long pseudo-random sequence, in every
 * model (docs/spec/heap.md): every block is placed, overlaps no other
 * live block and keeps its contents through other calls and through
 * realloc; and after everything is freed, the free blocks have all
 * merged, so that one request as large as the span the blocks covered
 * gets the lowest address any of them had. Prints one line per check
 * (check.h). */

#include <errno.h>
#include <stdlib.h>
#include "check.h"

#ifdef FAR_DATA
#define SLOTS 96
#else
#define SLOTS 48
#endif
#define OPS 3000

static unsigned char *slot[SLOTS];
static unsigned size[SLOTS];
static unsigned char seed[SLOTS];

static unsigned long state = 1;

static unsigned rnd(void)
{
    state = state * 1103515245ul + 12345;
    return (unsigned)(state >> 16) & 0x7FFF;
}

/* Mostly small sizes, as an interpreter's strings, and some larger. */
static unsigned random_size(void)
{
    return rnd() % 8 ? rnd() % 64 : rnd() % 400;
}

/* The lowest address a block had, and the highest end of any block in
 * the same segment. */
static unsigned long lowest = 0xFFFFFFFFul, highest;
#ifdef FAR_DATA
static unsigned lowest_seg;
#endif

/* Slot i's block lies inside one segment, overlaps no other slot's and
 * moves the lowest address and highest end. */
static int alone(unsigned i)
{
    unsigned long a = linear(slot[i]), b;
    unsigned j;

    if (!placed(slot[i], size[i]))
        return 0;
    for (j = 0; j < SLOTS; j++) {
        if (j == i || !slot[j])
            continue;
        b = linear(slot[j]);
        if (a < b + size[j] && b < a + size[i])
            return 0;
        /* Two blocks of no bytes still differ. */
        if (a == b)
            return 0;
    }
    if (a < lowest) {
        lowest = a;
#ifdef FAR_DATA
        lowest_seg = FP_SEG(slot[i]);
#endif
    }
#ifdef FAR_DATA
    if (FP_SEG(slot[i]) == lowest_seg && a + size[i] > highest)
#else
    if (a + size[i] > highest)
#endif
        highest = a + size[i];
    return 1;
}

int main(void)
{
    unsigned op, i, n, keep;
    int ok = 1, kept = 1, fails = 0;
    unsigned char *p;

    for (op = 0; op < OPS && ok; op++) {
        i = rnd() % SLOTS;
        if (!slot[i]) {
            size[i] = random_size();
            seed[i] = (unsigned char)rnd();
            errno = 0;
            slot[i] = malloc(size[i]);
            if (!slot[i]) {
                fails++;
                continue;
            }
            ok = alone(i);
            fill(slot[i], size[i], seed[i]);
        } else if (rnd() % 3 == 0) {
            n = random_size();
            p = realloc(slot[i], n);
            if (!p) {
                fails++;
                continue;
            }
            keep = n < size[i] ? n : size[i];
            if (!filled(p, keep, seed[i]))
                kept = 0;
            slot[i] = p;
            size[i] = n;
            ok = alone(i);
            seed[i] = (unsigned char)rnd();
            fill(p, n, seed[i]);
        } else {
            if (!filled(slot[i], size[i], seed[i]))
                kept = 0;
            free(slot[i]);
            slot[i] = 0;
        }
    }
    check("random blocks are placed and do not overlap", ok);
    check("random blocks keep their contents", kept);
    check("no random request fails", fails == 0);
    for (i = 0; i < SLOTS; i++) {
        if (slot[i] && !filled(slot[i], size[i], seed[i]))
            kept = 0;
        free(slot[i]);
    }
    check("every block keeps its contents until freed", kept);
    n = (unsigned)(highest - lowest) - 4;
    p = malloc(n);
    check("the freed blocks merge into one at the lowest address",
        placed(p, n) && linear(p) <= lowest
#ifdef FAR_DATA
        && FP_SEG(p) == lowest_seg
#endif
        );
    free(p);
    return failed;
}
