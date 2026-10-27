/* atoi - libc16 (docs/spec/format.md; C89 7.10.1.2)
 *
 * The spec's Notes keep this from linking strtoul or the <ctype.h> table,
 * so it reads the number itself. The result is the low 16 bits of the
 * value read modulo 2^32, which is the value modulo 2^16: unsigned 16-bit
 * arithmetic gives it directly, overflow included. */

#include <stdlib.h>

int __cdecl atoi(const char *nptr)
{
    const unsigned char *s = (const unsigned char *)nptr;
    unsigned value = 0;
    char negative = 0;

    /* Space, \t, \n, \v, \f and \r: the "C" locale's white space. */
    while (*s == ' ' || (*s >= '\t' && *s <= '\r'))
        s++;
    if (*s == '-') {
        negative = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9')
        value = value * 10 + (*s++ - '0');
    if (negative)
        value = 0 - value;
    return (int)value;
}
