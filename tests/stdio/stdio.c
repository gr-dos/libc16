/* stdio streams (docs/spec/stdio.md) */

#include <errno.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "check.h"

int main(void)
{
    FILE *fp, *fps[FOPEN_MAX];
    char line[16];
    int i;

    CHECK(fputs("to stdout\n", stdout) == 0);
    CHECK(fputc('x', stdout) == 'x' && fputc('\n' + 256, stdout) == '\n');
    CHECK(fputs("", stdout) == 0);
    CHECK(putc('y', stdout) == 'y' && (putc)('\n', stdout) == '\n');
    CHECK(fflush(stdout) == 0 && fflush(NULL) == 0 && !ferror(stdout));
    CHECK(fputs("to stderr\n", stderr) == 0);

    /* write, then read back untranslated */
    fp = fopen("T.TXT", "w");
    CHECK(fp != NULL);
    CHECK(fputs("x\ny", fp) == 0 && fputc('\n', fp) == '\n');
    CHECK(fgetc(fp) == EOF && ferror(fp));
    CHECK(fclose(fp) == 0);
    fp = fopen("T.TXT", "rb");
    CHECK(fp != NULL);
    CHECK(fgetc(fp) == 'x' && getc(fp) == '\r' && (getc)(fp) == '\n');
    CHECK(fgets(line, sizeof line, fp) == line && strcmp(line, "y\r\n") == 0);
    CHECK(!feof(fp) && fgetc(fp) == EOF && feof(fp) && !ferror(fp));
    CHECK(fgetc(fp) == EOF && fgets(line, sizeof line, fp) == NULL);
    CHECK(fputc('z', fp) == EOF && ferror(fp));
    CHECK(fclose(fp) == 0);

    /* text mode, fgets */
    fp = fopen("T.TXT", "a");
    CHECK(fputs("last", fp) == 0 && fclose(fp) == 0);
    fp = fopen("T.TXT", "r");
    strcpy(line, "unchanged");
    CHECK(fgets(line, 2, fp) == line && strcmp(line, "x") == 0);
    CHECK(fgets(line, sizeof line, fp) == line && strcmp(line, "\n") == 0);
    CHECK(fgets(line, sizeof line, fp) == line && strcmp(line, "y\n") == 0);
    CHECK(fgets(line, 1, fp) == line && line[0] == '\0');
    CHECK(fgets(line, sizeof line, fp) == line && strcmp(line, "last") == 0);
    strcpy(line, "unchanged");
    CHECK(fgets(line, sizeof line, fp) == NULL && strcmp(line, "unchanged") == 0);
    CHECK(feof(fp) && !ferror(fp));

    /* ungetc */
    CHECK(ungetc(EOF, fp) == EOF);
    CHECK(ungetc('q', fp) == 'q' && !feof(fp));
    CHECK(ungetc('r', fp) == EOF);
    CHECK(fgetc(fp) == 'q' && fgetc(fp) == EOF && feof(fp));
    CHECK(ungetc(0x1ff, fp) == 0xff && fgetc(fp) == 0xff);
    CHECK(fclose(fp) == 0);

    /* modes and errors */
    CHECK(fopen("NOSUCH.TXT", "r") == NULL && errno == ENOENT);
    errno = 0;
    CHECK(fopen("T.TXT", "r+") == NULL && errno == EINVAL);
    errno = 0;
    CHECK(fopen("T.TXT", "rw") == NULL && errno == EINVAL);
    errno = 0;
    CHECK(fopen("T.TXT", "") == NULL && errno == EINVAL);
    fp = fopen("B.TXT", "wb");
    CHECK(fputs("1\n", fp) == 0 && fclose(fp) == 0);
    fp = fopen("B.TXT", "ab");
    CHECK(fputs("2\n", fp) == 0 && fclose(fp) == 0);
    fp = fopen("B.TXT", "r");
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "1\n") == 0);
    CHECK(fclose(fp) == 0);
    fp = fopen("B.TXT", "rb");
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "1\n") == 0);
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "2\n") == 0);
    CHECK(fclose(fp) == 0);

    /* FOPEN_MAX streams with the standard ones */
    for (i = 0; i < FOPEN_MAX - 3; i++)
        CHECK((fps[i] = fopen("T.TXT", "r")) != NULL);
    CHECK(fopen("T.TXT", "r") == NULL && errno == EMFILE);
    CHECK(fclose(fps[2]) == 0 && (fps[2] = fopen("B.TXT", "r")) != NULL);
    CHECK(fgetc(fps[2]) == '1' && fgetc(fps[1]) == 'x');
    for (i = 0; i < FOPEN_MAX - 3; i++)
        CHECK(fclose(fps[i]) == 0);

    /* a stream follows its handle's mode */
    CHECK(setmode(1, O_BINARY) == O_TEXT);
    CHECK(fputs("binary\r\n", stdout) == 0);
    CHECK(setmode(1, O_TEXT) == O_BINARY);
    return done();
}
