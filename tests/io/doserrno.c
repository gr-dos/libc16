/* _doserrno, the DOS error code of the last failed DOS call
 * (docs/spec/io.md, "_doserrno"). tests/stdbuf/wrhook makes writes short
 * or fail, as DOS does. */

#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
void __cdecl wrhook_limit(unsigned skip, unsigned max);
void __cdecl wrhook_fail(unsigned skip, unsigned err);

static char buf[16];

int main(void)
{
    int fd, h;
    unsigned n;
    struct stat st;

    CHECK(_doserrno == 0);

    /* file not found and path not found: one errno, two DOS codes */
    errno = 0;
    CHECK(open("NOFILE.XYZ", O_RDONLY) == -1 && errno == ENOENT && _doserrno == 2);
    errno = 0;
    CHECK(open("NODIR\\X.TXT", O_RDONLY) == -1 && errno == ENOENT && _doserrno == 3);

    /* access denied: a directory that exists */
    rmdir("DE.DIR");
    CHECK(mkdir("DE.DIR") == 0);
    _doserrno = 0;
    errno = 0;
    CHECK(mkdir("DE.DIR") == -1 && errno == EACCES && _doserrno == 5);
    CHECK(rmdir("DE.DIR") == 0);

    /* a successful call after a failure leaves it */
    CHECK(open("NODIR\\X.TXT", O_RDONLY) == -1 && _doserrno == 3);
    fd = open("DE.TMP", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
    CHECK(fd > 4 && _doserrno == 3);
    CHECK(write(fd, "abc", 3) == 3 && _doserrno == 3);
    CHECK(close(fd) == 0 && _doserrno == 3);
    /* open of a missing file with O_CREAT recovers from DOS's error 2 */
    remove("DE2.TMP");
    _doserrno = 77;
    fd = open("DE2.TMP", O_WRONLY | O_CREAT | O_BINARY, 0666);
    CHECK(fd > 4 && _doserrno == 77);
    CHECK(close(fd) == 0 && remove("DE2.TMP") == 0 && _doserrno == 77);

    /* errors the runtime finds itself: errno only */
    _doserrno = 77;
    errno = 0;
    CHECK(fopen("DE.TMP", "rw") == NULL && errno == EINVAL && _doserrno == 77);
    errno = 0;
    CHECK(open("DE.TMP", 3) == -1 && errno == EINVAL && _doserrno == 77);
    errno = 0;
    CHECK(remove("*.TMP") == -1 && errno == ENOENT && _doserrno == 77);
    errno = 0;
    CHECK(setmode(-1, O_BINARY) == -1 && errno == EBADF && _doserrno == 77);

    /* a short write, which DOS reports as a success: ENOSPC and no DOS
     * code; a failed one: DOS's code */
    fd = open("DE.TMP", O_WRONLY | O_TRUNC | O_BINARY);
    CHECK(fd > 4);
    wrhook_install();
    wrhook_watch(fd);
    wrhook_limit(0, 1);
    _doserrno = 77;
    errno = 0;
    CHECK(write(fd, "abc", 3) == -1 && errno == ENOSPC && _doserrno == 77);
    wrhook_fail(0, 29);
    errno = 0;
    CHECK(write(fd, "abc", 3) == -1 && errno == EACCES && _doserrno == 29);
    wrhook_watch(-1);
    wrhook_remove();
    CHECK(close(fd) == 0);

    /* the _dos_ functions: return values and errno as before, and
     * _doserrno their code */
    _doserrno = 0;
    errno = 0;
    h = 99;
    CHECK(_dos_open("NOFILE.XYZ", O_RDONLY, &h) == 2 && h == 99 && errno == ENOENT && _doserrno == 2);
    CHECK(_dos_open("NODIR\\X.TXT", O_RDONLY, &h) == 3 && h == 99 && errno == ENOENT && _doserrno == 3);
    n = 99;
    errno = 0;
    CHECK(_dos_read(-1, buf, sizeof buf, &n) == 6 && n == 99 && errno == EBADF && _doserrno == 6);
    CHECK(_dos_open("DE.TMP", O_RDONLY, &h) == 0 && _doserrno == 6);
    CHECK(_dos_read(h, buf, sizeof buf, &n) == 0 && n == 1 && _doserrno == 6);
    CHECK(_dos_close(h) == 0 && _doserrno == 6);
    CHECK(_dos_close(h) == 6 && errno == EBADF && _doserrno == 6);

    /* errno that is not the table's: DOS's code all the same */
    _doserrno = 0;
    errno = 0;
    CHECK(isatty(-1) == 0 && errno == EBADF && _doserrno == 6);
    _doserrno = 0;
    errno = 0;
    CHECK(dup2(15, 16) == -1 && errno == EBADF && _doserrno == 6);
    _doserrno = 77;
    errno = 0;
    CHECK(dup2(1, 20) == -1 && errno == EBADF && _doserrno == 77);
    _doserrno = 0;
    errno = 0;
    CHECK(fdopen(15, "r") == NULL && errno == EBADF && _doserrno == 6);
    _doserrno = 77;
    errno = 0;
    CHECK(fdopen(20, "r") == NULL && errno == EBADF && _doserrno == 77);
    /* 3 from DOS, or 5 from DOSBox-X */
    _doserrno = 0;
    errno = 0;
    CHECK(rename("DE.TMP", "NODIR\\X.TMP") == -1 && errno == ENOENT
          && (_doserrno == 3 || _doserrno == 5));
    /* a root directory: its failed search is not stat's failure */
    _doserrno = 77;
    CHECK(stat("C:\\", &st) == 0 && (st.st_mode & S_IFDIR) && _doserrno == 77);
    CHECK(stat("C:\\NOFILE.XYZ", &st) == -1 && errno == ENOENT
          && (_doserrno == 2 || _doserrno == 18));

    CHECK(remove("DE.TMP") == 0);
    return done();
}
