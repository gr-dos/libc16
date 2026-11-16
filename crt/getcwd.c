/* getcwd - libc16 (docs/spec/files.md) */

#include <direct.h>
#include <errno.h>
#include <stddef.h>
#include "io16.h"

static unsigned char get_drive(void);
#pragma aux get_drive = \
    "mov ah, 19h" \
    "int 21h" \
    value [al] modify exact [ax];

/* INT 21h 47h for the current drive: up to 64 bytes at DS:SI */
static unsigned long dos_getcwd(char *dir);
#pragma aux dos_getcwd = \
    "xor dl, dl" \
    "mov ah, 47h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_SI value [dx ax] modify exact [ax dx];

char * __cdecl getcwd(char *buf, int n)
{
    char path[3 + 64];
    unsigned long r;
    int i;

    if (buf == NULL) {
        errno = EINVAL;
        return NULL;
    }
    path[0] = 'A' + get_drive();
    path[1] = ':';
    path[2] = '\\';
    r = dos_getcwd(path + 3);
    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return NULL;
    }
    /* the path and its terminator, if they fit */
    for (i = 0; path[i] != '\0'; i++)
        ;
    if (i >= n) {
        errno = ERANGE;
        return NULL;
    }
    do
        buf[i] = path[i];
    while (i-- != 0);
    return buf;
}
