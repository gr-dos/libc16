/* setvbuf and setbuf on an update stream: treated as a read stream, so
 * output stays unbuffered and a read may follow a write directly
 * (docs/spec/stdio.md, "Update streams" and "Buffering") */

#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "check.h"

static char mine[64];

int main(void)
{
    FILE *fp;
    char line[16];
    unsigned i;

    fp = fopen("UB.TXT", "w+");
    CHECK(fp != NULL);
    /* _IOFBF: 0, nothing changes (no buffer is allocated) */
    CHECK(setvbuf(fp, NULL, _IOFBF, 0) == 0);
    CHECK(setvbuf(fp, mine, _IOFBF, sizeof mine) == 0);
    /* _IOLBF and _IONBF cannot be honoured on input */
    errno = 0;
    CHECK(setvbuf(fp, NULL, _IOLBF, 0) == -1 && errno == EINVAL);
    errno = 0;
    CHECK(setvbuf(fp, NULL, _IONBF, 0) == -1 && errno == EINVAL);
    setbuf(fp, mine);
    setbuf(fp, NULL);
    /* still unbuffered output: the write is in the file at once */
    CHECK(fputs("one\ntwo\n", fp) >= 0);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0);
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "one\n") == 0);
    CHECK(fseek(fp, 0L, SEEK_CUR) == 0 && fputs("TWO\n", fp) >= 0);
    CHECK(fseek(fp, 0L, SEEK_SET) == 0);
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "one\n") == 0);
    CHECK(fgets(line, sizeof line, fp) != NULL && strcmp(line, "TWO\n") == 0);
    /* nothing of mine was used as a buffer */
    for (i = 0; i < sizeof mine; i++)
        CHECK(mine[i] == 0);
    CHECK(fclose(fp) == 0);
    CHECK(unlink("UB.TXT") == 0);
    return done();
}
