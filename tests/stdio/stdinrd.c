/* Copies stdin to stdout a line at a time, in text mode, marking each
 * line (docs/spec/stdio.md). */

#include <stdio.h>

int main(void)
{
    char line[8];

    while (fgets(line, sizeof line, stdin) != NULL) {
        fputs("[", stdout);
        fputs(line, stdout);
        fputs("]", stdout);
    }
    fputs(feof(stdin) && !ferror(stdin) ? "\neof\n" : "\nerror\n", stdout);
    return 0;
}
