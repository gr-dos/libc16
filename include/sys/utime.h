/* sys/utime.h - libc16 (Microsoft C's utime) */

#ifndef _LIBC16_SYS_UTIME_H
#define _LIBC16_SYS_UTIME_H

#include <_libc16.h>
#include <sys/types.h>

struct utimbuf {
    time_t actime;              /* access time, not stored by DOS */
    time_t modtime;             /* modification time */
};

int __cdecl utime(const char *path, struct utimbuf *times);

#endif
