/* Line-buffered standard output, and a write to its handle that does
 * not go through the stream (docs/spec/stdio.md, "Mixing streams and
 * handles"): the write overtakes the stream's pending output unless the
 * stream is flushed first. bufline.expect checks the order. */

#include <io.h>
#include <stdio.h>

int main(void)
{
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0)
        return 1;
    fputs("one\n", stdout);
    write(1, "two\n", 4);
    fputs("four", stdout);
    write(1, "three\n", 6);
    fputs("\n", stdout);
    fputs("five", stdout);
    fflush(stdout);
    write(1, "six\n", 4);
    /* unbuffered again */
    setvbuf(stdout, NULL, _IONBF, 0);
    fputs("seven", stdout);
    write(1, "eight\n", 6);
    return 0;
}
