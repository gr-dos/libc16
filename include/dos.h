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

/* The DOS disk transfer area of a directory search (INT 21h 4Eh), 43 bytes */
#pragma pack(__push, 1)
struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};
#pragma pack(__pop)

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

struct SREGS {
    unsigned int es, cs, ss, ds;
};

/* The segment and offset words of a far pointer lvalue, as lvalues */
#define FP_SEG(fp) (((unsigned *)&(fp))[1])
#define FP_OFF(fp) (((unsigned *)&(fp))[0])

/* PSP segment of the program */
extern unsigned __cdecl _psp;

unsigned __cdecl _dos_findfirst(const char *path, unsigned attr, struct find_t *f);
unsigned __cdecl _dos_findnext(struct find_t *f);
int      __cdecl int86(int intno, union REGS *in, union REGS *out);
int      __cdecl int86x(int intno, union REGS *in, union REGS *out, struct SREGS *segs);
int      __cdecl intdos(union REGS *in, union REGS *out);
int      __cdecl intdosx(union REGS *in, union REGS *out, struct SREGS *segs);
void     __cdecl segread(struct SREGS *segs);
int      __cdecl bdos(int dosfn, unsigned dosdx, unsigned dosal);

#endif
