/* stat, chmod, mkdir and utime (docs/spec/files.md), run with TZ unset
 * and with TZ=EST5EDT */

#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <time.h>
#include "check.h"

/* 2026-07-01 12:00:00 UTC, 2026-01-15 12:00:00 UTC, and 1979-12-31 */
#define SUMMER 1782907200L
#define WINTER 1768478400L
#define OLD    315446400L

/* INT 21h 41h, so that each file starts without DOSBox-X's record of its
 * attributes from an earlier run (see below) */
static void remove_file(const char *path);
#pragma aux remove_file = \
    "mov ah, 41h" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

static void make(const char *name, unsigned n)
{
    int fd;

    remove_file(name);
    fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, "0123456789", n) == (int)n && close(fd) == 0);
}

/* The DOS date and time stored for name */
static int dos_stamp(const char *name, unsigned *date, unsigned *time)
{
    struct find_t f;

    if (_dos_findfirst(name, _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR, &f) != 0)
        return -1;
    *date = f.wr_date;
    *time = f.wr_time;
    return 0;
}

static void set_attr(const char *name, unsigned attr)
{
    CHECK(_dos_setfileattr(name, attr) == 0);
}

/* INT 21h 3Ah, so that a run starts without the directory an earlier one
 * made */
static void remove_dir(const char *path);
#pragma aux remove_dir = \
    "mov ah, 3ah" \
    "int 21h" \
    parm T_DS_DX modify exact [ax];

/* Each run makes its own directory: D, or DE under EST5EDT.
 *
 * DOSBox-X keeps the attributes of a file that has had any in a second
 * host file, and from then on its find first reports that file's size and
 * time.  So only A.TXT has its attributes changed, and only its mode is
 * checked; F.TXT's size and times are checked on a file that never has. */
static const char *dir;

static void stats(void)
{
    struct stat st;

    make("F.TXT", 7);
    memset(&st, 0x55, sizeof st);
    CHECK(stat("F.TXT", &st) == 0);
    CHECK(st.st_mode == (S_IFREG | 0666) && st.st_size == 7 && st.st_nlink == 1);
    CHECK(st.st_dev == 2 && st.st_rdev == 2 && st.st_ino == 0 && st.st_uid == 0 && st.st_gid == 0);
    CHECK(st.st_atime == st.st_mtime && st.st_ctime == st.st_mtime);
    CHECK(stat("C:F.TXT", &st) == 0 && st.st_dev == 2);

    make("A.TXT", 1);
    set_attr("A.TXT", _A_RDONLY);
    CHECK(stat("A.TXT", &st) == 0 && st.st_mode == (S_IFREG | 0444));
    set_attr("A.TXT", _A_HIDDEN);
    CHECK(stat("A.TXT", &st) == 0 && st.st_mode == (S_IFREG | 0666));
    set_attr("A.TXT", _A_NORMAL);
    make("P.EXE", 1);
    make("P.BAT", 1);
    make("P.COM", 1);
    CHECK(stat("P.EXE", &st) == 0 && st.st_mode == (S_IFREG | 0777));
    CHECK(stat("p.bat", &st) == 0 && st.st_mode == (S_IFREG | 0777));
    CHECK(stat("P.COM", &st) == 0 && st.st_mode == (S_IFREG | 0777));

    remove_dir(dir);
    CHECK(mkdir(dir) == 0);
    CHECK(mkdir(dir) == -1 && errno == EACCES);
    CHECK(mkdir("NO\\SUCH\\D") == -1 && errno == ENOENT);
    CHECK(stat(dir, &st) == 0 && st.st_mode == (S_IFDIR | 0777) && st.st_size == 0);
    CHECK(stat(daylight ? "DE\\." : "D\\.", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    CHECK(stat(daylight ? "DE\\.." : "D\\..", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    CHECK(stat("C:\\", &st) == 0 && st.st_mode == (S_IFDIR | 0777) && st.st_dev == 2);
    CHECK(stat("\\", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    CHECK(stat("C:", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    CHECK(stat("/", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);
    CHECK(stat(".", &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR);

    CHECK(stat("NOSUCH.X", &st) == -1 && errno == ENOENT);
    CHECK(stat("*.TXT", &st) == -1 && errno == ENOENT);
    CHECK(stat("F.?XT", &st) == -1 && errno == ENOENT);
}

static void times(void)
{
    struct utimbuf ut;
    struct stat st;
    unsigned date, time, want_hour;
    struct tm *tm;

    /* What DOS stores is local time: 12:00 UTC is 8:00 EDT in July and
     * 7:00 EST in January. */
    tzset();
    ut.actime = 0;
    ut.modtime = SUMMER;
    CHECK(utime("F.TXT", &ut) == 0);
    CHECK(dos_stamp("F.TXT", &date, &time) == 0);
    want_hour = daylight ? 8 : 12;
    CHECK(date == ((2026 - 1980) << 9 | 7 << 5 | 1) && time == want_hour << 11);
    CHECK(stat("F.TXT", &st) == 0 && st.st_mtime == SUMMER && st.st_atime == SUMMER);
    ut.modtime = WINTER + 1;    /* DOS keeps even seconds */
    CHECK(utime("F.TXT", &ut) == 0);
    CHECK(dos_stamp("F.TXT", &date, &time) == 0);
    want_hour = daylight ? 7 : 12;
    CHECK(date == ((2026 - 1980) << 9 | 1 << 5 | 15) && time == want_hour << 11);
    CHECK(stat("F.TXT", &st) == 0 && st.st_mtime == WINTER);

    /* read-only files can be changed; directories and missing files not */
    CHECK(chmod("A.TXT", S_IREAD) == 0);
    CHECK(_dos_getfileattr("A.TXT", &date) == 0 && (date & _A_RDONLY));
    ut.modtime = SUMMER;
    CHECK(utime("A.TXT", &ut) == 0);
    CHECK(stat("A.TXT", &st) == 0 && st.st_mode == (S_IFREG | 0444));
    CHECK(chmod("A.TXT", S_IREAD | S_IWRITE) == 0);
    CHECK(_dos_getfileattr("A.TXT", &date) == 0 && !(date & _A_RDONLY));
    CHECK(utime("F.TXT", &ut) == 0);
    CHECK(chmod("NOSUCH.X", S_IWRITE) == -1 && errno == ENOENT);
    CHECK(utime(dir, &ut) == -1 && errno == EACCES);
    CHECK(utime("NOSUCH.X", &ut) == -1 && errno == ENOENT);
    ut.modtime = OLD;
    CHECK(utime("F.TXT", &ut) == -1 && errno == EINVAL);
    CHECK(stat("F.TXT", &st) == 0 && st.st_mtime == SUMMER);

    /* NULL is now, as DOS's clock tells it */
    CHECK(utime("F.TXT", NULL) == 0);
    CHECK(dos_stamp("F.TXT", &date, &time) == 0 && stat("F.TXT", &st) == 0);
    tm = localtime(&st.st_mtime);
    CHECK(tm->tm_year + 1900 == 1980 + (date >> 9) && tm->tm_mon + 1 == (int)((date >> 5) & 15)
          && tm->tm_mday == (int)(date & 31) && tm->tm_hour == (int)(time >> 11)
          && tm->tm_min == (int)((time >> 5) & 63) && tm->tm_sec == (int)(time & 31) * 2);
}

int main(void)
{
    tzset();
    dir = daylight ? "DE" : "D";
    stats();
    times();
    return done();
}
