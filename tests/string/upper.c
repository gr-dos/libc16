/* strupr (docs/spec/string.md) */

#include <string.h>
#include "check.h"

int main(void)
{
    static char s[] = "Hello, World! az AZ @[`{ 09";
    static char hi[] = { 'a', (char)0x80, (char)0xe0, (char)0xff, 'z', '\0' };
    static char empty[] = "";

    CHECK(strupr(s) == s);
    CHECK(strcmp(s, "HELLO, WORLD! AZ AZ @[`{ 09") == 0);
    CHECK(strupr(hi) == hi);
    CHECK((unsigned char)hi[0] == 'A' && (unsigned char)hi[1] == 0x80
          && (unsigned char)hi[2] == 0xe0 && (unsigned char)hi[3] == 0xff
          && (unsigned char)hi[4] == 'Z' && hi[5] == '\0');
    CHECK(strupr(empty) == empty && empty[0] == '\0');
    /* back with strlwr */
    CHECK(strcmp(strlwr(s), "hello, world! az az @[`{ 09") == 0);
    return done();
}
