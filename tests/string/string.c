/* Strings and memory (docs/spec/string.md) */

#include <errno.h>
#include <string.h>
#include "check.h"

/* Outside DGROUP in every model */
static char __far fbuf[16];

static int sign(int v)
{
    return v < 0 ? -1 : v > 0;
}

int main(void)
{
    char buf[32];
    static const char hi[] = "a\x80";
    const char *s = "hello";

    /* comparisons are unsigned */
    CHECK(memcmp("abc", "abc", 3) == 0);
    CHECK(sign(memcmp("abc", "abd", 3)) == -1);
    CHECK(sign(memcmp("\x80", "\x01", 1)) == 1);
    CHECK(memcmp("a", "b", 0) == 0);
    CHECK(strcmp("", "") == 0);
    CHECK(strcmp("abc", "abc") == 0);
    CHECK(sign(strcmp("ab", "abc")) == -1);
    CHECK(sign(strcmp("abc", "ab")) == 1);
    CHECK(sign(strcmp(hi, "a\x01")) == 1);
    CHECK(strncmp("abcx", "abcy", 3) == 0);
    CHECK(sign(strncmp("abcx", "abcy", 4)) == -1);
    CHECK(strncmp("ab", "ab", 10) == 0);
    CHECK(sign(strncmp("ab", "abc", 10)) == -1);
    CHECK(strncmp("x", "y", 0) == 0);
    CHECK(stricmp("HeLLo", "hEllO") == 0);
    CHECK(sign(stricmp("abc", "ABD")) == -1);
    CHECK(sign(stricmp("ABC", "ab")) == 1);
    /* '_' sorts after letters once they are lowered */
    CHECK(sign(stricmp("_", "A")) == -1);

    /* copies */
    CHECK(strcpy(buf, s) == buf && strcmp(buf, "hello") == 0);
    CHECK(strcat(buf, ", world") == buf && strcmp(buf, "hello, world") == 0);
    CHECK(strcat(buf, "") == buf && strlen(buf) == 12);
    CHECK(memset(buf, 'x', 3) == buf && memcmp(buf, "xxxlo", 5) == 0);
    CHECK(memset(buf, 0x141, 1) == buf && buf[0] == 'A');
    CHECK(memcpy(buf, "12345", 5) == buf && memcmp(buf, "12345,", 6) == 0);
    CHECK(memcpy(buf, "zz", 0) == buf && buf[0] == '1');
    CHECK(strlen("") == 0 && strlen(s) == 5);
    strcpy(buf, "MiXeD 1-Z@[");
    CHECK(strlwr(buf) == buf && strcmp(buf, "mixed 1-z@[") == 0);

    /* overlapping and bounded copies */
    strcpy(buf, "abcdef");
    CHECK(memmove(buf + 2, buf, 4) == buf + 2 && memcmp(buf, "ababcd", 6) == 0);
    strcpy(buf, "abcdef");
    CHECK(memmove(buf, buf + 2, 4) == buf && memcmp(buf, "cdefef", 6) == 0);
    CHECK(memmove(buf, buf, 6) == buf && memmove(buf, "z", 0) == buf && buf[0] == 'c');
    memset(buf, 'x', sizeof buf);
    CHECK(strncpy(buf, "ab", 5) == buf && memcmp(buf, "ab\0\0\0x", 6) == 0);
    CHECK(strncpy(buf, "abcdef", 3) == buf && memcmp(buf, "abc\0", 4) == 0);
    CHECK(strncpy(buf, "q", 0) == buf && buf[0] == 'a');
    strcpy(buf, "ab");
    CHECK(strncat(buf, "cdef", 2) == buf && strcmp(buf, "abcd") == 0);
    CHECK(strncat(buf, "e", 5) == buf && strcmp(buf, "abcde") == 0);
    CHECK(strncat(buf, "f", 0) == buf && strcmp(buf, "abcde") == 0);
    CHECK(strnicmp("HeLLo!", "hEllO?", 5) == 0);
    CHECK(sign(strnicmp("HeLLo!", "hEllO?", 6)) == -1);
    CHECK(sign(strnicmp("abc", "AB", 3)) == 1 && strnicmp("abc", "ABD", 2) == 0);
    CHECK(strnicmp("a", "b", 0) == 0 && strnicmp("ab", "AB", 10) == 0);
    CHECK(_fmemcpy(fbuf, "far copy", 9) == fbuf);
    CHECK(_fmemcpy(buf, fbuf, 9) == (void __far *)buf && strcmp(buf, "far copy") == 0);
    CHECK(_fmemcpy(fbuf + 4, fbuf, 0) == fbuf + 4 && fbuf[4] == 'c');

    /* searches */
    CHECK(strchr(s, 'l') == s + 2);
    CHECK(strrchr(s, 'l') == s + 3);
    CHECK(strchr(s, 'z') == NULL && strrchr(s, 'z') == NULL);
    CHECK(strchr(s, '\0') == s + 5 && strrchr(s, '\0') == s + 5);
    CHECK(strchr(s, 'h' + 256) == s);
    CHECK(strchr(hi, 0x80) == hi + 1);
    CHECK(strpbrk(s, "ol") == s + 2 && strpbrk(s, "xyz") == NULL);
    CHECK(strpbrk(s, "") == NULL && strpbrk("", "a") == NULL);
    CHECK(strpbrk(hi, "\x80") == hi + 1);

    /* messages */
    CHECK(strcmp(strerror(0), "No error") == 0);
    CHECK(strcmp(strerror(ENOENT), "No such file or directory") == 0);
    CHECK(strcmp(strerror(EBADF), "Bad file number") == 0);
    CHECK(strcmp(strerror(EACCES), "Permission denied") == 0);
    CHECK(strcmp(strerror(EXDEV), "Cross-device link") == 0);
    CHECK(strcmp(strerror(ERANGE), "Result too large") == 0);
    CHECK(strcmp(strerror(1), "Unknown error") == 0);
    CHECK(strcmp(strerror(-1), "Unknown error") == 0);
    CHECK(strcmp(strerror(255), "Unknown error") == 0);
    CHECK(strcmp(strerror(0x102), "Unknown error") == 0);
    return done();
}
