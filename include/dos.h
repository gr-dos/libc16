/* dos.h - libc16 (Microsoft C's DOS interface, the parts docs/spec/dos.md
 * implements) */

#ifndef _LIBC16_DOS_H
#define _LIBC16_DOS_H

/* File attributes */
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_VOLID  0x08
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20

/* The DOS disk transfer area of a directory search (INT 21h 4Eh) */
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

struct WORDREGS {
    unsigned int ax, bx, cx, dx, si, di, cflag;
};

struct BYTEREGS {
    unsigned char al, ah, bl, bh, cl, ch, dl, dh;
};

union REGS {
    struct WORDREGS x;
    struct BYTEREGS h;
};

unsigned __cdecl _dos_findfirst(const char *path, unsigned attr, struct find_t *f);
unsigned __cdecl _dos_findnext(struct find_t *f);
int      __cdecl int86(int intno, union REGS *in, union REGS *out);

#endif
