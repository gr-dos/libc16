/* libc16_ltoa - libc16 (docs/spec/format.md)
 *
 * A program that uses only this links only this (the spec's Notes), so it
 * has its own copy of libc16_ultoa's digit loop rather than calling it. */

#include <libc16.h>

/* *value /= radix, returning the remainder: a radix below 2^16 lets two
 * 32-by-16 divides, high word first, do it without the 32-bit helper. */
#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data DS may hold buf's segment by now. *value is always a
 * local, so in DGROUP, which SS holds (docs/ABI.md, "Memory models"): pass
 * its offset and reach it through SS. */
static unsigned divide_ss(unsigned long __near *value, unsigned radix);
#define divide(value, radix) divide_ss((unsigned long __near *)(value), radix)
#pragma aux divide_ss = \
    "xor dx, dx" \
    "mov ax, ss:2[bx]" \
    "div cx" \
    "mov ss:2[bx], ax" \
    "mov ax, ss:[bx]" \
    "div cx" \
    "mov ss:[bx], ax" \
    parm [bx] [cx] value [dx] modify [ax dx];
#else
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
#endif

char * __cdecl libc16_ltoa(long value, char *buf, int radix)
{
    unsigned long magnitude = value;
    char *p = buf;
    char *q;
    char c;

    if (radix < 2 || radix > 36) {
        *buf = '\0';
        return buf;
    }
    if (value < 0) {
        /* Modulo 2^32, so LONG_MIN's magnitude is right too. */
        magnitude = 0 - magnitude;
        *p++ = '-';
    }
    q = p;
    /* Digits come out lowest first; reverse them afterwards. */
    do {
        c = (char)divide(&magnitude, radix);
        *p++ = c < 10 ? c + '0' : c - 10 + 'a';
    } while (magnitude);
    *p = '\0';
    for (; q < --p; q++) {
        c = *q;
        *q = *p;
        *p = c;
    }
    return buf;
}
