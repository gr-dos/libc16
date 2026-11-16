/* qsort (docs/spec/stdlib.md): orders, permutations, the pointers compar
 * sees, and the number of calls of compar against 3 n ceil(log2 n) */

#include <stdlib.h>
#include <string.h>
#include "check.h"

#define N 1000

/* What the comparison functions check: the array being sorted, and the
 * calls so far. Any pointer outside it, or not at an element, is
 * counted in strays. */
static const char *cur_base;
static unsigned cur_n, cur_size;
static unsigned long calls, strays;
static unsigned seed;

static void in_array(const void *p)
{
    unsigned off = (unsigned)((const char *)p - cur_base);

    if ((const char *)p < cur_base || off >= cur_n * cur_size || off % cur_size != 0)
        strays++;
}

static unsigned rnd(void)
{
    seed = seed * 25173u + 13849u;
    return seed >> 1;
}

/* 3 n ceil(log2 n) */
static unsigned long bound(unsigned n)
{
    unsigned lg = 0;

    while ((1u << lg) < n)
        lg++;
    return 3ul * n * (lg ? lg : 1);
}

static void begin(void *base, unsigned n, unsigned size)
{
    cur_base = base;
    cur_n = n;
    cur_size = size;
    calls = strays = 0;
}

/* ints */

static int ints[N];

static int __cdecl cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;

    calls++;
    in_array(a);
    in_array(b);
    return x < y ? -1 : x > y;
}

static int __cdecl cmp_random(const void *a, const void *b)
{
    calls++;
    in_array(a);
    in_array(b);
    return (int)(rnd() % 3) - 1;
}

/* Sum and sum of squares, mod 2^32: equal for a permutation */
static void sums(unsigned n, unsigned long *s1, unsigned long *s2)
{
    unsigned i;

    *s1 = *s2 = 0;
    for (i = 0; i < n; i++) {
        *s1 += (unsigned)ints[i];
        *s2 += (unsigned long)(unsigned)ints[i] * (unsigned)ints[i];
    }
}

enum { SORTED, REVERSED, EQUAL, FEW, RANDOM, PIPE, PATTERNS };

static void fill(unsigned n, int pattern)
{
    unsigned i;

    for (i = 0; i < n; i++) {
        switch (pattern) {
        case SORTED:   ints[i] = (int)i - 500; break;
        case REVERSED: ints[i] = 500 - (int)i; break;
        case EQUAL:    ints[i] = 7; break;
        case FEW:      ints[i] = (int)(i % 3); break;
        case RANDOM:   ints[i] = (int)rnd(); break;
        default:       ints[i] = (int)(i < n / 2 ? i : n - i); break;
        }
    }
}

static void sort_ints(unsigned n, int pattern)
{
    unsigned long s1, s2, t1, t2;
    unsigned i;
    int ordered = 1;

    fill(n, pattern);
    sums(n, &s1, &s2);
    begin(ints, n, sizeof ints[0]);
    qsort(ints, n, sizeof ints[0], cmp_int);
    for (i = 1; i < n; i++)
        if (ints[i - 1] > ints[i])
            ordered = 0;
    sums(n, &t1, &t2);
    CHECK(ordered && s1 == t1 && s2 == t2);
    CHECK(strays == 0);
    CHECK(n < 2 ? calls == 0 : calls <= bound(n));
    if (!(ordered && strays == 0 && (n < 2 || calls <= bound(n)))) {
        t_puts("ints: n ");
        t_putu(n);
        t_puts(" pattern ");
        t_putu(pattern);
        t_puts("\r\n");
    }
}

/* 7-byte records: a key, the record's original index, and a byte that
 * depends on the index, so that a record moved in pieces is seen. */

struct rec {
    unsigned char key[2];
    unsigned char tag[2];
    unsigned char check[3];
};

static struct rec recs[N];
static unsigned char seen[N / 8 + 1];

static unsigned key_of(const struct rec *r)
{
    return r->key[0] << 8 | r->key[1];
}

static unsigned tag_of(const struct rec *r)
{
    return r->tag[0] | r->tag[1] << 8;
}

static int __cdecl cmp_rec(const void *a, const void *b)
{
    unsigned x = key_of(a), y = key_of(b);

    calls++;
    in_array(a);
    in_array(b);
    return x < y ? -1 : x > y;
}

static void sort_recs(unsigned n, unsigned keys)
{
    unsigned i, t;
    int ok = 1;

    for (i = 0; i < n; i++) {
        t = rnd() % keys;
        recs[i].key[0] = (unsigned char)(t >> 8);
        recs[i].key[1] = (unsigned char)t;
        recs[i].tag[0] = (unsigned char)i;
        recs[i].tag[1] = (unsigned char)(i >> 8);
        recs[i].check[0] = (unsigned char)(i * 3);
        recs[i].check[1] = (unsigned char)(i * 5 + 1);
        recs[i].check[2] = (unsigned char)(i * 7 + 2);
    }
    begin(recs, n, sizeof recs[0]);
    qsort(recs, n, sizeof recs[0], cmp_rec);
    memset(seen, 0, sizeof seen);
    for (i = 0; i < n; i++) {
        t = tag_of(&recs[i]);
        if (t >= n || seen[t / 8] & 1 << t % 8)
            ok = 0;
        else
            seen[t / 8] |= 1 << t % 8;
        if (recs[i].check[0] != (unsigned char)(t * 3) || recs[i].check[1] != (unsigned char)(t * 5 + 1)
            || recs[i].check[2] != (unsigned char)(t * 7 + 2))
            ok = 0;
        if (i > 0 && key_of(&recs[i - 1]) > key_of(&recs[i]))
            ok = 0;
    }
    CHECK(sizeof (struct rec) == 7);
    CHECK(ok && strays == 0 && calls <= bound(n));
}

/* 300-byte records, outside DGROUP with far data */

#define BIG 300
#define NBIG 50

struct big {
    unsigned key;
    unsigned char fill[BIG - 2 * sizeof (unsigned)];
    unsigned tag;
};

#if defined(__COMPACT__) || defined(__LARGE__)
static struct big __far bigs[NBIG];
#else
static struct big bigs[NBIG];
#endif

static int __cdecl cmp_big(const void *a, const void *b)
{
    const struct big *x = a, *y = b;

    calls++;
    in_array(a);
    in_array(b);
    return x->key < y->key ? -1 : x->key > y->key;
}

static void sort_bigs(void)
{
    unsigned i, j;
    int ok = 1;
    unsigned long tags = 0;

    for (i = 0; i < NBIG; i++) {
        bigs[i].key = rnd() % 20;
        bigs[i].tag = i;
        for (j = 0; j < sizeof bigs[i].fill; j++)
            bigs[i].fill[j] = (unsigned char)(i + j);
    }
    begin(bigs, NBIG, sizeof bigs[0]);
    qsort(bigs, NBIG, sizeof bigs[0], cmp_big);
    for (i = 0; i < NBIG; i++) {
        tags += 1ul << (bigs[i].tag % 32) ^ bigs[i].tag;
        for (j = 0; j < sizeof bigs[i].fill; j++)
            if (bigs[i].fill[j] != (unsigned char)(bigs[i].tag + j))
                ok = 0;
        if (i > 0 && bigs[i - 1].key > bigs[i].key)
            ok = 0;
    }
    for (i = 0; i < NBIG; i++)
        tags -= 1ul << (i % 32) ^ i;
    CHECK(sizeof (struct big) == BIG);
    CHECK(ok && tags == 0 && strays == 0 && calls <= bound(NBIG));
}

int main(void)
{
    static const unsigned sizes[] = { 2, 3, 5, 16, 17, 100, N };
    unsigned i;
    int p;

    seed = 1;

    /* nothing to sort: compar is not called */
    ints[0] = 2;
    ints[1] = 1;
    begin(ints, 2, sizeof ints[0]);
    qsort(ints, 0, sizeof ints[0], cmp_int);
    qsort(ints, 1, sizeof ints[0], cmp_int);
    qsort(ints, 2, 0, cmp_int);
    CHECK(calls == 0 && ints[0] == 2 && ints[1] == 1);

    for (i = 0; i < sizeof sizes / sizeof sizes[0]; i++)
        for (p = 0; p < PATTERNS; p++)
            sort_ints(sizes[i], p);

    sort_recs(N, 65535u);
    sort_recs(N, 4);
    sort_recs(7, 3);
    sort_bigs();

    /* an inconsistent compar: qsort returns, with a permutation, having
     * touched only the array */
    for (i = 0; i < 4; i++) {
        unsigned long s1, s2, t1, t2;

        fill(N, RANDOM);
        sums(N, &s1, &s2);
        begin(ints, N, sizeof ints[0]);
        qsort(ints, N, sizeof ints[0], cmp_random);
        sums(N, &t1, &t2);
        CHECK(s1 == t1 && s2 == t2 && strays == 0);
    }
    return done();
}
