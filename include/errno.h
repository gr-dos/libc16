/* errno.h - libc16 (C89 7.1.4); values are Microsoft C's */

#ifndef _LIBC16_ERRNO_H
#define _LIBC16_ERRNO_H

extern int __cdecl errno;

#define ENOENT   2
#define E2BIG    7
#define ENOEXEC  8
#define EBADF    9
#define ENOMEM   12
#define EACCES   13
#define EEXIST   17
#define EXDEV    18
#define EINVAL   22
#define EMFILE   24
#define ENOSPC   28
#define EDOM     33
#define ERANGE   34

#endif
