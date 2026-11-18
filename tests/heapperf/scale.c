/* How the heap's time grows with the number of blocks in it
 * (docs/spec/heap.md, "Constraints" and "Performance"), in every model.
 *
 * Each workload is run twice with the same number of operations: n
 * blocks at a time repeated reps times, then 2n blocks at a time repeated
 * reps / 2 times. Each repetition frees everything it allocated, so it
 * starts from the same heap. When an operation takes about the same time
 * however many blocks the heap holds, the two runs take about the same
 * time; when it takes time proportional to them (a search that passes
 * every block, every free block or a block's contents), the second takes
 * twice as long, and the whole workload four times as long for twice the
 * blocks. The check passes when the second run takes at most LIMIT times
 * the first, plus SLACK ticks for the clock's granularity.
 *
 * Time is the BIOS tick count (18.2 per second) of the emulated machine,
 * which counts its instructions, not the host's speed (tests/run-dos runs
 * an 8086 at a fixed rate). Each line prints the workload, its sizes and
 * both counts, which the .expect file matches with a pattern.
 *
 *   scale [WORKLOAD [N [REPS]]]
 *
 * runs one workload with other sizes, to measure (docs/spec/heap.md,
 * "Performance"). */

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "testio.h"

#define LIMIT_NUM 3     /* LIMIT is 3/2 */
#define LIMIT_DEN 2
#define SLACK     2

/* The most blocks a workload keeps: the 2n run of "holes" keeps 4n of
 * the first size, then 2n more. */
#if defined(__COMPACT__) || defined(__LARGE__)
#define MAXBLOCKS 6000
#else
#define MAXBLOCKS 3000
#endif

static void *block[MAXBLOCKS];
static int failed;

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

static int ok(void *p)
{
    if (!p) {
        t_puts("out of memory\r\n");
        failed = 1;
    }
    return p != 0;
}

static void free_all(unsigned n)
{
    unsigned i;

    for (i = 0; i < n; i++) {
        free(block[i]);
        block[i] = 0;
    }
}

/* Many small allocations of varying sizes, as an editor's lines or an
 * interpreter's strings: allocate n, then free them all. */
static void small(unsigned n)
{
    unsigned i;

    for (i = 0; i < n; i++)
        if (!ok(block[i] = malloc(i * 7u % 24 + 1)))
            break;
    free_all(n);
}

/* A fragmented free list: allocate 2n blocks of 8 bytes, free every
 * other one, leaving n holes, then allocate n blocks of 24 bytes, which
 * none of the holes can hold. */
static void holes(unsigned n)
{
    unsigned i;

    for (i = 0; i < 2 * n; i++)
        if (!ok(block[i] = malloc(8)))
            break;
    for (i = 0; i < 2 * n; i += 2) {
        free(block[i]);
        block[i] = 0;
    }
    for (i = 0; i < n; i++)
        if (!ok(block[2 * n + i] = malloc(24)))
            break;
    free_all(3 * n);
}

/* Growing realloc: allocate n blocks of 8 bytes, then grow each to 24 and
 * then to 40. Each block is followed by the next, so it moves and leaves
 * a hole behind. */
static void grow(unsigned n)
{
    unsigned i, size;
    void *p;

    for (i = 0; i < n; i++)
        if (!ok(block[i] = malloc(8)))
            break;
    for (size = 24; size <= 40; size += 16)
        for (i = 0; i < n; i++) {
            if (!ok(p = realloc(block[i], size)))
                break;
            block[i] = p;
        }
    free_all(n);
}

/* One buffer at the end of the heap, grown by 8 bytes n times, as a
 * string read a piece at a time. */
static void append(unsigned n)
{
    unsigned i;
    char *p = 0, *q;

    for (i = 1; i <= n; i++) {
        if (!ok(q = realloc(p, i * 8)))
            break;
        q[i * 8 - 1] = (char)i;
        p = q;
    }
    free(p);
}

/* As an interpreter fills a hash table: for each key an entry and a key
 * string that stay, a temporary that does not, and a bucket array that
 * doubles (by realloc) as the table grows. */
static void table(unsigned n)
{
    unsigned i, buckets = 8, k = 0;
    void **array = malloc(buckets * sizeof *array);
    void *tmp;

    if (!ok(array))
        return;
    for (i = 0; i < n && k + 2 <= MAXBLOCKS; i++) {
        if (!ok(block[k++] = malloc(12)) || !ok(block[k++] = malloc(i % 13 + 4)))
            break;
        if (!ok(tmp = malloc(i % 5 * 8 + 16)))
            break;
        free(tmp);
        if (i == buckets) {
            buckets *= 2;
            if (!ok(tmp = realloc(array, buckets * sizeof *array)))
                break;
            array = tmp;
        }
        array[i % buckets] = block[k - 1];
    }
    free(array);
    free_all(k);
}

struct workload {
    const char *name;
    void (*run)(unsigned n);
    unsigned n, reps;
};

#if defined(__COMPACT__) || defined(__LARGE__)
#define N_SMALL 1000
#define N_HOLES 1000
#define N_TABLE 1000
#else
#define N_SMALL 700
#define N_HOLES 500
#define N_TABLE 600
#endif

static const struct workload workloads[] = {
    { "small", small, N_SMALL, 16 },
    { "holes", holes, N_HOLES, 16 },
    { "grow", grow, 500, 16 },
    { "append", append, 1000, 8 },
    { "table", table, N_TABLE, 16 },
};

#define COUNT(a) (sizeof (a) / sizeof (a)[0])

static unsigned long timed(const struct workload *w, unsigned n, unsigned reps)
{
    unsigned long start = ticks();

    while (reps--)
        w->run(n);
    return ticks() - start;
}

static void measure(const struct workload *w, unsigned n, unsigned reps)
{
    unsigned long t1 = timed(w, n, reps), t2 = timed(w, 2 * n, reps / 2);
    int good = t2 * LIMIT_DEN <= t1 * LIMIT_NUM + SLACK * LIMIT_DEN;

    t_puts(w->name);
    t_puts(" ");
    t_putu(n);
    t_puts("x");
    t_putu(reps);
    t_puts(": ");
    t_putu((unsigned)t1);
    t_puts(" ticks, ");
    t_putu(2 * n);
    t_puts("x");
    t_putu(reps / 2);
    t_puts(": ");
    t_putu((unsigned)t2);
    t_puts(" ticks");
    t_puts(good ? " ok\r\n" : " FAILED\r\n");
    if (!good)
        failed = 1;
}

static unsigned number(const char *s)
{
    unsigned n = 0;

    while (*s >= '0' && *s <= '9')
        n = n * 10 + (*s++ - '0');
    return n;
}

int main(int argc, char **argv)
{
    unsigned i;
    struct workload w;

    for (i = 0; i < COUNT(workloads); i++) {
        w = workloads[i];
        if (argc > 1 && strcmp(argv[1], w.name))
            continue;
        if (argc > 2)
            w.n = number(argv[2]);
        if (argc > 3)
            w.reps = number(argv[3]);
        measure(&w, w.n, w.reps);
    }
    return failed;
}
