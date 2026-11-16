/* strtol - libc16 (docs/spec/format.md; C89 7.10.1.5)
 *
 * strtoul reads the same subject sequence, so it does the reading; this
 * only finds the sign and limits the result. */

#include <errno.h>
#include <stdlib.h>

long __cdecl strtol(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long value, limit = 0x7fffffffUL;
    int saved = errno;

    /* the "C" locale's white space, as strtoul skips it */
    while (*s == ' ' || (*s >= '\t' && *s <= '\r'))
        s++;
    errno = 0;
    value = strtoul(nptr, endptr, base);
    if (*s == '-') {
        /* strtoul negated the magnitude */
        value = 0 - value;
        limit = 0x80000000UL;
    }
    /* ERANGE here is strtoul's overflow; LONG_MAX, or LONG_MIN for a -,
     * is the limit's bit pattern. */
    if (errno == ERANGE || value > limit) {
        errno = ERANGE;
        return (long)limit;
    }
    if (errno == 0)
        errno = saved;
    return *s == '-' ? (long)(0 - value) : (long)value;
}
