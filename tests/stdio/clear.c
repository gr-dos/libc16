/* clearerr (docs/spec/stdio.md), the function and the macro */

#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <sys/stat.h>
#include "check.h"

int main(void)
{
    FILE *fp, *fps[2];
    int fd, i;

    fd = open("CLEAR.BIN", O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
    CHECK(fd >= 0 && write(fd, "abcd", 4) == 4 && close(fd) == 0);

    fp = fopen("CLEAR.BIN", "rb");
    CHECK(fp != NULL);

    /* the error indicator, keeping the buffer and a pushed-back byte */
    CHECK(fgetc(fp) == 'a' && fgetc(fp) == 'b' && ungetc('Z', fp) == 'Z');
    CHECK(fputc('x', fp) == EOF && ferror(fp) && !feof(fp));
    (clearerr)(fp);
    CHECK(!ferror(fp) && !feof(fp));
    CHECK(fgetc(fp) == 'Z' && fgetc(fp) == 'c');

    /* the end-of-file indicator, with the function */
    CHECK(fgetc(fp) == 'd' && fgetc(fp) == EOF && feof(fp));
    (clearerr)(fp);
    CHECK(!feof(fp) && !ferror(fp));
    CHECK(fgetc(fp) == EOF && feof(fp));

    /* both, with the macro, which evaluates its argument once */
    CHECK(fputc('x', fp) == EOF && ferror(fp) && feof(fp));
    fps[0] = fp;
    fps[1] = NULL;
    i = 0;
    clearerr(fps[i++]);
    CHECK(i == 1 && !feof(fp) && !ferror(fp));

    CHECK(fclose(fp) == 0);
    return done();
}
