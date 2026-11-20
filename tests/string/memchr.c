/* memchr (docs/spec/string.md) */

#include <string.h>
#include "check.h"

#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data, bytes outside DGROUP */
static char __far far_s[] = "far\0data";
#endif

/* More than 32767 bytes would not fit; the match ends the search first. */
static unsigned char big[1000];

int main(void)
{
    static const char s[] = "hello, world";
    /* binary data: embedded nulls and bytes from 80h up */
    static const unsigned char bin[] = { 0x00, 0x41, 0x00, 0x80, 0xff, 0x7f, 0xff, 0x00 };

    /* the first, a middle and the final byte */
    CHECK(memchr(s, 'h', 12) == s);
    CHECK(memchr(s, 'o', 12) == s + 4);
    CHECK(memchr(s, 'd', 12) == s + 11);
    /* missing */
    CHECK(memchr(s, 'z', 12) == NULL);
    /* n of 0 */
    CHECK(memchr(s, 'h', 0) == NULL);
    /* only the first n bytes: a match just past them is not found */
    CHECK(memchr(s, 'd', 11) == NULL);
    CHECK(memchr(s, ',', 5) == NULL);
    CHECK(memchr(s, ',', 6) == s + 5);
    /* not a string: the terminator is a byte like any other, and the
     * search goes past embedded nulls */
    CHECK(memchr(s, '\0', 13) == s + 12);
    CHECK(memchr(s, '\0', 12) == NULL);
    CHECK(memchr(bin, 0x00, sizeof bin) == bin);
    CHECK(memchr(bin + 1, 0x00, sizeof bin - 1) == bin + 2);
    CHECK(memchr(bin, 0x41, sizeof bin) == bin + 1);
    CHECK(memchr(bin, 0x80, sizeof bin) == bin + 3);
    CHECK(memchr(bin, 0xff, sizeof bin) == bin + 4);
    CHECK(memchr(bin, 0x7f, sizeof bin) == bin + 5);
    /* c is converted to unsigned char */
    CHECK(memchr(bin, -1, sizeof bin) == bin + 4);
    CHECK(memchr(bin, -128, sizeof bin) == bin + 3);
    CHECK(memchr(bin, 0x141, sizeof bin) == bin + 1);
    CHECK(memchr(bin, 0x7f00, sizeof bin) == bin);
    CHECK(memchr(s, 'o' - 256, 12) == s + 4);
    CHECK(memchr(bin, 0x42, sizeof bin) == NULL);
    /* an unsigned size_t: n above 32767, with offsets past 255 */
    big[600] = 1;
    big[999] = 2;
    CHECK(memchr(big, 1, 40000u) == big + 600);
    CHECK(memchr(big, 2, sizeof big) == big + 999);
    CHECK(memchr(big, 2, sizeof big - 1) == NULL);
    CHECK(memchr(big, 0, 0xffffu) == big);

#if defined(__COMPACT__) || defined(__LARGE__)
    CHECK(memchr(far_s, 'd', sizeof far_s) == far_s + 4);
    CHECK(memchr(far_s, 'd', 4) == NULL);
#endif
    return done();
}
