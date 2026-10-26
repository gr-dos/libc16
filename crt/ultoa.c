/* libc16_ultoa - libc16 (docs/spec/format.md) */

#include <libc16.h>

/* *value /= radix, returning the remainder: a radix below 2^16 lets two
 * 32-by-16 divides, high word first, do it without the 32-bit helper. */
static unsigned divide(unsigned long *value, unsigned radix);
#pragma aux divide = \
    "xor dx, dx" \
    "mov ax, 2[bx]" \
    "div cx" \
    "mov 2[bx], ax" \
    "mov ax, [bx]" \
    "div cx" \
    "mov [bx], ax" \
    parm [bx] [cx] value [dx] modify [ax dx];

char * __cdecl libc16_ultoa(unsigned long value, char *buf, int radix)
{
    char *p = buf;
    char *q;
    char c;

    if (radix < 2 || radix > 36) {
        *buf = '\0';
        return buf;
    }
    /* Digits come out lowest first; reverse them afterwards. */
    do {
        c = (char)divide(&value, radix);
        *p++ = c < 10 ? c + '0' : c - 10 + 'a';
    } while (value);
    *p = '\0';
    for (q = buf; q < --p; q++) {
        c = *q;
        *q = *p;
        *p = c;
    }
    return buf;
}
