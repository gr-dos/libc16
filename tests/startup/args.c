/* Prints argc, then each argv[i] in brackets, then whether argv[argc] is
 * a null pointer. */

#include "testio.h"

int main(int argc, char **argv)
{
    int i;

    t_putu(argc);
    t_puts("\r\n");
    for (i = 0; i < argc; i++) {
        t_puts("[");
        t_puts(argv[i]);
        t_puts("]\r\n");
    }
    t_puts(argv[argc] ? "not null\r\n" : "null\r\n");
    return 0;
}
