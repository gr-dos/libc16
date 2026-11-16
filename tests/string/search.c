/* strcspn and strstr (docs/spec/string.md) */

#include <string.h>
#include "check.h"

#if defined(__COMPACT__) || defined(__LARGE__)
/* With far data, strings outside DGROUP */
static char __far far_s[] = "abcabcabd";
static char __far far_f[] = "abd";
#endif

int main(void)
{
    static const char s[] = "hello, world";
    static const char hi[] = "\x80\xff" "a\xfe";

    /* strcspn */
    CHECK(strcspn(s, ",") == 5);
    CHECK(strcspn(s, "dw") == 7);
    CHECK(strcspn(s, "h") == 0);
    CHECK(strcspn(s, "xyz") == 12);
    CHECK(strcspn(s, "") == 12);
    CHECK(strcspn("", "abc") == 0);
    CHECK(strcspn("", "") == 0);
    CHECK(strcspn(s, "zyxwvutsrqponmlkjihgfedcba ,") == 0);
    CHECK(strcspn(hi, "\xfe") == 3);
    CHECK(strcspn(hi, "a\xff") == 1);

    /* strstr */
    CHECK(strstr(s, "world") == s + 7);
    CHECK(strstr(s, "hello, world") == s);
    CHECK(strstr(s, "hello, world!") == NULL);
    CHECK(strstr(s, "o") == s + 4);
    CHECK(strstr(s, "d") == s + 11);
    CHECK(strstr(s, "") == s);
    CHECK(strstr("", "") != NULL && *strstr("", "") == '\0');
    CHECK(strstr("", "a") == NULL);
    CHECK(strstr(s, "World") == NULL);
    CHECK(strstr(s, "lo, w") == s + 3);
    /* a partial match then the real one */
    CHECK(strstr("aaab", "aab") != NULL && strcmp(strstr("aaab", "aab"), "aab") == 0);
    CHECK(strstr("abcabcabd", "abcabd") != NULL && strcmp(strstr("abcabcabd", "abcabd"), "abcabd") == 0);
    CHECK(strstr("ababa", "abac") == NULL);
    /* the match must not run past the end of s */
    CHECK(strstr("abc", "cd") == NULL);
    CHECK(strstr(hi, "\xff" "a") == hi + 1);

#if defined(__COMPACT__) || defined(__LARGE__)
    CHECK(strstr(far_s, far_f) == far_s + 6);
    CHECK(strcspn(far_s, far_f + 2) == 8);
#endif
    return done();
}
