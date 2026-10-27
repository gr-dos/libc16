/* Microsoft C's direct DOS calls: files, attributes, interrupt vectors
 * and the critical-error handler (docs/spec/dos.md) */

#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include "check.h"

static unsigned get_ds(void);
#pragma aux get_ds = "mov ax, ds" value [ax] modify exact [ax];

static unsigned get_ss(void);
#pragma aux get_ss = "mov ax, ss" value [ax] modify exact [ax];

/* Raises INT 24h as DOS would, with AX = 0102h, DI = 0007h and BP:SI =
 * 1111h:2222h. Returns the handler's action in AX, and in DX zero if BX,
 * CX, DX, SI, DI and BP came back unchanged. */
static unsigned long raise_crit(void);
#pragma aux raise_crit = \
    "push bp" \
    "push si" \
    "push di" \
    "mov ax, 0102h" \
    "mov di, 0007h" \
    "mov si, 2222h" \
    "mov bp, 1111h" \
    "mov bx, 1234h" \
    "mov cx, 5a5ah" \
    "mov dx, 0a5a5h" \
    "int 24h" \
    "xor bx, 1234h" \
    "xor cx, 5a5ah" \
    "xor dx, 0a5a5h" \
    "xor si, 2222h" \
    "xor di, 0007h" \
    "xor bp, 1111h" \
    "or dx, cx" \
    "or dx, bx" \
    "or dx, si" \
    "or dx, di" \
    "or dx, bp" \
    "pop di" \
    "pop si" \
    "pop bp" \
    "xor ah, ah" \
    value [dx ax] modify exact [ax bx cx dx];

/* A far buffer, outside DGROUP in every model */
static char __far fbuf[16];

static unsigned crit_deverr, crit_errcode, crit_ss, crit_ds;
static unsigned __far *crit_devhdr;

static int __cdecl __far crit(unsigned deverr, unsigned errcode, unsigned __far *devhdr)
{
    crit_deverr = deverr;
    crit_errcode = errcode;
    crit_devhdr = devhdr;
    crit_ss = get_ss();
    crit_ds = get_ds();
    return _HARDERR_FAIL;
}

static volatile int hits;

static void __interrupt __far isr(void)
{
    hits++;
}

static void files(void)
{
    int h = -1, h2;
    unsigned n = 99, attr = 99;

    CHECK(_dos_creat("DF.TMP", _A_NORMAL, &h) == 0 && h > 4);
    _fmemcpy(fbuf, "0123456789", 10);
    CHECK(_dos_write(h, fbuf, 10, &n) == 0 && n == 10);
    CHECK(_dos_write(h, "ab", 2, &n) == 0 && n == 2);
    CHECK(_dos_close(h) == 0);
    CHECK(_dos_close(h) == 6 && errno == EBADF);

    CHECK(_dos_getfileattr("DF.TMP", &attr) == 0 && (attr & ~_A_ARCH) == 0);
    CHECK(_dos_setfileattr("DF.TMP", _A_RDONLY) == 0);
    CHECK(_dos_getfileattr("DF.TMP", &attr) == 0 && (attr & _A_RDONLY) != 0);
    h2 = -7;
    CHECK(_dos_open("DF.TMP", O_WRONLY, &h2) == 5 && errno == EACCES && h2 == -7);
    CHECK(_dos_setfileattr("DF.TMP", _A_NORMAL) == 0);
    attr = 99;
    CHECK(_dos_getfileattr("NOSUCH.X", &attr) == 2 && errno == ENOENT && attr == 99);
    CHECK(_dos_setfileattr("NOSUCH.X", _A_NORMAL) == 2 && errno == ENOENT);
    CHECK(_dos_open("NOSUCH.X", O_RDONLY, &h2) == 2 && errno == ENOENT && h2 == -7);

    CHECK(_dos_open("DF.TMP", O_RDONLY, &h) == 0 && h > 4);
    _fmemcpy(fbuf, "xxxxxxxxxxxxxxxx", 16);
    CHECK(_dos_read(h, fbuf, 16, &n) == 0 && n == 12);
    CHECK(fbuf[0] == '0' && fbuf[9] == '9' && fbuf[10] == 'a' && fbuf[11] == 'b'
          && fbuf[12] == 'x');
    CHECK(_dos_read(h, fbuf, 16, &n) == 0 && n == 0);
    CHECK(_dos_close(h) == 0);
    n = 99;
    CHECK(_dos_read(h, fbuf, 16, &n) == 6 && errno == EBADF && n == 99);
    CHECK(_dos_write(h, fbuf, 1, &n) == 6 && errno == EBADF && n == 99);
}

static void vectors(void)
{
    void (__interrupt __far *old)() = _dos_getvect(0x66);
    union REGS r;

    _dos_setvect(0x66, isr);
    CHECK(_dos_getvect(0x66) == isr);
    hits = 0;
    int86(0x66, &r, &r);
    int86(0x66, &r, &r);
    CHECK(hits == 2);
    _dos_setvect(0x66, old);
    CHECK(_dos_getvect(0x66) == old);
}

static void critical(void)
{
    unsigned long r;

    _harderr(crit);
    r = raise_crit();
    CHECK((unsigned)r == _HARDERR_FAIL);
    CHECK((unsigned)(r >> 16) == 0);
    CHECK(crit_deverr == 0x0102 && crit_errcode == 0x0007);
    CHECK(FP_SEG(crit_devhdr) == 0x1111 && FP_OFF(crit_devhdr) == 0x2222);
    CHECK(crit_ss == get_ss() && crit_ds == get_ss());
}

int main(void)
{
    files();
    vectors();
    critical();
    return done();
}
