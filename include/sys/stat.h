/* sys/stat.h - libc16 (Microsoft C's stat, and permission flags for open
 * and chmod; values are Microsoft C's) */

#ifndef _LIBC16_SYS_STAT_H
#define _LIBC16_SYS_STAT_H

#include <_libc16.h>
#include <sys/types.h>

#define S_IFMT   0xF000         /* file type */
#define S_IFDIR  0x4000
#define S_IFREG  0x8000
#define S_IREAD  0x0100         /* the owner's permissions */
#define S_IWRITE 0x0080
#define S_IEXEC  0x0040

struct stat {
    dev_t st_dev;
    ino_t st_ino;
    unsigned short st_mode;
    short st_nlink;
    short st_uid;
    short st_gid;
    dev_t st_rdev;
    off_t st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

int __cdecl stat(const char *path, struct stat *buf);

#endif
