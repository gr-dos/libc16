/* Checks the files bufexit and bufabrt left (docs/spec/stdio.md,
 * "Termination"). */

#include <fcntl.h>
#include <io.h>
#include <string.h>
#include "check.h"

static char got[64];

static int is(const char *name, const char *want)
{
    int fd = open(name, O_RDONLY | O_BINARY), n;

    if (fd < 0)
        return 0;
    n = read(fd, got, sizeof got);
    close(fd);
    return n == (int)strlen(want) && memcmp(got, want, n) == 0;
}

int main(void)
{
    CHECK(is("BUFEXIT.TXT", "file pending"));
    CHECK(is("BUFABRT.TXT", "flushed\r\n"));
    return done();
}
