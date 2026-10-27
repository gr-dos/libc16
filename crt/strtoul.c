/* strtoul - libc16 (docs/spec/format.md; C89 7.10.1.6)
 *
 * The spec's Notes keep this from linking atoi or the <ctype.h> table, so
 * it classifies characters itself, in the "C" locale. */

#include <errno.h>
#include <stdlib.h>

/* a * b, 16 by 16 bits: the compiler would call the 32-bit helper. */
static unsigned long mul16(unsigned a, unsigned b);
#pragma aux mul16 = \
    "mul dx" \
    parm [ax] [dx] value [dx ax];

/* The value of c as a digit, 0 to 35, or 36 if it is not one. */
static unsigned digit(unsigned char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    c |= 0x20;                  /* 'A'-'Z' to 'a'-'z'; no other digit maps */
    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;
    return 36;
}

unsigned long __cdecl strtoul(const char *nptr, char **endptr, int base)
{
    const unsigned char *s = (const unsigned char *)nptr;
    const char *end = nptr;     /* until a digit is read: no number */
    unsigned long value = 0;
    unsigned long lo;
    unsigned long hi;
    unsigned d;
    char negative = 0;
    char overflow = 0;

    if (base < 0 || base == 1 || base > 36) {
        errno = EINVAL;
        goto done;
    }
    /* Space, \t, \n, \v, \f and \r: the "C" locale's white space. */
    while (*s == ' ' || (*s >= '\t' && *s <= '\r'))
        s++;
    if (*s == '-') {
        negative = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    /* "0x" is a prefix only when a hexadecimal digit follows it; otherwise
     * the number is the 0 alone and ends at the x (the spec). */
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] | 0x20) == 'x'
        && digit(s[2]) < 16) {
        s += 2;
        base = 16;
    } else if (base == 0) {
        base = *s == '0' ? 8 : 10;
    }
    while ((d = digit(*s)) < (unsigned)base) {
        /* value * base + d, a word at a time; a high word above 16 bits
         * is overflow. */
        lo = mul16((unsigned)value, base) + d;
        hi = mul16((unsigned)(value >> 16), base) + (unsigned)(lo >> 16);
        if (hi > 0xffffU)
            overflow = 1;
        value = hi << 16 | (unsigned)lo;
        s++;
        end = (const char *)s;
    }
    if (overflow) {
        /* ULONG_MAX for a "-" too (the spec). */
        errno = ERANGE;
        value = 0xffffffffUL;
    } else if (negative) {
        value = 0 - value;
    }
done:
    if (endptr)
        *endptr = (char *)end;
    return value;
}
