/* qsort - libc16 (docs/spec/stdlib.md)
 *
 * Heapsort: at most about 2 n log2 n comparisons for any input, no
 * recursion and no heap. Elements are exchanged in place a byte at a
 * time. */

#include <stdlib.h>

static void swap(char *a, char *b, unsigned size)
{
    char t;

    do {
        t = *a;
        *a++ = *b;
        *b++ = t;
    } while (--size != 0);
}

void __cdecl qsort(void *base, size_t nmemb, size_t size,
                   int (__cdecl *compar)(const void *, const void *))
{
    char *b = base;
    unsigned n = nmemb, i, r, c;

    if (n < 2 || size == 0)
        return;
    /* Build a heap with the greatest element at the root, sifting down
     * each element that has a child from the last up; then repeatedly
     * move the root to the end and sift the element that replaced it. */
    i = n / 2;
    for (;;) {
        if (i != 0) {
            i--;
        } else {
            if (--n == 0)
                return;
            swap(b, b + n * size, size);
        }
        /* Sift down from i in the heap of n elements. While r has a
         * child (r < n / 2, so 2 r + 2 cannot overflow), move it below
         * its greater child if that child is greater. */
        for (r = i; r < n / 2; r = c) {
            c = 2 * r + 1;
            if (c + 1 < n && compar(b + c * size, b + (c + 1) * size) < 0)
                c++;
            if (compar(b + r * size, b + c * size) >= 0)
                break;
            swap(b + r * size, b + c * size, size);
        }
    }
}
