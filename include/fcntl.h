/* fcntl.h - libc16 (open flags; values are Microsoft C's) */

#ifndef _LIBC16_FCNTL_H
#define _LIBC16_FCNTL_H

#define O_RDONLY 0x0000
#define O_WRONLY 0x0001
#define O_RDWR   0x0002
#define O_APPEND 0x0008
#define O_CREAT  0x0100
#define O_TRUNC  0x0200
#define O_EXCL   0x0400
#define O_TEXT   0x4000
#define O_BINARY 0x8000

#endif
