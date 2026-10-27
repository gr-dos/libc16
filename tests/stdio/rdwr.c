/* fread and fwrite (docs/spec/stdio.md) */

#include <stdio.h>
#include <string.h>
#include "check.h"

/* 1000 bytes are written; the rest is room for reading back more. */
static char big[1100];

int main(void)
{
    FILE *fp;
    char buf[16];
    unsigned i;

    for (i = 0; i < 1000; i++)
        big[i] = (char)('a' + i % 26);

    fp = fopen("RW.BIN", "wb");
    CHECK(fp != NULL);
    CHECK(fwrite("abcdef", 2, 3, fp) == 3);
    CHECK(fwrite("x", 0, 5, fp) == 0 && fwrite("x", 5, 0, fp) == 0);
    CHECK(fwrite(big, 1, 1000, fp) == 1000);
    CHECK(fread(buf, 1, 1, fp) == 0 && ferror(fp));
    CHECK(fclose(fp) == 0);

    fp = fopen("RW.BIN", "rb");
    CHECK(fp != NULL);
    CHECK(fwrite("x", 1, 1, fp) == 0 && ferror(fp));
    CHECK(fgetc(fp) == 'a' && ungetc('A', fp) == 'A');
    memset(buf, 0, sizeof buf);
    /* the pushed-back byte, then the buffer */
    CHECK(fread(buf, 3, 2, fp) == 2 && memcmp(buf, "Abcdef", 6) == 0);
    memset(big, 0, sizeof big);
    /* beyond the buffer, straight from the file */
    CHECK(fread(big, 10, 100, fp) == 100 && big[0] == 'a' && big[999] == 'l');
    CHECK(!feof(fp) && fgetc(fp) == EOF && feof(fp));
    CHECK(fread(buf, 1, 4, fp) == 0);
    CHECK(fclose(fp) == 0);

    /* a partial element at the end is stored but not counted */
    fp = fopen("RW.BIN", "rb");
    CHECK(fp != NULL && fread(big, 1, 1003, fp) == 1003 && big[1002] == 'i');
    CHECK(fclose(fp) == 0);
    fp = fopen("RW.BIN", "rb");
    memset(big, 0, sizeof big);
    CHECK(fp != NULL && fread(big, 1000, 2, fp) == 1 && feof(fp) && big[1005] == 'l');
    CHECK(fclose(fp) == 0);

    /* text mode translates in both directions */
    fp = fopen("RW.TXT", "w");
    CHECK(fp != NULL && fwrite("a\nb\n", 1, 4, fp) == 4 && fclose(fp) == 0);
    fp = fopen("RW.TXT", "rb");
    CHECK(fp != NULL && fread(buf, 1, sizeof buf, fp) == 6 && memcmp(buf, "a\r\nb\r\n", 6) == 0);
    CHECK(feof(fp) && fclose(fp) == 0);
    fp = fopen("RW.TXT", "r");
    CHECK(fp != NULL && fread(buf, 2, 3, fp) == 2 && memcmp(buf, "a\nb\n", 4) == 0);
    CHECK(fclose(fp) == 0);

    CHECK(fwrite("to stdout\n", 1, 10, stdout) == 10);
    return done();
}
