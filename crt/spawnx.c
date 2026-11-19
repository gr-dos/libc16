/* spawnx - libc16 (docs/spec/process.md): the spawn functions' core */

#include <errno.h>
#include <process.h>
#include "io16.h"
#include "proc16.h"
#include "stdio16.h"

int __libc16_spawnext(const char *name, unsigned env, const char *tail)
{
    char buf[EXEC_NAME_MAX];
    const char *s;
    char *d;
    int r;
    char c;

    /* a . in the last component: the name as it is */
    for (s = name, c = 0; *s != '\0'; s++) {
        if (*s == '\\' || *s == '/' || *s == ':')
            c = 0;
        else if (*s == '.')
            c = 1;
    }
    if (c)
        return __libc16_dosexec(name, env, tail);
    if (s - name > EXEC_NAME_MAX - 5)
        return NOT_FOUND;
    for (d = buf; (*d = *name) != '\0'; d++, name++)
        ;
    d[0] = '.';
    d[1] = 'C';
    d[2] = 'O';
    d[3] = 'M';
    d[4] = '\0';
    r = __libc16_dosexec(buf, env, tail);
    if (r == NOT_FOUND) {
        d[1] = 'E';
        d[2] = 'X';
        d[3] = 'E';
        r = __libc16_dosexec(buf, env, tail);
    }
    return r;
}

#define PUT(ch) do { if (d == end) goto big; *d++ = (ch); } while (0)

int __libc16_spawnx(int mode, const char *path, char * const *argv,
    char * const *envp, __libc16_try try)
{
    char tail[TAIL_MAX + 2];
    char *d, *end;
    const char *s, *p;
    unsigned seg;
    int r, quote, bs;
    char c;

    if (mode != P_WAIT || argv == NULL || *argv == NULL) {
        errno = EINVAL;
        return -1;
    }

    /* the command tail */
    d = tail + 1;
    end = d + TAIL_MAX;
    while ((s = *++argv) != NULL) {
        quote = *s == '\0';
        for (p = s; (c = *p) != '\0'; p++)
            if (c == ' ' || c == '\t' || c == '"')
                quote = 1;
        PUT(' ');
        if (quote)
            PUT('"');
        for (bs = 0; ; bs = c == '\\' ? bs + 1 : 0) {
            c = *s++;
            if (quote && (c == '"' || c == '\0')) {
                /* the backslashes before it doubled, one more before a
                 * quote; then the quote */
                if (c != '\0')
                    bs++;
                for (; bs > 0; bs--)
                    PUT('\\');
                PUT('"');
            } else if (c != '\0')
                PUT(c);
            if (c == '\0')
                break;
        }
    }
    *d = '\r';
    tail[0] = (char)(d - tail - 1);

    /* the environment block */
    seg = 0;
    if (envp != NULL && (seg = __libc16_envblock(envp)) == 0)
        return -1;

    /* Buffered output before the child's, which shares the handles; a
     * child must not run without it. */
    if (FLUSH_FAILED(seg))
        return -1;
    r = try(path, seg, tail);
    if (seg != 0)
        __libc16_dosfree(seg);
    if (r < 0)
        return __libc16_doserr(-r);
    return r;

big:
    errno = E2BIG;
    return -1;
}
