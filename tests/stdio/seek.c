/* fseek and ftell (docs/spec/stdio.md) */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "check.h"

#define RAWLEN 600

static unsigned char raw[RAWLEN];
/* The bytes a text-mode read returns, and the file offset each came from */
static unsigned char chars[RAWLEN];
static long at[RAWLEN + 1];
static unsigned nchars;

/* RAWLEN bytes with \r\n, lone \r and \r\r\n throughout, and at offset
 * 127 (the end of the first 128-byte read) either a \r\n split across
 * reads (split != 0) or a \r followed by another byte. */
static void make_text(const char *name, int split)
{
    FILE *fp;
    unsigned i;

    for (i = 0; i < RAWLEN; i++)
        raw[i] = (unsigned char)('a' + i % 23);
    for (i = 5; i + 2 < RAWLEN; i += 17) {
        raw[i] = '\r';
        raw[i + 1] = '\n';
    }
    for (i = 11; i < RAWLEN; i += 41)
        raw[i] = '\r';
    for (i = 30; i + 3 < RAWLEN; i += 97) {
        raw[i] = '\r';
        raw[i + 1] = '\r';
        raw[i + 2] = '\n';
    }
    raw[126] = 'q';
    raw[127] = '\r';
    raw[128] = split ? '\n' : 'x';
    raw[129] = 'y';

    fp = fopen(name, "wb");
    CHECK(fp != NULL && fwrite(raw, 1, RAWLEN, fp) == RAWLEN && fclose(fp) == 0);

    /* What text-mode reading makes of it: each \r\n becomes \n. */
    nchars = 0;
    for (i = 0; i < RAWLEN; ) {
        at[nchars] = i;
        if (raw[i] == '\r' && i + 1 < RAWLEN && raw[i + 1] == '\n') {
            chars[nchars++] = '\n';
            i += 2;
        } else
            chars[nchars++] = raw[i++];
    }
    at[nchars] = RAWLEN;
}

static void text_positions(const char *name)
{
    FILE *fp = fopen(name, "r");
    unsigned k;
    int c;

    CHECK(fp != NULL);
    /* before every byte, through all the buffer refills */
    for (k = 0; k <= nchars; k++) {
        if (ftell(fp) != at[k]) {
            CHECK(0);
            break;
        }
        c = fgetc(fp);
        if (k < nchars && c != chars[k]) {
            CHECK(0);
            break;
        }
    }
    CHECK(c == EOF && feof(fp));

    /* back to every position, reading the same byte again */
    for (k = 0; k < nchars; k++)
        if (fseek(fp, at[k], SEEK_SET) != 0 || fgetc(fp) != chars[k]) {
            CHECK(0);
            break;
        }

    /* ungetc of a different byte: one byte, or one \r\n, earlier */
    for (k = 1; k + 1 < nchars; k += 7) {
        CHECK(fseek(fp, at[k], SEEK_SET) == 0 && fgetc(fp) == chars[k]);
        CHECK(ungetc('Z', fp) == 'Z' && ftell(fp) == at[k]);
        CHECK(fgetc(fp) == 'Z' && ftell(fp) == at[k + 1]);
        CHECK(fgetc(fp) == chars[k + 1]);
    }
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && ungetc('Z', fp) == 'Z');
    errno = 0;
    CHECK(ftell(fp) == -1L && errno == EINVAL);
    CHECK(fgetc(fp) == 'Z' && fgetc(fp) == chars[0]);
    CHECK(fclose(fp) == 0);
}

int main(void)
{
    FILE *fp;
    unsigned i;
    int c;

    make_text("SEEK1.TXT", 1);
    CHECK(raw[127] == '\r' && raw[128] == '\n');
    text_positions("SEEK1.TXT");
    make_text("SEEK2.TXT", 0);
    CHECK(raw[127] == '\r' && raw[128] == 'x');
    text_positions("SEEK2.TXT");

    /* binary: the same file, byte for byte */
    fp = fopen("SEEK2.TXT", "rb");
    CHECK(fp != NULL && ftell(fp) == 0);
    for (i = 0; i < RAWLEN; i++)
        if (ftell(fp) != (long)i || fgetc(fp) != raw[i]) {
            CHECK(0);
            break;
        }
    CHECK(ftell(fp) == RAWLEN && fgetc(fp) == EOF && feof(fp));

    /* each origin */
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && !feof(fp) && fgetc(fp) == raw[0]);
    for (i = 0; i < 9; i++)
        fgetc(fp);
    CHECK(fseek(fp, 5L, SEEK_CUR) == 0 && ftell(fp) == 15 && fgetc(fp) == raw[15]);
    CHECK(fseek(fp, -6L, SEEK_CUR) == 0 && fgetc(fp) == raw[10]);
    CHECK(fseek(fp, -1L, SEEK_END) == 0 && ftell(fp) == RAWLEN - 1);
    CHECK(fgetc(fp) == raw[RAWLEN - 1] && fgetc(fp) == EOF);
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && fgetc(fp) == EOF && feof(fp));

    /* past the end */
    CHECK(fseek(fp, 1000L, SEEK_SET) == 0 && ftell(fp) == 1000 && fgetc(fp) == EOF);

    /* before the start, and a bad origin, change nothing */
    CHECK(fseek(fp, 0L, SEEK_SET) == 0);
    for (i = 0; i < 3; i++)
        fgetc(fp);
    errno = 0;
    CHECK(fseek(fp, -10L, SEEK_CUR) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, -1L, SEEK_SET) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, -(long)RAWLEN - 1, SEEK_END) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(fseek(fp, 0L, 3) == -1 && errno == EINVAL);
    CHECK(ftell(fp) == 3 && fgetc(fp) == raw[3]);

    /* fseek discards a pushed-back byte */
    CHECK(ungetc('Q', fp) == 'Q' && ftell(fp) == 3);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && fgetc(fp) == raw[0]);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && ungetc('Q', fp) == 'Q');
    errno = 0;
    CHECK(ftell(fp) == -1L && errno == EINVAL);

    /* the error indicator stays */
    CHECK(fputc('x', fp) == EOF && ferror(fp));
    CHECK(fseek(fp, 0L, SEEK_SET) == 0 && ferror(fp));
    CHECK(fclose(fp) == 0);

    /* streams opened for writing */
    fp = fopen("SEEK3.BIN", "wb");
    CHECK(fp != NULL && ftell(fp) == 0);
    CHECK(fwrite("0123456789", 1, 10, fp) == 10 && ftell(fp) == 10);
    CHECK(fseek(fp, 2L, SEEK_SET) == 0 && fputc('X', fp) == 'X' && ftell(fp) == 3);
    CHECK(fseek(fp, 0L, SEEK_END) == 0 && fputc('E', fp) == 'E' && ftell(fp) == 11);
    CHECK(fclose(fp) == 0);
    fp = fopen("SEEK3.BIN", "rb");
    CHECK(fp != NULL && fread(chars, 1, 20, fp) == 11 && memcmp(chars, "01X3456789E", 11) == 0);
    CHECK(fclose(fp) == 0);
    fp = fopen("SEEK3.TXT", "w");
    CHECK(fp != NULL && fputs("a\nb", fp) == 0 && ftell(fp) == 4);
    CHECK(fclose(fp) == 0);
    fp = fopen("SEEK3.TXT", "r");
    CHECK(fp != NULL && fgetc(fp) == 'a' && fgetc(fp) == '\n' && ftell(fp) == 3);
    c = fgetc(fp);
    CHECK(c == 'b' && ftell(fp) == 4 && fclose(fp) == 0);

    return done();
}
