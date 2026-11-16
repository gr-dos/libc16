/* What abort wrote to handle 2 in tests/stdlib/abort */

#include <fcntl.h>
#include <io.h>
#include <string.h>
#include "check.h"

int main(void)
{
    static const char want[] = "abnormal program termination\r\n";
    char buf[64];
    int fd, n;

    fd = open("ABORT.ERR", O_RDONLY | O_BINARY);
    CHECK(fd >= 0);
    n = read(fd, buf, sizeof buf);
    CHECK(n == (int)strlen(want) && memcmp(buf, want, n) == 0);
    CHECK(close(fd) == 0);
    return done();
}
