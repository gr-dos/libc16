/* isatty (docs/spec/io.md). Under tests/run-dos handle 1 is redirected to
 * a file, and handles 0 and 2 are the console. */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include "check.h"

int main(void)
{
    int fd;

    CHECK(isatty(0) == 1);
    CHECK(isatty(1) == 0);
    CHECK(isatty(2) == 1);

    fd = open("CON", O_RDWR);
    CHECK(fd >= 0 && isatty(fd) == 1);
    CHECK(close(fd) == 0);
    fd = open("NUL", O_RDWR);
    CHECK(fd >= 0 && isatty(fd) == 1);
    CHECK(close(fd) == 0);

    /* a file, whose position isatty leaves alone */
    fd = open("ISATTY.TXT", O_RDWR | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, "abc", 3) == 3 && lseek(fd, 1L, SEEK_SET) == 1L);
    errno = 0;
    CHECK(isatty(fd) == 0 && errno == 0);
    CHECK(lseek(fd, 0L, SEEK_CUR) == 1L);
    CHECK(close(fd) == 0);

    /* not open */
    errno = 0;
    CHECK(isatty(fd) == 0 && errno == EBADF);
    errno = 0;
    CHECK(isatty(-1) == 0 && errno == EBADF);
    errno = 0;
    CHECK(isatty(20) == 0 && errno == EBADF);
    return done();
}
