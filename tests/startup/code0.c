/* The code reserve (docs/spec/startup.md, "Null function pointer"): this
 * module is linked first, and first_code is its first function, so
 * without the reserve it would be at offset 0 of the code segment in the
 * small and compact models, and its pointer would be a null pointer. */

#include "check.h"

void first_code(void)
{
}

int main(void)
{
    void (*volatile f)(void) = first_code;
    int (*volatile m)(void) = main;

    CHECK(f != 0);
    CHECK(m != 0);
#if !defined(__LARGE__)
    CHECK(*(volatile unsigned *)&f >= 2);
    CHECK(*(volatile unsigned *)&m >= 2);
#endif
    first_code();
    return done();
}
