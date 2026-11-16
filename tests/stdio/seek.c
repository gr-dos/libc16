/* fseek and ftell (docs/spec/stdio.md, "File positions") */

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "check.h"

/* tests/io/seekfail.asm: make INT 21h function 42h calls fail with DOS
 * error 6 (EBADF) */
void __cdecl seekfail_install(void);
void __cdecl seekfail_remove(void);
void __cdecl seekfail(int origin, unsigned skip);
void __cdecl seekfail_after(unsigned skip);
unsigned __cdecl seekfail_count(void);

#define SIZE 600
#define CTLZ 0x1a

static unsigned char raw[SIZE];
/* The forward model of a text file: ch[k] is the k-th byte a text-mode
 * read returns, and pos[k] the file offset before it; pos[nch] is the
 * position at the end (the size, or the offset of a ^Z). */
static unsigned char ch[SIZE];
static long pos[SIZE + 1];
static unsigned nch;

/* For checks repeated at each position: reports the first failure only,
 * with the position's index. */
static unsigned bad;

static void fail_at(const char *what, unsigned k, unsigned line)
{
    if (bad++ == 0) {
        t_puts(what);
        t_puts(" at index ");
        t_putu(k);
        t_puts(": ");
    }
    if (bad == 1)
        check(0, line);
}

#define AT(cond, what, k) do { if (!(cond)) fail_at(what, k, __LINE__); } while (0)

static void write_raw(const char *name, unsigned n)
{
    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);

    CHECK(fd >= 0 && write(fd, raw, n) == (int)n && close(fd) == 0);
}

/* Fills raw with letters, \r\n, lone \r, \r\r\n and lone \n, from a seed. */
static void generate(unsigned seed)
{
    unsigned i = 0, r;

    while (i < SIZE) {
        seed = seed * 25173u + 13849u;
        r = (seed >> 8) % 10;
        if (r == 0 && i + 2 <= SIZE) {
            raw[i++] = '\r';
            raw[i++] = '\n';
        } else if (r == 1) {
            raw[i++] = '\r';
        } else if (r == 2 && i + 3 <= SIZE) {
            raw[i++] = '\r';
            raw[i++] = '\r';
            raw[i++] = '\n';
        } else if (r == 3) {
            raw[i++] = '\n';
        } else {
            raw[i++] = (unsigned char)('a' + (seed >> 4) % 26);
        }
    }
}

/* The forward model, from docs/spec/io.md's text-mode rule. */
static void model(unsigned n)
{
    unsigned i = 0;

    nch = 0;
    while (i < n && raw[i] != CTLZ) {
        pos[nch] = i;
        if (raw[i] == '\r' && i + 1 < n && raw[i + 1] == '\n') {
            ch[nch++] = '\n';
            i += 2;
        } else {
            ch[nch++] = raw[i++];
        }
    }
    pos[nch] = i;
}

static void text_file(const char *name, unsigned n)
{
    FILE *fp;
    unsigned k;
    int c;

    write_raw(name, n);
    model(n);

    /* ftell before every byte, reading forward */
    fp = fopen(name, "r");
    CHECK(fp != NULL);
    bad = 0;
    for (k = 0; k < nch; k++) {
        AT(ftell(fp) == pos[k], "forward ftell", k);
        c = fgetc(fp);
        AT(c == ch[k], "forward fgetc", k);
    }
    AT(ftell(fp) == pos[nch], "ftell at end", nch);
    AT(fgetc(fp) == EOF && feof(fp), "EOF", nch);
    AT(ftell(fp) == pos[nch], "ftell after EOF", nch);
    CHECK(fclose(fp) == 0);

    /* a pushed-back byte moves the position back one byte */
    fp = fopen(name, "r");
    CHECK(fp != NULL);
    for (k = 0; k < nch; k++) {
        c = fgetc(fp);
        AT(c == ch[k], "fgetc", k);
        AT(ungetc('Q', fp) == 'Q', "ungetc", k);
        AT(ftell(fp) == pos[k + 1] - 1, "ftell after ungetc", k);
        AT(fgetc(fp) == 'Q', "pushed-back byte", k);
        AT(ftell(fp) == pos[k + 1], "ftell after pushed-back byte", k);
    }
    CHECK(fclose(fp) == 0);

    /* back to every position, last first, with SEEK_SET and SEEK_CUR */
    fp = fopen(name, "r");
    CHECK(fp != NULL);
    for (k = nch; k-- > 0;) {
        AT(fseek(fp, pos[k], SEEK_SET) == 0, "fseek SEEK_SET", k);
        AT(ftell(fp) == pos[k], "ftell after SEEK_SET", k);
        AT(fgetc(fp) == ch[k], "fgetc after SEEK_SET", k);
        AT(ftell(fp) == pos[k + 1], "ftell after SEEK_SET and fgetc", k);
        AT(fseek(fp, pos[k] - pos[k + 1], SEEK_CUR) == 0, "fseek SEEK_CUR", k);
        AT(fgetc(fp) == ch[k], "fgetc after SEEK_CUR", k);
        /* one byte back reads the same byte again (docs/spec/stdio.md) */
        AT(fseek(fp, pos[k + 1] - 1, SEEK_SET) == 0 && fgetc(fp) == ch[k], "one byte back", k);
        if (k % 7 == 0)
            AT(fseek(fp, pos[k] - (long)n, SEEK_END) == 0 && fgetc(fp) == ch[k],
                "fseek SEEK_END", k);
    }
    CHECK(fclose(fp) == 0);
    CHECK(bad == 0);
}

static void text_files(void)
{
    /* a \r\n across the end of the first 128-byte read */
    generate(1);
    raw[126] = 'a';
    raw[127] = '\r';
    raw[128] = '\n';
    text_file("T1.TXT", SIZE);

    /* a lone \r there, whose next byte read keeps */
    generate(2);
    raw[126] = 'a';
    raw[127] = '\r';
    raw[128] = 'x';
    text_file("T2.TXT", SIZE);

    /* a lone \r there, followed by \r\n */
    generate(3);
    raw[126] = 'a';
    raw[127] = '\r';
    raw[128] = '\r';
    raw[129] = '\n';
    text_file("T3.TXT", SIZE);

    /* a \r as the last byte of the file */
    generate(4);
    raw[SIZE - 2] = 'a';
    raw[SIZE - 1] = '\r';
    text_file("T4.TXT", SIZE);

    /* a ^Z ends the text */
    generate(5);
    raw[300] = CTLZ;
    text_file("T5.TXT", SIZE);
}

static void binary_file(void)
{
    FILE *fp;
    unsigned k;
    char buf[64];

    for (k = 0; k < SIZE; k++)
        raw[k] = (unsigned char)(k * 7 + k / 256);
    write_raw("B.BIN", SIZE);

    fp = fopen("B.BIN", "rb");
    CHECK(fp != NULL);

    /* a pushed-back byte at position 0 has no position */
    CHECK(ungetc('a', fp) == 'a');
    errno = 0;
    CHECK(ftell(fp) == -1L && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, 0L, SEEK_CUR) != 0 && errno == EINVAL);
    CHECK(fgetc(fp) == 'a' && ftell(fp) == 0);

    bad = 0;
    for (k = 0; k < SIZE; k++) {
        AT(ftell(fp) == (long)k, "binary ftell", k);
        AT(fgetc(fp) == raw[k], "binary fgetc", k);
    }
    CHECK(bad == 0);
    CHECK(fgetc(fp) == EOF && feof(fp) && ftell(fp) == SIZE);

    /* each origin; SEEK_SET clears the end-of-file indicator */
    CHECK(fseek(fp, 10L, SEEK_SET) == 0 && !feof(fp) && ftell(fp) == 10);
    CHECK(fgetc(fp) == raw[10]);
    CHECK(fseek(fp, 5L, SEEK_CUR) == 0 && ftell(fp) == 16 && fgetc(fp) == raw[16]);
    CHECK(fseek(fp, -3L, SEEK_END) == 0 && ftell(fp) == SIZE - 3 && fgetc(fp) == raw[SIZE - 3]);
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && ftell(fp) == SIZE && fgetc(fp) == EOF && feof(fp));
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && !feof(fp) && fgetc(fp) == raw[0]);

    /* past the end, and past 64 KiB */
    CHECK(fseek(fp, SIZE + 50L, SEEK_SET) == 0 && ftell(fp) == SIZE + 50L);
    CHECK(fgetc(fp) == EOF && feof(fp) && ftell(fp) == SIZE + 50L);
    CHECK(fseek(fp, 70000L, SEEK_SET) == 0 && ftell(fp) == 70000L && !feof(fp));
    CHECK(fseek(fp, -69990L, SEEK_CUR) == 0 && ftell(fp) == 10 && fgetc(fp) == raw[10]);

    /* fread after fseek, through more than one buffer */
    CHECK(fseek(fp, 100L, SEEK_SET) == 0);
    CHECK(fread(buf, 1, 20, fp) == 20 && memcmp(buf, raw + 100, 20) == 0 && ftell(fp) == 120);
    CHECK(fseek(fp, 250L, SEEK_SET) == 0);
    for (k = 0; k < 5; k++)
        CHECK(fread(buf, 1, 64, fp) == 64 && memcmp(buf, raw + 250 + 64 * k, 64) == 0);
    CHECK(ftell(fp) == 570);

    /* a pushed-back byte: SEEK_CUR counts from one byte back, and fseek
     * discards it */
    CHECK(fseek(fp, 5L, SEEK_SET) == 0 && fgetc(fp) == raw[5]);
    CHECK(ungetc('Q', fp) == 'Q' && ftell(fp) == 5);
    CHECK(fseek(fp, 2L, SEEK_CUR) == 0 && ftell(fp) == 7 && fgetc(fp) == raw[7]);
    CHECK(ungetc('Q', fp) == 'Q' && fseek(fp, 0L, SEEK_CUR) == 0);
    CHECK(ftell(fp) == 7 && fgetc(fp) == raw[7]);

    /* a bad origin or a target before the start changes nothing */
    CHECK(fseek(fp, 0L, SEEK_SET) == 0);
    CHECK(fread(buf, 1, 4, fp) == 4 && ungetc('Z', fp) == 'Z');
    errno = 0;
    CHECK(fseek(fp, -1L, SEEK_SET) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, -100L, SEEK_CUR) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, -(SIZE + 1L), SEEK_END) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, 0L, 3) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, 0L, -1) == -1 && errno == EINVAL);
    CHECK(ftell(fp) == 3 && fgetc(fp) == 'Z' && fgetc(fp) == raw[4] && ftell(fp) == 5);

    /* and leaves the end-of-file indicator */
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && fgetc(fp) == EOF && feof(fp));
    CHECK(fseek(fp, -1L, SEEK_SET) == -1 && feof(fp));

    /* fseek does not clear the error indicator */
    CHECK(fputc('x', fp) == EOF && ferror(fp));
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && ferror(fp) && !feof(fp) && fgetc(fp) == raw[0]);
    CHECK(fclose(fp) == 0);
}

/* 1 if name holds exactly n bytes, the first of them text */
static int file_is(const char *name, const char *text, unsigned n)
{
    char buf[32];
    int fd, got;

    fd = open(name, O_RDONLY | O_BINARY);
    if (fd < 0)
        return 0;
    got = read(fd, buf, sizeof buf);
    close(fd);
    return got == (int)n && memcmp(buf, text, strlen(text)) == 0;
}

static void write_streams(void)
{
    FILE *fp;
    char buf[8];
    int fd;

    /* binary */
    fp = fopen("W.BIN", "wb");
    CHECK(fp != NULL && ftell(fp) == 0);
    CHECK(fputs("hello", fp) == 0 && ftell(fp) == 5);
    CHECK(fseek(fp, 1L, SEEK_SET) == 0 && ftell(fp) == 1);
    CHECK(fputc('E', fp) == 'E' && ftell(fp) == 2);
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && ftell(fp) == 5);
    CHECK(fseek(fp, 2L, SEEK_CUR) == 0 && ftell(fp) == 7);
    CHECK(fputc('!', fp) == '!' && ftell(fp) == 8);
    errno = 0;
    CHECK(fseek(fp, 0L, 3) == -1 && errno == EINVAL && ftell(fp) == 8);
    errno = 0;
    CHECK(fseek(fp, -9L, SEEK_CUR) == -1 && errno == EINVAL && ftell(fp) == 8);
    CHECK(fclose(fp) == 0);
    CHECK(file_is("W.BIN", "hEllo", 8));
    fd = open("W.BIN", O_RDONLY | O_BINARY);
    CHECK(fd >= 0 && lseek(fd, 7L, SEEK_SET) == 7 && read(fd, buf, 1) == 1 && buf[0] == '!');
    CHECK(close(fd) == 0);

    /* text: positions count the \r of each \r\n */
    fp = fopen("W.TXT", "w");
    CHECK(fp != NULL && fputs("a\nb\n", fp) == 0 && ftell(fp) == 6);
    CHECK(fseek(fp, 3L, SEEK_SET) == 0 && fputc('B', fp) == 'B' && ftell(fp) == 4);
    CHECK(fclose(fp) == 0);
    CHECK(file_is("W.TXT", "a\r\nB\r\n", 6));

    /* append: writes still go to the end */
    fp = fopen("W.TXT", "ab");
    CHECK(fp != NULL && fputc('z', fp) == 'z' && ftell(fp) == 7);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && ftell(fp) == 0);
    CHECK(fputc('y', fp) == 'y' && ftell(fp) == 8);
    CHECK(fclose(fp) == 0);
    CHECK(file_is("W.TXT", "a\r\nB\r\nzy", 8));
}

/* Seek errors (docs/spec/stdio.md): a text stream with a buffer, a byte
 * kept by read and a pushed-back byte, after 10 bytes are read. */
#define READ 10
#define CALLS 6
/* text-mode ftell may seek once for each \n in the buffer */
#define MAXSKIP 300

static FILE *setup(void)
{
    FILE *fp = fopen("E.TXT", "r");
    unsigned k;

    if (fp == NULL)
        return NULL;
    for (k = 0; k < READ; k++)
        fgetc(fp);
    ungetc('Q', fp);
    return fp;
}

/* Exactly as setup left it: the position, the pushed-back byte, then the
 * rest of the file, the buffer and the kept byte included. */
static int unchanged(FILE *fp)
{
    unsigned k;

    if (feof(fp) || ferror(fp) || ftell(fp) != pos[READ] - 1 || fgetc(fp) != 'Q')
        return 0;
    for (k = READ; k < nch; k++)
        if (fgetc(fp) != ch[k])
            return 0;
    return fgetc(fp) == EOF;
}

/* Call i: four fseeks with targets in the file, one with a target before
 * the start that only SEEK_END's DOS calls reveal, and ftell. */
static long call(FILE *fp, unsigned i)
{
    switch (i) {
    case 0: return fseek(fp, 5L, SEEK_SET);
    case 1: return fseek(fp, -3L, SEEK_CUR);
    case 2: return fseek(fp, -10L, SEEK_END);
    case 3: return fseek(fp, 0L, SEEK_END);
    case 4: return fseek(fp, -(SIZE + 1L), SEEK_END);
    default: return ftell(fp);
    }
}

/* What call i returns with no DOS error */
static long success(unsigned i)
{
    return i == 4 ? -1L : i == 5 ? pos[READ] - 1 : 0L;
}

static void seek_errors(void)
{
    FILE *fp;
    unsigned i, skip, n;
    long r;

    generate(2);
    raw[126] = 'a';
    raw[127] = '\r';
    raw[128] = 'x';
    write_raw("E.TXT", SIZE);
    model(SIZE);

    seekfail_install();
    /* One DOS seek fails, the first, the second and so on: the handle can
     * always be moved back, so the stream is unchanged. */
    for (i = 0; i < CALLS; i++) {
        for (skip = 0; skip < MAXSKIP; skip++) {
            fp = setup();
            CHECK(fp != NULL);
            seekfail(-2, skip);
            errno = 0;
            r = call(fp, i);
            n = seekfail_count();
            seekfail(-1, 0);
            if (n == 0) {
                /* past the call's last seek */
                CHECK(r == success(i));
                if (i == 4)
                    CHECK(errno == EINVAL && unchanged(fp));
                if (i == 5)
                    CHECK(unchanged(fp));
                CHECK(fclose(fp) == 0);
                break;
            }
            AT(r == -1L && errno == EBADF, "one seek error, result", i * 100 + skip);
            AT(unchanged(fp), "one seek error, stream", i * 100 + skip);
            CHECK(fclose(fp) == 0);
        }
        CHECK(skip < MAXSKIP);
    }

    /* Every DOS seek fails from the first, the second and so on, so moving
     * the handle back may fail too: never success, and a later fseek
     * works. */
    for (i = 0; i < CALLS; i++) {
        for (skip = 0; skip < MAXSKIP; skip++) {
            fp = setup();
            CHECK(fp != NULL);
            seekfail_after(skip);
            errno = 0;
            r = call(fp, i);
            n = seekfail_count();
            seekfail(-1, 0);
            if (n == 0) {
                CHECK(r == success(i));
                CHECK(fclose(fp) == 0);
                break;
            }
            AT(r == -1L && errno == EBADF, "seek errors, result", i * 100 + skip);
            AT(!feof(fp) && !ferror(fp), "seek errors, indicators", i * 100 + skip);
            AT(fseek(fp, 0L, SEEK_SET) == 0 && fgetc(fp) == ch[0] && ftell(fp) == pos[1],
                "seek errors, then fseek", i * 100 + skip);
            CHECK(fclose(fp) == 0);
        }
        CHECK(skip < MAXSKIP);
    }
    seekfail_remove();
}

int main(void)
{
    text_files();
    binary_file();
    write_streams();
    bad = 0;
    seek_errors();
    CHECK(bad == 0);
    return done();
}
