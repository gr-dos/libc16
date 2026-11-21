/* gcat: the example consumer of docs/spec/glob.md, a minimal cat. Each
 * operand is expanded in order with libc16_glob; one that matches
 * nothing, or is a bad pattern, is reported on standard error. Then
 * each file is copied to standard output, in binary mode, byte for byte. The status is 1
 * if any operand failed. */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <libc16.h>
#include <string.h>

static void say(const char *s)
{
    write(2, s, strlen(s));
}

static void report(const char *what, const char *why)
{
    say("gcat: ");
    say(what);
    say(": ");
    say(why);
    say("\r\n");
}

int main(int argc, char **argv)
{
    static char buf[512];
    libc16_glob_t g = { 0 };
    int i, n, fd, status = 0;
    unsigned k;

    /* byte for byte: no LF to CR LF on the way out */
    setmode(1, O_BINARY);
    for (i = 1; i < argc; i++) {
        int r = libc16_glob(argv[i], LIBC16_GLOB_APPEND, &g);

        if (r == LIBC16_GLOB_NOMATCH) {
            report(argv[i], "no match");
            status = 1;
        } else if (r != 0) {
            report(argv[i], strerror(errno));
            status = 1;
        }
    }
    for (k = 0; k < g.gl_pathc; k++) {
        fd = open(g.gl_pathv[k], O_RDONLY | O_BINARY);
        if (fd < 0) {
            report(g.gl_pathv[k], strerror(errno));
            status = 1;
            continue;
        }
        while ((n = read(fd, buf, sizeof buf)) > 0)
            if (write(1, buf, n) != n) {
                report("standard output", strerror(errno));
                status = 1;
                break;
            }
        if (n < 0) {
            report(g.gl_pathv[k], strerror(errno));
            status = 1;
        }
        close(fd);
    }
    libc16_globfree(&g);
    return status;
}
