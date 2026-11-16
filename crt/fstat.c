/* fstat - libc16 (docs/spec/files.md) */

#include <stdio.h>
#include <sys/stat.h>
#include "io16.h"
#include "time16.h"

#define SEEK_FAILED(r) ((unsigned)((r) >> 16) == 0xffff)

/* INT 21h 4400h: the device information in AX */
static unsigned long dos_devinfo(int fd);
#pragma aux dos_devinfo = \
    "mov ax, 4400h" \
    "int 21h" \
    "jc short fail" \
    "mov ax, dx" \
    "fail:" \
    "sbb dx, dx" \
    parm [bx] value [dx ax] modify exact [ax dx];

/* INT 21h 5700h: the date in DX and the time in AX */
static unsigned long dos_filetime(int fd);
#pragma aux dos_filetime = \
    "mov ax, 5700h" \
    "int 21h" \
    "jc short fail" \
    "mov ax, cx" \
    "jmp short done" \
    "fail:" \
    "mov dx, 0ffffh" \
    "done:" \
    parm [bx] value [dx ax] modify exact [ax cx dx];

int __cdecl fstat(int fd, struct stat *buf)
{
    unsigned long r, here, size = 0;
    time_t t = 0;
    unsigned info, mode = S_IFCHR, dev = fd;

    r = dos_devinfo(fd);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    info = DOS_AX(r);
    if (!(info & 0x80)) {
        /* A file: its length, from a seek to the end and back, which
         * leaves a byte kept by read as it was. */
        mode = S_IFREG;
        dev = info & 0x3f;
        here = __libc16_dos_seek(fd, 0L, SEEK_CUR);
        if (SEEK_FAILED(here))
            return __libc16_doserr(DOS_AX(here));
        size = __libc16_dos_seek(fd, 0L, SEEK_END);
        r = __libc16_dos_seek(fd, (long)here, SEEK_SET);
        if (SEEK_FAILED(size))
            r = size;
        if (SEEK_FAILED(r))
            return __libc16_doserr(DOS_AX(r));
        /* The date can be FFFFh only with an error. */
        r = dos_filetime(fd);
        if ((unsigned)(r >> 16) == 0xffff)
            return __libc16_doserr(DOS_AX(r));
        t = __libc16_from_dos((unsigned)(r >> 16), DOS_AX(r));
    }
    buf->st_mode = mode | 0666;
    buf->st_dev = buf->st_rdev = dev;
    buf->st_size = size;
    buf->st_atime = buf->st_mtime = buf->st_ctime = t;
    buf->st_ino = 0;
    buf->st_nlink = 1;
    buf->st_uid = buf->st_gid = 0;
    return 0;
}
