/* getch and putch (docs/spec/dos.md), with standard input redirected
 * from KEYS.TXT: getch must return each byte without echoing it, and
 * putch must write each byte, 0FFh too, to standard output. */

#include <conio.h>
#include "check.h"

int main(void)
{
    CHECK(getch() == 'a');
    CHECK(getch() == 'b');
    CHECK(getch() == 0);
    CHECK(getch() == 0x3b);
    CHECK(getch() == 0xff);
    CHECK(getch() == '\r');

    CHECK(putch('[') == '[');
    CHECK(putch('x') == 'x');
    CHECK(putch(0x1ff) == 0xff);
    CHECK(putch(']') == ']');
    CHECK(putch('\r') == '\r');
    CHECK(putch('\n') == '\n');
    return done();
}
