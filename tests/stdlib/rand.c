/* rand and srand (docs/spec/stdlib.md). The expected values are the
 * spec's formula, x = 1664525 x + 1013904223 mod 2^32 returning x >> 17,
 * computed on the host. */

#include <stdlib.h>
#include "check.h"

#define N 8

static const int seq1[N] = { 7748, 12100, 16523, 23097, 1656, 12108, 25387, 18225 };
static const int seq0[N] = { 7735, 9128, 26854, 21884, 12585, 20375, 11263, 20972 };
static const int seq12345[N] = { 668, 542, 17798, 20804, 29819, 3685, 16249, 17968 };
static const int seq65535[N] = { 20785, 18016, 5772, 17544, 8201, 27966, 11653, 23736 };

/* The next N values of rand are want */
static int next_are(const int *want)
{
    int i, ok = 1;

    for (i = 0; i < N; i++)
        if (rand() != want[i])
            ok = 0;
    return ok;
}

int main(void)
{
    unsigned bins[8] = { 0 };
    unsigned odd = 0;
    unsigned i;
    int r, inrange = 1;

    CHECK(RAND_MAX == 32767);

    /* before srand, as after srand(1) */
    CHECK(next_are(seq1));
    srand(1);
    CHECK(next_are(seq1));
    srand(0);
    CHECK(next_are(seq0));
    srand(12345);
    CHECK(next_are(seq12345));
    srand(65535);
    CHECK(next_are(seq65535));
    /* the same seed again */
    srand(12345);
    CHECK(next_are(seq12345));
    srand(0);
    CHECK(next_are(seq0));

    /* 10,000 values: in range and spread over it */
    srand(1);
    for (i = 0; i < 10000; i++) {
        r = rand();
        if (r < 0) {                            /* an int is at most 32767 */
            inrange = 0;
            continue;
        }
        bins[r >> 12]++;
        odd += r & 1;
    }
    CHECK(inrange);
    for (i = 0; i < 8; i++)
        CHECK(bins[i] >= 1000 && bins[i] <= 1500);
    CHECK(odd >= 4500 && odd <= 5500);
    return done();
}
