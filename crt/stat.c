/* stat - libc16 (docs/spec/files.md) */

#include <dos.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "time16.h"

static unsigned char get_drive(void);
#pragma aux get_drive = \
    "mov ah, 19h" \
    "int 21h" \
    value [al] modify exact [ax];

/* Whether name, the last component of a path, ends in .EXE, .COM or .BAT */
static int is_program(const char *name)
{
    const char *dot = strrchr(name, '.');

    return dot != NULL && (stricmp(dot, ".EXE") == 0 || stricmp(dot, ".COM") == 0
                           || stricmp(dot, ".BAT") == 0);
}

int __cdecl stat(const char *path, struct stat *buf)
{
    struct find_t f;
    char full[_MAX_PATH];
    unsigned mode;
    int drive;

    if (strpbrk(path, "*?") != NULL) {
        errno = ENOENT;
        return -1;
    }
    if (_dos_findfirst(path, _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR, &f) == 0) {
        mode = f.attrib & _A_SUBDIR ? S_IFDIR | S_IEXEC : S_IFREG;
        mode |= S_IREAD;
        if (!(f.attrib & _A_RDONLY))
            mode |= S_IWRITE;
        if (!(f.attrib & _A_SUBDIR) && is_program(f.name))
            mode |= S_IEXEC;
        buf->st_size = f.attrib & _A_SUBDIR ? 0 : f.size;
        buf->st_mtime = __libc16_from_dos(f.wr_date, f.wr_time);
    } else {
        /* A root directory has no entry of its own. */
        if (_fullpath(full, path, sizeof full) == NULL || full[3] != '\0') {
            errno = ENOENT;
            return -1;
        }
        mode = S_IFDIR | S_IREAD | S_IWRITE | S_IEXEC;
        buf->st_size = 0;
        buf->st_mtime = __libc16_from_dos(0x21, 0);        /* 1980-01-01 */
    }
    if (path[0] != '\0' && path[1] == ':')
        drive = (path[0] | 0x20) - 'a';
    else
        drive = get_drive();
    /* The owner's permissions are also the group's and everyone else's. */
    buf->st_mode = mode | (mode & 0x1c0) >> 3 | (mode & 0x1c0) >> 6;
    buf->st_dev = buf->st_rdev = drive;
    buf->st_ino = 0;
    buf->st_nlink = 1;
    buf->st_uid = buf->st_gid = 0;
    buf->st_atime = buf->st_ctime = buf->st_mtime;
    return 0;
}
