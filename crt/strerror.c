/* strerror - libc16 (docs/spec/string.md) */

#include <errno.h>
#include <string.h>

/* Each message is preceded by its errno; the list ends with the message
 * for any other value. */
static const char messages[] =
    "\0No error\0"
    "\x02No such file or directory\0"
    "\x07" "Arg list too long\0"
    "\x08" "Exec format error\0"
    "\x09" "Bad file number\0"
    "\x0c" "Not enough memory\0"
    "\x0d" "Permission denied\0"
    "\x11" "File exists\0"
    "\x12" "Cross-device link\0"
    "\x16" "Invalid argument\0"
    "\x18" "Too many open files\0"
    "\x1c" "No space left on device\0"
    "\x21" "Math argument\0"
    "\x22" "Result too large\0"
    "\xff" "Unknown error";

char * __cdecl strerror(int errnum)
{
    const char *p = messages;

    while ((unsigned char)*p != 0xff && (unsigned char)*p != errnum) {
        p++;
        while (*p++ != '\0')
            ;
    }
    return (char *)p + 1;
}
