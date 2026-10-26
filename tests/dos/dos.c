/* DOS interface, environment and paths (docs/spec/dos.md) */

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

int main(void)
{
    finds();
    interrupts();
    environment();
    paths();
    return done();
}
