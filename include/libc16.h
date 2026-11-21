/* libc16.h - libc16's own helpers (docs/spec/format.md, docs/spec/glob.md) */

#ifndef _LIBC16_LIBC16_H
#define _LIBC16_LIBC16_H

#include <_libc16.h>

char * __cdecl libc16_ltoa(long value, char *buf, int radix);
char * __cdecl libc16_ultoa(unsigned long value, char *buf, int radix);

/* Wildcard expansion of file operands (docs/spec/glob.md) */
typedef struct {
    unsigned gl_pathc;          /* paths in gl_pathv */
    char **gl_pathv;            /* gl_pathc paths, then a null pointer */
    unsigned __gl_slots;        /* private: pointers allocated in gl_pathv */
} libc16_glob_t;

/* flags */
#define LIBC16_GLOB_APPEND  0x01    /* add to the paths already in the glob */
#define LIBC16_GLOB_NOCHECK 0x02    /* no match gives the pattern itself */
#define LIBC16_GLOB_NOSORT  0x04    /* DOS's order, not sorted */
#define LIBC16_GLOB_DIRS    0x08    /* directories match too */
#define LIBC16_GLOB_HIDDEN  0x10    /* Hidden and System entries match too */

/* results other than 0 */
#define LIBC16_GLOB_NOMATCH 1       /* the pattern matched nothing */
#define LIBC16_GLOB_NOSPACE 2       /* out of memory */
#define LIBC16_GLOB_ERROR   3       /* a bad pattern, drive or directory */

int  __cdecl libc16_glob(const char *pattern, unsigned flags, libc16_glob_t *g);
void __cdecl libc16_globfree(libc16_glob_t *g);

#endif
