/* Character classes (docs/spec/ctype.md), for every value from EOF to 255 */

#include <ctype.h>
#include <stdio.h>
#include "check.h"

static int in(int c, int lo, int hi)
{
    return c >= lo && c <= hi;
}

int main(void)
{
    int c, bad = 0;

    for (c = EOF; c <= 255; c++) {
        int lower = in(c, 'a', 'z'), upper = in(c, 'A', 'Z'), digit = in(c, '0', '9');
        int alpha = lower || upper;
        int graph = in(c, 0x21, 0x7e);

        bad += !isdigit(c) != !digit;
        bad += !isxdigit(c) != !(digit || in(c, 'a', 'f') || in(c, 'A', 'F'));
        bad += !islower(c) != !lower;
        bad += !isupper(c) != !upper;
        bad += !isalpha(c) != !alpha;
        bad += !isalnum(c) != !(alpha || digit);
        bad += !isspace(c) != !(c == ' ' || in(c, '\t', '\r'));
        bad += !iscntrl(c) != !(in(c, 0, 0x1f) || c == 0x7f);
        bad += !isprint(c) != !in(c, 0x20, 0x7e);
        bad += !isgraph(c) != !graph;
        bad += !ispunct(c) != !(graph && !alpha && !digit);
        bad += tolower(c) != (upper ? c + 32 : c);
        bad += toupper(c) != (lower ? c - 32 : c);
        if (bad) {
            t_puts("wrong at ");
            t_putu((unsigned)c);
            t_puts("\r\n");
            break;
        }
    }
    CHECK(bad == 0);
    return done();
}
