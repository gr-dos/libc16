/* Private to libc16: the handle table, DOS error mapping and INT 21h
 * calls used by the I/O members (docs/spec/io.md). */

#ifndef LIBC16_IO16_H
#define LIBC16_IO16_H

/* A data pointer that a pragma passes to DOS in DS:DX or DS:SI. In the
 * small model DS is DGROUP already and the pointer is the offset; with far
 * data (compact and large) the pragma loads DS too, which those models let
 * any code change (docs/ABI.md, "Memory models"). */
#if defined(__COMPACT__) || defined(__LARGE__)
#define DS_DX [ds dx]
#define DS_SI [ds si]
#else
#define DS_DX [dx]
#define DS_SI [si]
/* Read and write with a far buffer in every model; the buffer's segment
 * is passed in ES and DS is restored. */
unsigned long __libc16_dos_read_far(int fd, void __far *buf, unsigned n);
#pragma aux __libc16_dos_read_far = \
    "push ds" \
    "push es" \
    "pop ds" \
    "mov ah, 3fh" \
    "int 21h" \
    "pop ds" \
    "sbb dx, dx" \
    parm [bx] [es dx] [cx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_write_far(int fd, const void __far *buf, unsigned n);
#pragma aux __libc16_dos_write_far = \
    "push ds" \
    "push es" \
    "pop ds" \
    "mov ah, 40h" \
    "int 21h" \
    "pop ds" \
    "sbb dx, dx" \
    parm [bx] [es dx] [cx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_setattr(const char *path, unsigned attr);
#pragma aux __libc16_dos_setattr = \
    "mov ax, 4301h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX [cx] value [dx ax] modify exact [ax dx];

#endif

/* Handles 0 to FD_MAX - 1 have a mode in __libc16_fdflags; zero is text,
 * not appending, nothing pending. */
#define FD_MAX     20
#define FD_BINARY  0x01
#define FD_APPEND  0x02
#define FD_PENDING 0x04   /* __libc16_fdpend holds a byte for read */

extern unsigned char __libc16_fdflags[FD_MAX];
extern unsigned char __libc16_fdpend[FD_MAX];

/* Sets errno from a DOS error code and returns -1. */
int __libc16_doserr(unsigned code);

/* Each call returns DX:AX; DX is 0xFFFF if DOS set the carry flag, and
 * AX is then the error code. */
#define DOS_FAILED(r) ((unsigned)((r) >> 16) != 0)
#define DOS_AX(r)     ((unsigned)(r))

unsigned long __libc16_dos_open(const char *path, unsigned char mode);
#pragma aux __libc16_dos_open = \
    "mov ah, 3dh" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX [al] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_creat(const char *path, unsigned attr);
#pragma aux __libc16_dos_creat = \
    "mov ah, 3ch" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX [cx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_close(int fd);
#pragma aux __libc16_dos_close = \
    "mov ah, 3eh" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_read(int fd, void *buf, unsigned n);
#pragma aux __libc16_dos_read = \
    "mov ah, 3fh" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] DS_DX [cx] value [dx ax] modify exact [ax dx];

/* A count of 0 truncates the file at the file pointer. */
unsigned long __libc16_dos_write(int fd, const void *buf, unsigned n);
#pragma aux __libc16_dos_write = \
    "mov ah, 40h" \
    "int 21h" \
    "sbb dx, dx" \
    parm [bx] DS_DX [cx] value [dx ax] modify exact [ax dx];

/* Returns the new position; on failure DX is 0xFFFF and AX the error. */
unsigned long __libc16_dos_seek(int fd, long offset, unsigned char origin);
#pragma aux __libc16_dos_seek = \
    "mov ah, 42h" \
    "int 21h" \
    "sbb cx, cx" \
    "or dx, cx" \
    parm [bx] [cx dx] [al] value [dx ax] modify exact [ax cx dx];

/* Returns the attributes in AX. */
unsigned long __libc16_dos_getattr(const char *path);
#pragma aux __libc16_dos_getattr = \
    "mov ax, 4300h" \
    "int 21h" \
    "jc short fail" \
    "mov ax, cx" \
    "fail:" \
    "sbb dx, dx" \
    parm DS_DX value [dx ax] modify exact [ax cx dx];

/* Read and write with a far buffer in every model; the buffer's segment
 * is passed in ES and DS is restored. */
unsigned long __libc16_dos_read_far(int fd, void __far *buf, unsigned n);
#pragma aux __libc16_dos_read_far = \
    "push ds" \
    "push es" \
    "pop ds" \
    "mov ah, 3fh" \
    "int 21h" \
    "pop ds" \
    "sbb dx, dx" \
    parm [bx] [es dx] [cx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_write_far(int fd, const void __far *buf, unsigned n);
#pragma aux __libc16_dos_write_far = \
    "push ds" \
    "push es" \
    "pop ds" \
    "mov ah, 40h" \
    "int 21h" \
    "pop ds" \
    "sbb dx, dx" \
    parm [bx] [es dx] [cx] value [dx ax] modify exact [ax dx];

unsigned long __libc16_dos_setattr(const char *path, unsigned attr);
#pragma aux __libc16_dos_setattr = \
    "mov ax, 4301h" \
    "int 21h" \
    "sbb dx, dx" \
    parm DS_DX [cx] value [dx ax] modify exact [ax dx];

#endif
