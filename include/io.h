/* io.h - libc16 (Microsoft C's low-level I/O) */

#ifndef _LIBC16_IO_H
#define _LIBC16_IO_H

int  __cdecl access(const char *path, int mode);
int  __cdecl chsize(int fd, long size);
int  __cdecl close(int fd);
long __cdecl filelength(int fd);
long __cdecl lseek(int fd, long offset, int origin);
int  __cdecl open(const char *path, int oflag, ...);
int  __cdecl read(int fd, void *buf, unsigned n);
int  __cdecl setmode(int fd, int mode);
int  __cdecl write(int fd, const void *buf, unsigned n);

#endif
