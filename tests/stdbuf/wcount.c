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
unsigned __cdecl wrhook_size(unsigned i);

#define LINES 2000
#define NONE  (-1)      /* unbuffered: no setvbuf */

static char big[20000];
static char a[512], b[512];
static char sbuf[BUFSIZ];

/* The sizes of the writes since wrhook_watch are those of want[], n of
 * them (at most 128, all the hook records). */
static int sizes(const unsigned *want, unsigned n)
{
    unsigned i;

    if (wrhook_count() != n)
        return 0;
    for (i = 0; i < n; i++)
        if (wrhook_size(i) != want[i])
            return 0;
    return 1;
}

/* Every write since wrhook_watch but the last is a multiple of align
 * bytes, so each starts at a multiple of align in a file written from
 * its start, and together they are total bytes. */
static int aligned(unsigned align, long total)
{
    unsigned i, n = wrhook_count();
    long sum = 0;

    if (n == 0 || n > 128)
        return 0;
    for (i = 0; i < n; i++) {
        if (i < n - 1 && wrhook_size(i) % align != 0)
            return 0;
        sum += wrhook_size(i);
    }
    return sum == total;
}

/* pieces calls of 1016 bytes, a 1 KiB frame's payload, to a binary file
 * from its start with the buffer set by setbuf (sbuf) or setvbuf (a size
 * of size, from malloc); the file holds them all. */
static void frames(const char *name, unsigned size, unsigned pieces)
{
    FILE *fp = fopen(name, "wb");
    unsigned i;
    int fd;

    CHECK(fp != NULL);
    if (size == 0)
        setbuf(fp, sbuf);
    else
        CHECK(setvbuf(fp, NULL, _IOFBF, size) == 0);
    wrhook_watch(fileno(fp));
    for (i = 0; i < pieces; i++)
        CHECK(fwrite(big + i, 1, 1016, fp) == 1016);
    CHECK(fclose(fp) == 0);
    CHECK(aligned(size == 0 ? BUFSIZ : size, 1016L * pieces));
    fd = open(name, O_RDONLY | O_BINARY);
    CHECK(fd >= 0 && filelength(fd) == 1016L * pieces);
    for (i = 0; i < pieces; i++)
        CHECK(read(fd, a, 508) == 508 && read(fd, b, 508) == 508
              && memcmp(a, big + i, 508) == 0 && memcmp(b, big + i + 508, 508) == 0);
    close(fd);
}

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

    /* Binary output keeps the writes whole buffers: pending output is
     * completed from the request and written, then the largest multiple
     * of the buffer's size of what is left, straight from the request in
     * one write, and the rest is kept. */
    for (i = 0; i < sizeof big; i++)
        big[i] = (char)i;
    fp = fopen("SEQ.BIN", "wb");
    CHECK(fp != NULL && setvbuf(fp, NULL, _IOFBF, 0) == 0);
    wrhook_watch(fileno(fp));
    {
        /* 20,000 is 39 buffers and 32 bytes; 3 more; then 512 completes
         * the buffer and leaves 35; each 1,000 then makes two writes of
         * 512 or one, and fclose writes the last 307. */
        static const unsigned want[] = {
            19968, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512,
            512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 307
        };

        CHECK(fwrite(big, 1, sizeof big, fp) == sizeof big && sizes(want, 1));
        CHECK(fwrite(big, 1, 3, fp) == 3 && sizes(want, 1));
        CHECK(fwrite(big, 1, BUFSIZ, fp) == BUFSIZ && sizes(want, 2));
        for (i = 0; i < 10; i++)
            CHECK(fwrite(big, 1000, 1, fp) == 1);
        CHECK(sizes(want, 21));
        CHECK(fclose(fp) == 0 && sizes(want, 22));
    }
    CHECK(filelength(open("SEQ.BIN", O_RDONLY)) == 20000L + 3 + BUFSIZ + 10000);
    /* A buffer whose size is not a power of 2: 3,000 is three buffers of
     * 1,000, in one write */
    fp = fopen("SEQ3.BIN", "wb");
    CHECK(fp != NULL && setvbuf(fp, NULL, _IOFBF, 1000) == 0);
    wrhook_watch(fileno(fp));
    CHECK(fwrite(big, 1, 10, fp) == 10 && wrhook_count() == 0);
    CHECK(fwrite(big, 1, 3000, fp) == 3000 && wrhook_count() == 2);
    CHECK(wrhook_size(0) == 1000 && wrhook_size(1) == 2000);
    CHECK(fclose(fp) == 0 && wrhook_count() == 3 && wrhook_size(2) == 10);
    /* gr-dos/libc16#45: 1,016-byte pieces through setbuf's 512 bytes,
     * and through 4,096, reach DOS as whole buffers, but for the last */
    frames("FR512.BIN", 0, 30);
    CHECK(wrhook_count() == 60);
    frames("FR4K.BIN", 4096, 30);
    CHECK(wrhook_count() == 8);
    /* A line-buffered binary stream does the same, and writes what is
     * left at a \n */
    fp = fopen("SEQL.BIN", "wb");
    CHECK(fp != NULL && setvbuf(fp, NULL, _IOLBF, 16) == 0);
    wrhook_watch(fileno(fp));
    CHECK(fwrite("0123456789abcdefghij", 1, 20, fp) == 20 && wrhook_count() == 1
          && wrhook_size(0) == 16);
    CHECK(fwrite("0123456789abcdefghi\n", 1, 20, fp) == 20 && wrhook_count() == 3
          && wrhook_size(1) == 16 && wrhook_size(2) == 8);
    CHECK(fclose(fp) == 0 && wrhook_count() == 3);

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
