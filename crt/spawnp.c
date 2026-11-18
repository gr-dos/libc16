/* spawnp - libc16 (docs/spec/process.md): the PATH search */

#include <stdlib.h>
#include "proc16.h"

int __libc16_spawnp(const char *name, unsigned env, const char *tail)
{
    char buf[EXEC_NAME_MAX];
    char *end = buf + EXEC_NAME_MAX;
    const char *s, *p;
    char *d;
    int r;
    char c;

    r = __libc16_spawnext(name, env, tail);
    if (r != NOT_FOUND)
        return r;
    for (s = name; (c = *s) != '\0'; s++)
        if (c == '\\' || c == '/' || c == ':')
            return r;
    if ((s = getenv("PATH")) == NULL)
        return r;
    for (;;) {
        /* the next element, then \ unless it ends in one, then name;
         * one that does not fit is skipped */
        for (d = buf; (c = *s) != '\0' && c != ';'; s++)
            if (d < end)
                *d++ = c;
        if (d != buf) {
            if (c = d[-1], d < end && c != '\\' && c != '/' && c != ':')
                *d++ = '\\';
            for (p = name; d < end && (*d = *p) != '\0'; d++, p++)
                ;
            if (d < end) {
                r = __libc16_spawnext(buf, env, tail);
                if (r != NOT_FOUND)
                    return r;
            }
        }
        if (*s++ == '\0')
            return r;
    }
}
