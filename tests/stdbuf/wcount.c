/* How many DOS writes buffered output makes (docs/spec/stdio.md,
 * "Buffering", "Performance"), counted by tests/stdbuf/wrhook, and that
 * the file is the same as unbuffered output makes it. */

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
unsigned __cdecl wrhook_count(void);

#define LINES 2000
#define NONE  (-1)      /* unbuffered: no setvbuf */

static char big[20000];
static char a[512], b[512];

/* Four items a line, as an interpreter's print of a list writes them. */
static void lines(FILE *fp, int how)
{
    char num[8];
    unsigned i, k;

    for (i = 0; i < LINES; i++) {
        k = i;
        num[5] = '\0';
        num[4] = (char)('0' + k % 10);
        num[3] = (char)('0' + (k /= 10) % 10);
        num[2] = (char)('0' + (k /= 10) % 10);
        num[1] = (char)('0' + (k /= 10) % 10);
        num[0] = 'n';
        switch (how) {
        case 0:
            fputs(num, fp);
            fputs(": ", fp);
            fputs("the quick brown fox", fp);
            fputs("\n", fp);
            break;
        case 1:
            fwrite(num, 1, 5, fp);
            fwrite(": ", 1, 2, fp);
            fwrite("the quick brown fox", 1, 19, fp);
            fwrite("\n", 1, 1, fp);
            break;
        default:
            for (k = 0; k < 5; k++)
                putc(num[k], fp);
            putc(':', fp);
            putc(' ', fp);
            for (k = 0; k < 19; k++)
                putc("the quick brown fox"[k], fp);
            putc('\n', fp);
        }
    }
}

/* The same bytes in two files */
static int same(const char *x, const char *y)
{
    int fx = open(x, O_RDONLY | O_BINARY), fy = open(y, O_RDONLY | O_BINARY), nx, ny, ok = 1;

    if (fx < 0 || fy < 0)
        return 0;
    do {
        nx = read(fx, a, sizeof a);
        ny = read(fy, b, sizeof b);
        if (nx != ny || (nx > 0 && memcmp(a, b, nx) != 0))
            ok = 0;
    } while (ok && nx > 0);
    close(fx);
    close(fy);
    return ok;
}

static unsigned run(const char *name, const char *mode, int buffer, int how, unsigned size)
{
    FILE *fp = fopen(name, mode);
    unsigned n;

    CHECK(fp != NULL);
    if (buffer != NONE)
        CHECK(setvbuf(fp, NULL, buffer, size) == 0);
    wrhook_watch(fileno(fp));
    lines(fp, how);
    CHECK(fclose(fp) == 0);
    n = wrhook_count();
    t_puts(name);
    t_puts(": ");
    t_putu(n);
    t_puts(" writes\r\n");
    return n;
}

int main(void)
{
    FILE *fp;
    unsigned i, n;
    long bytes;

    wrhook_install();

    /* 2,000 lines of 4 items, 27 bytes and its \r\n: 58,000 bytes */
    bytes = (long)LINES * 28;
    CHECK(run("UNBUF.TXT", "w", NONE, 0, 0) == 4 * LINES);
    /* full buffering: one write per buffer, BUFSIZ less at most 1 byte
     * (a \r\n is not split), and the last */
    n = run("FPUTS.TXT", "w", _IOFBF, 0, 0);
    CHECK(n <= bytes / (BUFSIZ - 1) + 1);
    CHECK(same("UNBUF.TXT", "FPUTS.TXT"));
    n = run("FWRITE.TXT", "w", _IOFBF, 1, 0);
    CHECK(n <= bytes / (BUFSIZ - 1) + 1);
    CHECK(same("UNBUF.TXT", "FWRITE.TXT"));
    n = run("PUTC.TXT", "w", _IOFBF, 2, 0);
    CHECK(n <= bytes / (BUFSIZ - 1) + 1);
    CHECK(same("UNBUF.TXT", "PUTC.TXT"));
    n = run("BIG.TXT", "w", _IOFBF, 0, 4096);
    CHECK(n <= bytes / 4095 + 1);
    CHECK(same("UNBUF.TXT", "BIG.TXT"));
    /* line buffering: one write per line */
    CHECK(run("LINE.TXT", "w", _IOLBF, 0, 0) == LINES);
    CHECK(same("UNBUF.TXT", "LINE.TXT"));
    /* binary mode: no \r */
    n = run("BIN.TXT", "wb", _IOFBF, 1, 0);
    CHECK(n == (LINES * 27L + BUFSIZ - 1) / BUFSIZ);

    /* A binary request as large as the buffer is not copied: what is
     * pending, then the request, one write each. */
    for (i = 0; i < sizeof big; i++)
        big[i] = (char)i;
    fp = fopen("SEQ.BIN", "wb");
    CHECK(fp != NULL && setvbuf(fp, NULL, _IOFBF, 0) == 0);
    wrhook_watch(fileno(fp));
    CHECK(fwrite(big, 1, sizeof big, fp) == sizeof big && wrhook_count() == 1);
    CHECK(fwrite(big, 1, 3, fp) == 3 && wrhook_count() == 1);
    CHECK(fwrite(big, 1, BUFSIZ, fp) == BUFSIZ && wrhook_count() == 3);
    for (i = 0; i < 10; i++)
        CHECK(fwrite(big, 1000, 1, fp) == 1);
    CHECK(wrhook_count() == 13);
    CHECK(fclose(fp) == 0 && filelength(open("SEQ.BIN", O_RDONLY)) == 20000L + 3 + BUFSIZ + 10000);

    /* Text mode is translated through the buffer. */
    for (i = 0; i < 2000; i++)
        big[i] = (char)(i % 50 == 49 ? '\n' : 'a' + i % 26);
    fp = fopen("SEQ.TXT", "w");
    CHECK(fp != NULL && setvbuf(fp, NULL, _IOFBF, 0) == 0);
    wrhook_watch(fileno(fp));
    CHECK(fwrite(big, 1, 2000, fp) == 2000 && fclose(fp) == 0);
    CHECK(wrhook_count() <= 2040 / (BUFSIZ - 1) + 1);

    wrhook_remove();
    return done();
}
