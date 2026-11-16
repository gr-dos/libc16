/* rename - libc16 (docs/spec/files.md) */

#include <dos.h>
#include <errno.h>
#include <stdio.h>
#include "io16.h"

static unsigned char get_drive(void);
#pragma aux get_drive = \
    "mov ah, 19h" \
    "int 21h" \
    value [al] modify exact [ax];

/* INT 21h 56h: DS:DX the old name, ES:DI the new one */
static unsigned long dos_rename(const char *oldname, const char *newname);
#if defined(__COMPACT__) || defined(__LARGE__)
#pragma aux dos_rename = \
    "mov ah, 56h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [ds dx] [es di] value [dx ax] modify exact [ax dx];
#else
#pragma aux dos_rename = \
    "push ds" \
    "pop es" \
    "mov ah, 56h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [dx] [di] value [dx ax] modify exact [ax dx es];
#endif

/* Whether name has a wildcard, which DOS would apply to several files */
static int wild(const char *name)
{
    char c;

    while ((c = *name++) != '\0')
        if (c == '*' || c == '?')
            return 1;
    return 0;
}

/* The drive of name, 0 for A: */
static unsigned drive(const char *name)
{
    if (name[0] != '\0' && name[1] == ':')
        return ((unsigned char)name[0] | 0x20) - 'a';
    return get_drive();
}

/* Whether the directory name is in is missing. A name with no directory,
 * or in a root directory, is in one that exists. */
static int no_dir(const char *name)
{
    char dir[FILENAME_MAX];
    unsigned i, end = 0;
    unsigned long r;
    char c;

    for (i = 0; i < sizeof dir && (c = name[i]) != '\0'; i++) {
        dir[i] = c;
        if (c == '\\' || c == '/')
            end = i;
    }
    if (end == 0 || (end == 2 && name[1] == ':'))
        return 0;
    dir[end] = '\0';
    r = __libc16_dos_getattr(dir);
    return DOS_FAILED(r) || !(DOS_AX(r) & _A_SUBDIR);
}

int __cdecl rename(const char *oldname, const char *newname)
{
    unsigned long r;

    if (wild(oldname) || wild(newname)) {
        errno = ENOENT;
        return -1;
    }
    if (drive(oldname) != drive(newname)) {
        errno = EXDEV;
        return -1;
    }
    r = dos_rename(oldname, newname);
    if (DOS_FAILED(r)) {
        /* DOS gives access denied (5) when newname exists, and DOSBox-X
         * also when its directory does not. */
        if (DOS_AX(r) == 5 && no_dir(newname)) {
            errno = ENOENT;
            return -1;
        }
        return __libc16_doserr(DOS_AX(r));
    }
    return 0;
}
