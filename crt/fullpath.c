/* _fullpath - libc16 (docs/spec/dos.md) */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "io16.h"

#define WORK_MAX 256

static unsigned char get_drive(void);
#pragma aux get_drive = \
    "mov ah, 19h" \
    "int 21h" \
    value [al] modify exact [ax];

/* The current directory of drive (1 = A:) in the 64 bytes at dir, without
 * a drive or leading backslash */
static unsigned long get_cwd(unsigned char drive, char *dir);
#pragma aux get_cwd = \
    "mov ah, 47h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [dl] [si] value [dx ax] modify exact [ax dx];

static int is_sep(char c)
{
    return c == '\\' || c == '/';
}

/* Appends the components of p to the path in work, which ends at *end
 * after the root "D:\"; returns 0, or -1 if it would not fit. */
static int add(char *work, char **end, const char *p)
{
    char *e = *end;
    const char *c;
    unsigned n;

    for (;;) {
        while (is_sep(*p))
            p++;
        if (*p == '\0')
            break;
        for (c = p; *p != '\0' && !is_sep(*p); p++)
            ;
        n = p - c;
        if (n == 1 && c[0] == '.')
            continue;
        if (n == 2 && c[0] == '.' && c[1] == '.') {
            while (e > work + 3 && *--e != '\\')
                ;
            continue;
        }
        if ((e - work) + (e > work + 3) + n >= WORK_MAX)
            return -1;
        if (e > work + 3)
            *e++ = '\\';
        memcpy(e, c, n);
        e += n;
    }
    *end = e;
    return 0;
}

char * __cdecl _fullpath(char *buf, const char *path, size_t size)
{
    char work[WORK_MAX];
    char cwd[64];
    char *end = work + 3;
    unsigned char drive;
    unsigned long r;

    if (buf == NULL || path == NULL || *path == '\0') {
        errno = EINVAL;
        return NULL;
    }
    if (path[0] != '\0' && path[1] == ':') {
        drive = path[0] | 0x20;
        if (drive < 'a' || drive > 'z') {
            errno = ENOENT;
            return NULL;
        }
        drive -= 'a';
        path += 2;
    } else {
        drive = get_drive();
    }
    work[0] = 'A' + drive;
    work[1] = ':';
    work[2] = '\\';
    /* the current directory, which also checks the drive */
    r = get_cwd(drive + 1, cwd);
    if (DOS_FAILED(r)) {
        __libc16_doserr(DOS_AX(r));
        return NULL;
    }
    if (!is_sep(*path))
        add(work, &end, cwd);
    if (add(work, &end, path) != 0 || (size_t)(end - work) + 1 > size) {
        errno = ERANGE;
        return NULL;
    }
    *end = '\0';
    return memcpy(buf, work, end - work + 1);
}
