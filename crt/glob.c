/* libc16_glob - libc16 (docs/spec/glob.md) */

#include <dos.h>
#include <errno.h>
#include <libc16.h>
#include <stdlib.h>
#include <string.h>

/* The most pointers one block of gl_pathv can hold */
#define MAXSLOTS (0xFFFFu / sizeof(char *))

/* DOS error codes of a search that found nothing */
#define DOS_NOFILE 2
#define DOS_NOMORE 18

static int __cdecl order(const void *a, const void *b)
{
    const char *x = *(char * const *)a;
    const char *y = *(char * const *)b;
    int r = stricmp(x, y);

    return r != 0 ? r : strcmp(x, y);
}

/* Adds the path made of plen bytes of pre and then name; 0 if out of
 * memory, with nothing added. */
static int add(libc16_glob_t *g, const char *pre, unsigned plen, const char *name)
{
    unsigned nlen = strlen(name);
    char *p;

    if (g->gl_pathc + 1 >= g->__gl_slots) {
        unsigned n = g->__gl_slots == 0 ? 8 : g->__gl_slots * 2;
        char **v;

        if (n > MAXSLOTS || n < g->__gl_slots)
            n = MAXSLOTS;
        if (n <= g->gl_pathc + 1) {
            errno = ENOMEM;
            return 0;
        }
        v = realloc(g->gl_pathv, n * sizeof *v);
        if (v == NULL)
            return 0;
        g->gl_pathv = v;
        g->__gl_slots = n;
    }
    p = malloc(plen + nlen + 1);
    if (p == NULL)
        return 0;
    memcpy(p, pre, plen);
    memcpy(p + plen, name, nlen + 1);
    g->gl_pathv[g->gl_pathc++] = p;
    g->gl_pathv[g->gl_pathc] = NULL;
    return 1;
}

/* Whether a directory entry is a path for these flags */
static int wanted(const struct find_t *f, unsigned flags)
{
    if (f->attrib & _A_VOLID)
        return 0;
    if (f->name[0] == '.' && (f->name[1] == '\0' || (f->name[1] == '.' && f->name[2] == '\0')))
        return 0;
    if ((f->attrib & _A_SUBDIR) && !(flags & LIBC16_GLOB_DIRS))
        return 0;
    if ((f->attrib & (_A_HIDDEN | _A_SYSTEM)) && !(flags & LIBC16_GLOB_HIDDEN))
        return 0;
    return 1;
}

int __cdecl libc16_glob(const char *pattern, unsigned flags, libc16_glob_t *g)
{
    struct find_t f;
    unsigned start, plen = 0, len, attr, err;
    int wild = 0, r;

    if (!(flags & LIBC16_GLOB_APPEND)) {
        g->gl_pathc = 0;
        g->gl_pathv = NULL;
        g->__gl_slots = 0;
    }
    start = g->gl_pathc;

    /* plen: up to and including the last \, / or :; wildcards only after */
    for (len = 0; pattern[len] != '\0'; len++) {
        char c = pattern[len];

        if (c == '\\' || c == '/' || c == ':') {
            if (wild) {
                errno = EINVAL;
                return LIBC16_GLOB_ERROR;
            }
            plen = len + 1;
        } else if (c == '*' || c == '?') {
            wild = 1;
        }
    }
    if (!wild) {
        r = LIBC16_GLOB_NOSPACE;
        if (!add(g, pattern, len, ""))
            goto fail;
        return 0;
    }

    attr = 0;
    if (flags & LIBC16_GLOB_DIRS)
        attr |= _A_SUBDIR;
    if (flags & LIBC16_GLOB_HIDDEN)
        attr |= _A_HIDDEN | _A_SYSTEM;
    for (err = _dos_findfirst(pattern, attr, &f); err == 0; err = _dos_findnext(&f))
        if (wanted(&f, flags) && !add(g, pattern, plen, f.name)) {
            r = LIBC16_GLOB_NOSPACE;
            goto fail;
        }
    if (err != DOS_NOFILE && err != DOS_NOMORE) {
        r = LIBC16_GLOB_ERROR;
        goto fail;
    }

    if (g->gl_pathc == start) {
        r = LIBC16_GLOB_NOSPACE;
        if (flags & LIBC16_GLOB_NOCHECK) {
            if (!add(g, pattern, len, ""))
                goto fail;
            return 0;
        }
        errno = ENOENT;
        return LIBC16_GLOB_NOMATCH;
    }
    if (!(flags & LIBC16_GLOB_NOSORT))
        qsort(g->gl_pathv + start, g->gl_pathc - start, sizeof(char *), order);
    return 0;

fail:
    /* errno is the failed call's, whatever free does to it */
    err = errno;
    while (g->gl_pathc > start)
        free(g->gl_pathv[--g->gl_pathc]);
    if (g->gl_pathc == 0) {
        free(g->gl_pathv);
        g->gl_pathv = NULL;
        g->__gl_slots = 0;
    } else {
        g->gl_pathv[g->gl_pathc] = NULL;
    }
    errno = err;
    return r;
}
