/* utime - libc16 (docs/spec/files.md) */

#include <errno.h>
#include <sys/utime.h>
#include "io16.h"
#include "time16.h"

/* INT 21h 2Ah: the year in the high word, month and day (DH, DL) in the
 * low one */
static unsigned long dos_date(void);
#pragma aux dos_date = \
    "mov ah, 2ah" \
    "int 21h" \
    value [cx dx] modify exact [ax cx dx];

/* INT 21h 2Ch: hour and minute (CH, CL) in the high word, second and
 * hundredth (DH, DL) in the low one */
static unsigned long dos_time(void);
#pragma aux dos_time = \
    "mov ah, 2ch" \
    "int 21h" \
    value [cx dx] modify exact [ax cx dx];

static unsigned long set_time(int fd, unsigned date, unsigned time);
#pragma aux set_time = \
    "mov ax, 5701h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] [dx] [cx] value [dx ax] modify exact [ax dx];

int __cdecl utime(const char *path, struct utimbuf *times)
{
    unsigned date, time;
    unsigned long r;
    int fd;

    if (times == NULL) {
        r = dos_date();
        date = ((unsigned)(r >> 16) - 1980) << 9 | ((unsigned)r >> 8) << 5 | ((unsigned)r & 0xff);
        r = dos_time();
        time = ((unsigned)(r >> 24)) << 11 | ((unsigned)(r >> 16) & 0xff) << 5
            | ((unsigned)r >> 8) / 2;
    } else if (__libc16_to_dos(times->modtime, &date, &time) != 0) {
        errno = EINVAL;
        return -1;
    }
    r = __libc16_dos_open(path, 0);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    fd = DOS_AX(r);
    r = set_time(fd, date, time);
    __libc16_dos_close(fd);
    if (DOS_FAILED(r))
        return __libc16_doserr(DOS_AX(r));
    return 0;
}
