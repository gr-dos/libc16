/* DOS interface, environment and paths (docs/spec/dos.md) */

#include <conio.h>
#include <dos.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"

static unsigned long get_dta(void);
#pragma aux get_dta = \
    "mov ah, 2fh" \
    "int 21h" \
    "mov dx, es" \
    "mov ax, bx" \
    value [dx ax] modify exact [ax bx dx es];

static void set_dta(unsigned long dta);
#pragma aux set_dta = \
    "push ds" \
    "mov ds, cx" \
    "mov ah, 1ah" \
    "int 21h" \
    "pop ds" \
    parm [cx dx] modify exact [ax];

static unsigned get_cs(void);
#pragma aux get_cs = "mov ax, cs" value [ax] modify exact [ax];

static unsigned get_ds(void);
#pragma aux get_ds = "mov ax, ds" value [ax] modify exact [ax];

static unsigned get_es(void);
#pragma aux get_es = "mov ax, es" value [ax] modify exact [ax];

unsigned __cdecl es_after_int86x(unsigned es, int intno, union REGS *in,
                                 union REGS *out, struct SREGS *segs);

static void make_dir(const char *path);
#pragma aux make_dir = \
    "mov ah, 39h" \
    "int 21h" \
    parm [dx] modify exact [ax];

static void finds(void)
{
    struct find_t f;
    unsigned long dta = get_dta();
    int one = 0, two = 0, other = 0;
    unsigned rc;

    CHECK(sizeof f == 43 && offsetof(struct find_t, size) == 26
          && offsetof(struct find_t, name) == 30);
    for (rc = _dos_findfirst("*.FND", _A_NORMAL, &f); rc == 0; rc = _dos_findnext(&f)) {
        if (strcmp(f.name, "ONE.FND") == 0)
            one += f.size == 5 && (f.attrib & _A_SUBDIR) == 0;
        else if (strcmp(f.name, "TWO.FND") == 0)
            two += f.size == 10;
        else
            other++;
    }
    CHECK(one == 1 && two == 1 && other == 0);
    CHECK((rc == 18 || rc == 2) && errno == ENOENT);
    errno = 0;
    CHECK(_dos_findfirst("*.NONE", _A_NORMAL, &f) != 0 && errno == ENOENT);
    CHECK(_dos_findfirst("NODIR\\*.*", _A_NORMAL, &f) != 0 && errno == ENOENT);
    make_dir("DIR.FND");
    CHECK(_dos_findfirst("DIR.FND", _A_NORMAL, &f) != 0);
    CHECK(_dos_findfirst("DIR.FND", _A_SUBDIR, &f) == 0 && (f.attrib & _A_SUBDIR)
          && strcmp(f.name, "DIR.FND") == 0);
    CHECK(get_dta() == dta);
}

static void interrupts(void)
{
    union REGS in, out;
    char cwd[64];

    memset(&in, 0, sizeof in);
    in.h.ah = 0x30;
    CHECK(int86(0x21, &in, &out) == out.x.ax && out.h.al >= 2 && out.x.cflag == 0);
    in.h.ah = 0x3d;
    in.h.al = 0;
    in.x.dx = (unsigned)"NOSUCH.X";
    CHECK(int86(0x21, &in, &out) == 2 && out.x.ax == 2 && out.x.cflag != 0);
    in.h.ah = 0x19;
    CHECK(int86(0x21, &in, &in) == in.x.ax && in.h.al == 2 && in.x.cflag == 0);
    memset(cwd, 'x', sizeof cwd);
    in.h.ah = 0x47;
    in.h.dl = 0;
    in.x.si = (unsigned)cwd;
    in.x.di = 0x1234;
    CHECK(int86(0x21, &in, &out) == out.x.ax && out.x.cflag == 0 && cwd[0] == '\0'
          && out.x.si == (unsigned)cwd && out.x.di == 0x1234);
    CHECK(int86(0x12, &in, &out) == out.x.ax && out.x.ax >= 64 && out.x.ax <= 640);
}

static void environment(void)
{
    const char *v = getenv("COMSPEC");

    CHECK(v != NULL && strcmp(v, "Z:\\COMMAND.COM") == 0);
    CHECK(getenv("COMSPE") == NULL && getenv("COMSPECX") == NULL);
    CHECK(getenv("comspec") == NULL && getenv("NOSUCHVAR") == NULL);
    CHECK(getenv("") == NULL);
}

static int path_is(const char *path, const char *want)
{
    char buf[_MAX_PATH];

    return _fullpath(buf, path, sizeof buf) == buf && strcmp(buf, want) == 0;
}

static void paths(void)
{
    char buf[_MAX_PATH];

    CHECK(path_is("x.txt", "C:\\x.txt"));
    CHECK(path_is(".", "C:\\"));
    CHECK(path_is("a/b/../c", "C:\\a\\c"));
    CHECK(path_is("\\a\\.\\b\\", "C:\\a\\b"));
    CHECK(path_is("a\\\\b//c", "C:\\a\\b\\c"));
    CHECK(path_is("c:..\\..", "C:\\"));
    CHECK(path_is("c:sub", "C:\\sub"));
    CHECK(path_is("Z:\\", "Z:\\"));
    CHECK(path_is("z:foo", "Z:\\foo"));
    CHECK(path_is("DIR.FND\\x", "C:\\DIR.FND\\x"));
    CHECK(_fullpath(buf, "q:x", sizeof buf) == NULL && errno == ENOENT);
    CHECK(_fullpath(buf, "q:\\x", sizeof buf) == NULL && errno == ENOENT);
    CHECK(_fullpath(buf, "1:x", sizeof buf) == NULL && errno == ENOENT);
    CHECK(_fullpath(buf, "", sizeof buf) == NULL && errno == EINVAL);
    CHECK(_fullpath(NULL, "x", sizeof buf) == NULL && errno == EINVAL);
    CHECK(_fullpath(buf, "abc", 6) == NULL && errno == ERANGE);
    CHECK(_fullpath(buf, "abc", 7) == buf && strcmp(buf, "C:\\abc") == 0);
}

static void segments(void)
{
    union REGS in, out;
    struct SREGS segs;
    unsigned long dta = get_dta();
    char buf[43];

    CHECK(sizeof segs == 8 && offsetof(struct SREGS, es) == 0
          && offsetof(struct SREGS, cs) == 2 && offsetof(struct SREGS, ss) == 4
          && offsetof(struct SREGS, ds) == 6);
    memset(&segs, 0, sizeof segs);
    segread(&segs);
    CHECK(segs.cs == get_cs() && segs.ds == get_ds() && segs.ss == get_ds()
          && segs.es == get_es());

    /* Set the disk transfer area from DS:DX, then read it back in ES:BX. */
    memset(&in, 0, sizeof in);
    in.h.ah = 0x1a;
    in.x.dx = 0x5678;
    segs.ds = 0x1234;
    segs.es = 0;
    segs.cs = segs.ss = 0xabcd;
    CHECK(int86x(0x21, &in, &out, &segs) == out.x.ax && out.x.cflag == 0
          && segs.ds == 0x1234 && segs.cs == 0xabcd && segs.ss == 0xabcd);
    in.h.ah = 0x2f;
    segs.ds = get_ds();
    CHECK(intdosx(&in, &in, &segs) == in.x.ax && in.x.bx == 0x5678 && segs.es == 0x1234
          && in.x.cflag == 0 && segs.ds == get_ds());
    CHECK(get_dta() == 0x12345678UL);

    /* int86x returns the interrupt's ES in segs->es and restores the caller's. */
    in.h.ah = 0x2f;
    segs.es = 0x4321;
    segs.ds = get_ds();
    CHECK(es_after_int86x(0x2345, 0x21, &in, &out, &segs) == 0x2345
          && out.x.bx == 0x5678 && segs.es == 0x1234 && segs.ds == get_ds());
    in.h.ah = 0x19;
    segs.es = 0x4321;
    CHECK(es_after_int86x(0x2345, 0x21, &in, &out, &segs) == 0x2345
          && out.h.al == 2 && segs.es == 0x4321 && segs.ds == get_ds());
    set_dta(dta);
    in.h.ah = 0x3d;
    in.h.al = 0;
    in.x.dx = (unsigned)"NOSUCH.X";
    CHECK(int86x(0x21, &in, &out, &segs) == 2 && out.x.ax == 2 && out.x.cflag != 0);
    CHECK(intdos(&in, &out) == 2 && out.x.ax == 2 && out.x.cflag != 0);
    in.h.ah = 0x19;
    CHECK(intdos(&in, &out) == out.x.ax && out.h.al == 2 && out.x.cflag == 0);

    CHECK((bdos(0x19, 0x1234, 0x5678) & 0xff) == 2);
    bdos(0x1a, (unsigned)buf, 0);
    CHECK(get_dta() == ((unsigned long)get_ds() << 16 | (unsigned)buf));
    set_dta(dta);
    CHECK(kbhit() == 0);
}

static void far_pointers(void)
{
    char __far *p = 0;
    char __far *q;

    FP_SEG(p) = 0x1234;
    FP_OFF(p) = 0x5678;
    CHECK((unsigned long)p == 0x12345678UL && FP_SEG(p) == 0x1234 && FP_OFF(p) == 0x5678);
    FP_SEG(p) += 0x10;
    FP_OFF(p)++;
    CHECK((unsigned long)p == 0x12445679UL);
    q = p;
    CHECK(FP_SEG(q) == 0x1244 && FP_OFF(q) == 0x5679);
}

int main(void)
{
    finds();
    interrupts();
    segments();
    far_pointers();
    environment();
    paths();
    return done();
}
