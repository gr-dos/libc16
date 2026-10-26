/* Calls only free, with a null pointer: links free but none of malloc
 * (tests/heap/members). */

#include <stdlib.h>

int main(void)
{
    free(NULL);
    return 0;
}
