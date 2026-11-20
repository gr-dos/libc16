/* _dos_getftime and _dos_setftime (docs/spec/dos.md) */

#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include "check.h"

/* DOS's packed date and time */
#define DATE(y, m, d) (((y) - 1980) << 9 | (m) << 5 | (d))
#define TIME(h, m, s) ((h) << 11 | (m) << 5 | (s) / 2)

/* The stamp of name, from a new handle to it */
static int stamp(const char *name, unsigned date, unsigned time)
{
    unsigned d = 0, t = 0;
    int h, ok;

    if (_dos_open(name, O_RDONLY, &h) != 0)
        return 0;
    ok = _dos_getftime(h, &d, &t) == 0 && d == date && t == time;
    return _dos_close(h) == 0 && ok;
}

/* Whether a directory search reports the stamp of name that a handle
 * does: the same format as struct find_t's */
static int found(const char *name)
{
    struct find_t f;
    unsigned d = 0, t = 0;
    int h, ok;

    if (_dos_open(name, O_RDONLY, &h) != 0)
        return 0;
    ok = _dos_getftime(h, &d, &t) == 0;
    return _dos_close(h) == 0 && ok && _dos_findfirst(name, _A_NORMAL, &f) == 0
        && f.wr_date == d && f.wr_time == t;
}

int main(void)
{
    unsigned date = 99, time = 99, n;
    int h = -1;

    /* the time DOS gave a new file, as a search reports it */
    CHECK(_dos_creat("FT2.TMP", _A_NORMAL, &h) == 0 && h > 4);
    CHECK(_dos_write(h, "data", 4, &n) == 0 && n == 4);
    CHECK(_dos_close(h) == 0);
    CHECK(found("FT2.TMP"));

    CHECK(_dos_creat("FT.TMP", _A_NORMAL, &h) == 0 && h > 4);
    CHECK(_dos_write(h, "data", 4, &n) == 0 && n == 4);
    /* a stamp of its own, after the close, which would otherwise stamp
     * the written file with the time. No 5700h on the handle after the
     * set: under DOSBox-X that cancels it (docs/spec/dos.md). */
    CHECK(_dos_getftime(h, &date, &time) == 0 && date != 99 && time != 99);
    CHECK(_dos_setftime(h, DATE(2001, 2, 3), TIME(4, 5, 6)) == 0);
    CHECK(_dos_close(h) == 0);
    CHECK(stamp("FT.TMP", DATE(2001, 2, 3), TIME(4, 5, 6)));

    /* untranslated: the first and last stamps DOS can hold, odd
     * seconds' halves included */
    CHECK(_dos_open("FT.TMP", O_RDWR, &h) == 0);
    CHECK(_dos_getftime(h, &date, &time) == 0);
    CHECK(date == DATE(2001, 2, 3) && time == TIME(4, 5, 6));
    CHECK(_dos_setftime(h, DATE(2107, 12, 31), TIME(23, 59, 58)) == 0);
    CHECK(_dos_close(h) == 0);
    CHECK(stamp("FT.TMP", 0xff9f, 0xbf7d));
    CHECK(_dos_open("FT.TMP", O_RDWR, &h) == 0);
    CHECK(_dos_setftime(h, DATE(1980, 1, 1), TIME(0, 0, 0)) == 0);
    CHECK(_dos_close(h) == 0);
    CHECK(stamp("FT.TMP", 0x0021, 0));

    /* a read-only file, through a read-only handle */
    CHECK(_dos_setfileattr("FT.TMP", _A_RDONLY) == 0);
    CHECK(_dos_open("FT.TMP", O_RDONLY, &h) == 0);
    CHECK(_dos_setftime(h, DATE(1999, 12, 31), TIME(12, 34, 56)) == 0);
    CHECK(_dos_close(h) == 0);
    CHECK(stamp("FT.TMP", DATE(1999, 12, 31), TIME(12, 34, 56)));
    CHECK(_dos_setfileattr("FT.TMP", _A_NORMAL) == 0);

    /* a handle that is not open: DOS's code, errno and _doserrno, and
     * the outputs unchanged */
    date = time = 99;
    errno = 0;
    _doserrno = 0;
    CHECK(_dos_getftime(h, &date, &time) == 6 && errno == EBADF && _doserrno == 6);
    CHECK(date == 99 && time == 99);
    errno = 0;
    _doserrno = 0;
    CHECK(_dos_setftime(h, DATE(2001, 2, 3), TIME(4, 5, 6)) == 6 && errno == EBADF && _doserrno == 6);
    CHECK(_dos_getftime(-1, &date, &time) == 6 && date == 99 && time == 99);

    return done();
}
