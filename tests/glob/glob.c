/* libc16_glob and libc16_globfree (docs/spec/glob.md), in every model.
 *
 * The fixtures are made fresh on drive D:, a FAT floppy image with the
 * volume label GLOBVOL (tests/glob/cases), which keeps DOS attributes,
 * and in C:\CT on the test directory. Files are created out of order,
 * so that DOS's order (creation order on FAT) differs from the sorted
 * one. */

#include <dos.h>
#include <direct.h>
#include <errno.h>
#include <libc16.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"

#define BIGN 300

/* the heap before check_big's glob, in fill_heap's measure */
static unsigned long heap_free;
static unsigned long fill_heap(void);
static void empty_heap(void);

static void mkfile(const char *path, unsigned attr, const char *text)
{
    int h;
    unsigned n;

    CHECK(_dos_creat(path, attr, &h) == 0);
    if (text != NULL)
        CHECK(_dos_write(h, text, strlen(text), &n) == 0 && n == strlen(text));
    CHECK(_dos_close(h) == 0);
}

/* g holds, from index `from`, exactly the paths in want (separated by
 * spaces), followed by a null pointer */
static int holds(const libc16_glob_t *g, unsigned from, const char *want)
{
    unsigned i = from;

    while (*want != '\0') {
        unsigned n = 0;

        while (want[n] != ' ' && want[n] != '\0')
            n++;
        if (i >= g->gl_pathc || strlen(g->gl_pathv[i]) != n
            || memcmp(g->gl_pathv[i], want, n) != 0)
            return 0;
        i++;
        want += n;
        if (*want == ' ')
            want++;
    }
    return i == g->gl_pathc && g->gl_pathv[i] == NULL;
}

static int empty(const libc16_glob_t *g)
{
    return g->gl_pathc == 0 && g->gl_pathv == NULL;
}

/* glob pattern without APPEND and compare */
static int globs(const char *pattern, unsigned flags, const char *want)
{
    libc16_glob_t g;
    int ok = libc16_glob(pattern, flags, &g) == 0 && holds(&g, 0, want);

    libc16_globfree(&g);
    return ok && empty(&g);
}

/* glob pattern without APPEND and expect result r, errno e and g empty */
static int fails(const char *pattern, unsigned flags, int r, int e)
{
    libc16_glob_t g;

    errno = 0;
    return libc16_glob(pattern, flags, &g) == r && errno == e && empty(&g);
}

static void fixtures(void)
{
    char name[] = "D:\\BIG\\N000.DAT";
    int i;

    CHECK(mkdir("D:\\GT") == 0);
    mkfile("D:\\GT\\SYS.TXT", _A_SYSTEM, NULL);
    mkfile("D:\\GT\\RO.TXT", _A_RDONLY, NULL);
    mkfile("D:\\GT\\HID.TXT", _A_HIDDEN, NULL);
    CHECK(mkdir("D:\\GT\\DIR.TXT") == 0);
    mkfile("D:\\GT\\b.txt", _A_NORMAL, NULL);
    mkfile("D:\\GT\\_U.TXT", _A_ARCH, NULL);
    mkfile("D:\\GT\\A.TXT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\FILE10.DAT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\FILE2.DAT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\FILE.DAT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\FILE1.DAT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\NOEXT", _A_NORMAL, NULL);
    CHECK(mkdir("D:\\GT\\SUB") == 0);
    mkfile("D:\\GT\\SUB\\S2.TXT", _A_NORMAL, NULL);
    mkfile("D:\\GT\\SUB\\S1.TXT", _A_NORMAL, NULL);

    /* BIGN files, created from the last name down */
    CHECK(mkdir("D:\\BIG") == 0);
    for (i = BIGN - 1; i >= 0; i--) {
        name[8] = (char)('0' + i / 100);
        name[9] = (char)('0' + i / 10 % 10);
        name[10] = (char)('0' + i % 10);
        mkfile(name, _A_NORMAL, NULL);
    }

    /* C:\CT may be left from an earlier run; _dos_creat truncates */
    mkdir("CT");
    mkfile("CT\\N.TXT", _A_NORMAL, "n\r\n");
    mkfile("CT\\M.TXT", _A_NORMAL, "m\r\n");
}

static void check_patterns(void)
{
    struct find_t f;

    /* files only, sorted case-insensitively: '_' before the letters;
     * Read-only and Archive do not matter */
    CHECK(globs("D:\\GT\\*.TXT", 0, "D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\RO.TXT"));
    /* Hidden and System only when asked for, though DOSBox-X returns them */
    CHECK(globs("D:\\GT\\*.TXT", LIBC16_GLOB_HIDDEN,
                "D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\HID.TXT D:\\GT\\RO.TXT D:\\GT\\SYS.TXT"));
    CHECK(fails("D:\\GT\\HID.*", 0, LIBC16_GLOB_NOMATCH, ENOENT));
    /* directories only when asked for; never . and .. */
    CHECK(globs("D:\\GT\\*.TXT", LIBC16_GLOB_DIRS,
                "D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\DIR.TXT D:\\GT\\RO.TXT"));
    CHECK(globs("D:\\GT\\SUB\\*.*", LIBC16_GLOB_DIRS | LIBC16_GLOB_HIDDEN,
                "D:\\GT\\SUB\\S1.TXT D:\\GT\\SUB\\S2.TXT"));
    CHECK(globs("D:\\GT\\*.*", LIBC16_GLOB_DIRS | LIBC16_GLOB_HIDDEN,
                "D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\DIR.TXT D:\\GT\\FILE.DAT "
                "D:\\GT\\FILE1.DAT D:\\GT\\FILE10.DAT D:\\GT\\FILE2.DAT D:\\GT\\HID.TXT "
                "D:\\GT\\NOEXT D:\\GT\\RO.TXT D:\\GT\\SUB D:\\GT\\SYS.TXT"));
    /* DOS's order */
    CHECK(globs("D:\\GT\\*.TXT", LIBC16_GLOB_NOSORT | LIBC16_GLOB_HIDDEN | LIBC16_GLOB_DIRS,
                "D:\\GT\\SYS.TXT D:\\GT\\RO.TXT D:\\GT\\HID.TXT D:\\GT\\DIR.TXT D:\\GT\\B.TXT "
                "D:\\GT\\_U.TXT D:\\GT\\A.TXT"));

    /* DOS's 8.3 matching: ? matches a character or the padding, * with
     * no dot only names without an extension */
    CHECK(globs("D:\\GT\\FILE?.DAT", 0, "D:\\GT\\FILE.DAT D:\\GT\\FILE1.DAT D:\\GT\\FILE2.DAT"));
    CHECK(globs("D:\\GT\\FILE??.DAT", 0,
                "D:\\GT\\FILE.DAT D:\\GT\\FILE1.DAT D:\\GT\\FILE10.DAT D:\\GT\\FILE2.DAT"));
    CHECK(globs("D:\\GT\\*", 0, "D:\\GT\\NOEXT"));
    /* what follows a * in the name is ignored: F*1 is F* */
    CHECK(globs("D:\\GT\\F*1.*", 0,
                "D:\\GT\\FILE.DAT D:\\GT\\FILE1.DAT D:\\GT\\FILE10.DAT D:\\GT\\FILE2.DAT"));
    /* case-insensitive; the pattern's directory part is kept as written */
    CHECK(globs("d:\\gt\\file?.dat", 0, "d:\\gt\\FILE.DAT d:\\gt\\FILE1.DAT d:\\gt\\FILE2.DAT"));
    CHECK(globs("D:/GT/a*.T?T", 0, "D:/GT/A.TXT"));
    CHECK(globs("D:\\GT\\?.TXT", 0, "D:\\GT\\A.TXT D:\\GT\\B.TXT"));

    /* never the volume label, whatever the flags; it is there */
    CHECK(_dos_findfirst("D:\\*.*", _A_VOLID, &f) == 0 && strcmp(f.name, "GLOBVOL") == 0);
    CHECK(globs("D:\\*.*", LIBC16_GLOB_DIRS | LIBC16_GLOB_HIDDEN, "D:\\BIG D:\\GT"));
    CHECK(globs("D:\\GLOB*", LIBC16_GLOB_NOCHECK | LIBC16_GLOB_DIRS | LIBC16_GLOB_HIDDEN, "D:\\GLOB*"));
}

static void check_literals_and_errors(void)
{
    libc16_glob_t g;

    /* no wildcard: the operand itself, without a search */
    CHECK(globs("NOSUCH.TXT", 0, "NOSUCH.TXT"));
    CHECK(globs("CON", 0, "CON"));
    CHECK(globs("D:\\GT\\DIR.TXT", 0, "D:\\GT\\DIR.TXT"));
    CHECK(globs("Q:\\NODIR\\X", 0, "Q:\\NODIR\\X"));

    /* nothing matches */
    CHECK(fails("D:\\GT\\*.XYZ", 0, LIBC16_GLOB_NOMATCH, ENOENT));
    CHECK(globs("D:\\GT\\*.XYZ", LIBC16_GLOB_NOCHECK, "D:\\GT\\*.XYZ"));
    /* a missing directory, an invalid drive */
    _doserrno = 0;
    CHECK(fails("D:\\NODIR\\*.*", 0, LIBC16_GLOB_ERROR, ENOENT) && _doserrno == 3);
    CHECK(fails("D:\\NODIR\\*.*", LIBC16_GLOB_NOCHECK, LIBC16_GLOB_ERROR, ENOENT));
    _doserrno = 0;
    CHECK(fails("Q:*.*", 0, LIBC16_GLOB_ERROR, ENOENT) && _doserrno != 0);
    /* a wildcard before the last component */
    _doserrno = 0;
    CHECK(fails("D:\\G*\\*.TXT", 0, LIBC16_GLOB_ERROR, EINVAL) && _doserrno == 0);
    CHECK(fails("D:\\G?\\A.TXT", LIBC16_GLOB_NOCHECK, LIBC16_GLOB_ERROR, EINVAL));
    CHECK(fails("*:\\A.TXT", 0, LIBC16_GLOB_ERROR, EINVAL));

    /* without APPEND *g is set empty first, whatever it held */
    g.gl_pathc = 5;
    g.gl_pathv = (char **)&g;
    g.__gl_slots = 7;
    CHECK(libc16_glob("D:\\GT\\?.TXT", 0, &g) == 0 && holds(&g, 0, "D:\\GT\\A.TXT D:\\GT\\B.TXT"));
    libc16_globfree(&g);
    CHECK(empty(&g));
    libc16_globfree(&g);
    CHECK(empty(&g));
    g.gl_pathc = 5;
    g.gl_pathv = (char **)&g;
    CHECK(libc16_glob("D:\\GT\\*.XYZ", 0, &g) == LIBC16_GLOB_NOMATCH && empty(&g));
}

/* operands one after the other: their order kept, each pattern's own
 * matches sorted, duplicates kept, and a failed operand adding nothing */
static void check_append(void)
{
    libc16_glob_t g = { 0 };

    CHECK(libc16_glob("D:\\GT\\FILE?.DAT", LIBC16_GLOB_APPEND, &g) == 0);
    CHECK(libc16_glob("D:\\GT\\A.TXT", LIBC16_GLOB_APPEND, &g) == 0);
    CHECK(libc16_glob("D:\\GT\\*.XYZ", LIBC16_GLOB_APPEND, &g) == LIBC16_GLOB_NOMATCH);
    CHECK(libc16_glob("D:\\NODIR\\*.*", LIBC16_GLOB_APPEND, &g) == LIBC16_GLOB_ERROR);
    CHECK(libc16_glob("D:\\G*\\A.TXT", LIBC16_GLOB_APPEND, &g) == LIBC16_GLOB_ERROR);
    CHECK(libc16_glob("D:\\GT\\*.TXT", LIBC16_GLOB_APPEND, &g) == 0);
    CHECK(libc16_glob("D:\\GT\\*.XYZ", LIBC16_GLOB_APPEND | LIBC16_GLOB_NOCHECK, &g) == 0);
    CHECK(holds(&g, 0, "D:\\GT\\FILE.DAT D:\\GT\\FILE1.DAT D:\\GT\\FILE2.DAT D:\\GT\\A.TXT "
                       "D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\RO.TXT D:\\GT\\*.XYZ"));
    libc16_globfree(&g);
    CHECK(empty(&g));
}

/* relative, drive-relative and absolute patterns across two drives, each
 * with its own current directory */
static void check_drives(void)
{
    CHECK(chdir("D:\\GT\\SUB") == 0);
    CHECK(globs("D:*.TXT", 0, "D:S1.TXT D:S2.TXT"));
    CHECK(globs("D:..\\?.TXT", 0, "D:..\\A.TXT D:..\\B.TXT"));
    CHECK(globs("D:\\GT\\?.TXT", 0, "D:\\GT\\A.TXT D:\\GT\\B.TXT"));
    CHECK(globs("CT\\*.TXT", 0, "CT\\M.TXT CT\\N.TXT"));
    CHECK(globs("C:CT\\*.TXT", 0, "C:CT\\M.TXT C:CT\\N.TXT"));
    CHECK(globs("\\CT\\*.TXT", 0, "\\CT\\M.TXT \\CT\\N.TXT"));
    CHECK(chdir("CT") == 0);
    CHECK(globs("*.TXT", 0, "M.TXT N.TXT"));
    CHECK(globs("..\\CT\\M*.*", 0, "..\\CT\\M.TXT"));
    CHECK(globs("C:\\CT\\N.*", 0, "C:\\CT\\N.TXT"));
    CHECK(globs("D:S?.*", 0, "D:S1.TXT D:S2.TXT"));
    CHECK(chdir("..") == 0);
    CHECK(chdir("D:\\") == 0);
    CHECK(globs("D:*.*", LIBC16_GLOB_DIRS, "D:BIG D:GT"));
}

static unsigned bign(const char *p)
{
    return (unsigned)(p[8] - '0') * 100 + (p[9] - '0') * 10 + (p[10] - '0');
}

static void check_big(void)
{
    libc16_glob_t g;
    unsigned long held;
    unsigned i;
    int bad = 0;

    heap_free = fill_heap();
    empty_heap();

    CHECK(libc16_glob("D:\\BIG\\*.DAT", 0, &g) == 0 && g.gl_pathc == BIGN
          && g.gl_pathv[BIGN] == NULL);
    for (i = 0; i < g.gl_pathc; i++)
        if (strlen(g.gl_pathv[i]) != 15 || bign(g.gl_pathv[i]) != i)
            bad++;
    CHECK(bad == 0);
    /* the heap it holds: 7,128 bytes in the small model and 8,140 with
     * far data under DOSBox-X, within 24 bytes a path and the 512-slot
     * array */
    held = fill_heap();
    empty_heap();
    CHECK(heap_free - held <= BIGN * 24ul + 512 * sizeof(char *));
    libc16_globfree(&g);
    CHECK(libc16_glob("D:\\BIG\\*.DAT", LIBC16_GLOB_NOSORT, &g) == 0 && g.gl_pathc == BIGN);
    for (i = 0; i < g.gl_pathc; i++)
        if (bign(g.gl_pathv[i]) != BIGN - 1 - i)
            bad++;
    CHECK(bad == 0);
    libc16_globfree(&g);
    CHECK(empty(&g));
}

/* The heap filled with ballast, 1 KiB blocks and then the smallest,
 * with two of the large blocks then traded for small ones, and given
 * back a block at a time, smallest first, so that each glob before the
 * one that succeeds has a few more bytes: each must fail with NOSPACE,
 * leaving g as it was. Afterwards the heap holds as much as before, so
 * nothing leaked. */
struct blk {
    struct blk *next;
};

static struct blk *ballast[2];
static const unsigned sizes[2] = { 1024, sizeof(struct blk) };

static unsigned long grab(int k)
{
    unsigned long n = 0;
    struct blk *p;

    while ((p = malloc(sizes[k])) != NULL) {
        p->next = ballast[k];
        ballast[k] = p;
        n += sizes[k];
    }
    return n;
}

static unsigned long fill_heap(void)
{
    unsigned long n = grab(0) + grab(1);
    int i;

    for (i = 0; i < 2 && ballast[0] != NULL; i++) {
        struct blk *p = ballast[0];

        ballast[0] = p->next;
        free(p);
        n = n - sizes[0] + grab(1);
    }
    return n;
}

static int give_back(void)
{
    int k;

    for (k = 1; k >= 0; k--)
        if (ballast[k] != NULL) {
            struct blk *p = ballast[k];

            ballast[k] = p->next;
            free(p);
            return 1;
        }
    return 0;
}

static void empty_heap(void)
{
    while (give_back())
        ;
}

static void check_nospace(void)
{
    static const char want[] =
        "D:\\GT\\A.TXT D:\\GT\\_U.TXT D:\\GT\\A.TXT D:\\GT\\B.TXT D:\\GT\\DIR.TXT D:\\GT\\FILE.DAT "
        "D:\\GT\\FILE1.DAT D:\\GT\\FILE10.DAT D:\\GT\\FILE2.DAT D:\\GT\\HID.TXT "
        "D:\\GT\\NOEXT D:\\GT\\RO.TXT D:\\GT\\SUB D:\\GT\\SYS.TXT";
    unsigned flags = LIBC16_GLOB_APPEND | LIBC16_GLOB_DIRS | LIBC16_GLOB_HIDDEN;
    libc16_glob_t g = { 0 };
    unsigned long before, after;
    unsigned tries = 0;
    int r, bad = 0;

    before = fill_heap();
    empty_heap();

    CHECK(libc16_glob("D:\\GT\\A.TXT", LIBC16_GLOB_APPEND, &g) == 0);
    fill_heap();
    errno = 0;
    while ((r = libc16_glob("D:\\GT\\*.*", flags, &g)) != 0) {
        if (r != LIBC16_GLOB_NOSPACE || errno != ENOMEM || !holds(&g, 0, "D:\\GT\\A.TXT"))
            bad++;
        tries++;
        if (!give_back())
            break;
        errno = 0;
    }
    CHECK(r == 0 && bad == 0 && tries > 20);
    CHECK(holds(&g, 0, want));
    empty_heap();
    libc16_globfree(&g);

    /* failing from nothing leaves g empty, the array freed */
    fill_heap();
    tries = 0;
    while ((r = libc16_glob("D:\\GT\\*.*", LIBC16_GLOB_DIRS, &g)) != 0) {
        if (r != LIBC16_GLOB_NOSPACE || !empty(&g))
            bad++;
        tries++;
        if (!give_back())
            break;
    }
    CHECK(r == 0 && bad == 0 && tries > 10 && g.gl_pathc == 11);
    empty_heap();
    libc16_globfree(&g);

    after = fill_heap();
    empty_heap();
    CHECK(after == before);
}

int main(void)
{
    fixtures();
    check_patterns();
    check_literals_and_errors();
    check_append();
    check_drives();
    check_big();
    check_nospace();
    return done();
}
