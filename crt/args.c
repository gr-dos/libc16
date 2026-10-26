/* argc/argv from the command tail in the PSP (decision 8), linked through
 * argv.asm only when main takes parameters.
 *
 * The tail is split as Microsoft C does:
 *   - arguments are separated by spaces and tabs;
 *   - double quotes are removed and toggle quoting, inside which spaces
 *     and tabs are part of the argument;
 *   - 2n backslashes before a double quote give n backslashes and the
 *     quote toggles quoting; 2n + 1 give n backslashes and a literal
 *     double quote; other backslashes are literal.
 *
 * argv[0] is "" (the C standard's value when the program name is not
 * available) and argv[argc] is a null pointer. The strings are copies in
 * DGROUP and may be modified. */

#include "startup.h"

#define TAIL_MAX 127

/* line[0] is argv[0]; the tail is copied to line[1] onward and split in
 * place, which never writes past the input read so far plus one NUL. */
static char line[1 + TAIL_MAX + 1];

/* At most (TAIL_MAX + 1) / 2 arguments, after argv[0] and before NULL. */
static char *args[1 + (TAIL_MAX + 1) / 2 + 1];

void __libc16_args(void)
{
    unsigned char __far *tail = ((__segment)_psp) :> ((unsigned char __near *)0x80);
    unsigned n = tail[0];
    unsigned i, o, slashes;
    int argc = 1;
    int quoted = 0;
    char c;

    if (n > TAIL_MAX)
        n = TAIL_MAX;
    for (i = 1; i <= n && tail[i] != '\r'; i++)
        line[i] = tail[i];
    n = i - 1;

    args[0] = line;
    i = o = 1;
    for (;;) {
        while (i <= n && (line[i] == ' ' || line[i] == '\t'))
            i++;
        if (i > n)
            break;
        args[argc++] = &line[o];
        slashes = 0;
        while (i <= n) {
            c = line[i];
            if (c == '\\') {
                slashes++;
                i++;
                continue;
            }
            if (c == '"') {
                for (; slashes >= 2; slashes -= 2)
                    line[o++] = '\\';
                if (slashes) {
                    line[o++] = '"';
                    slashes = 0;
                } else {
                    quoted = !quoted;
                }
                i++;
                continue;
            }
            for (; slashes; slashes--)
                line[o++] = '\\';
            if (!quoted && (c == ' ' || c == '\t'))
                break;
            line[o++] = c;
            i++;
        }
        for (; slashes; slashes--)
            line[o++] = '\\';
        line[o++] = '\0';
    }

    __libc16_argc = argc;
    __libc16_argv = args;
}
