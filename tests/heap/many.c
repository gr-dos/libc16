/* Thousands of small allocations, as an editor makes for its lines
 * (docs/spec/heap.md, "Constraints"), in every model: allocate them,
 * free every other one, allocate those again, and check every block's
 * contents. With far data there are more lines, more than 64 KiB of
 * them, and a search that passed every earlier block for each new one
 * would take long enough for tests/run-dos to time the run out. The
 * small model's heap is a single segment, searched from its start, so it
 * has fewer. Prints one line per check (check.h). */

#include <stdlib.h>
#include "check.h"

#ifdef FAR_DATA
#define LINES 8000
#else
#define LINES 800
#endif

static char *line[LINES];

/* Line i's size: 1 to 40 bytes. */
#define SIZE(i) ((i) * 7u % 40 + 1)

static int allocate(unsigned from, unsigned step)
{
    unsigned i;

    for (i = from; i < LINES; i += step) {
        line[i] = malloc(SIZE(i));
        if (!placed(line[i], SIZE(i)))
            return 0;
        fill(line[i], SIZE(i), (unsigned char)i);
    }
    return 1;
}

static int intact(void)
{
    unsigned i;

    for (i = 0; i < LINES; i++)
        if (!filled(line[i], SIZE(i), (unsigned char)i))
            return 0;
    return 1;
}

int main(void)
{
    unsigned i;
    unsigned long total = 0;
    char *p;

    check("thousands of small allocations are placed", allocate(0, 1));
    check("thousands of small allocations keep their contents", intact());
    for (i = 0; i < LINES; i++)
        total += SIZE(i);
#ifdef FAR_DATA
    check(DATA "more than 64 KiB in lines", total > 0x10000ul);
#else
    check(DATA "the lines fit in DGROUP", total < 0x10000ul);
#endif
    for (i = 0; i < LINES; i += 2)
        free(line[i]);
    check("freed lines are allocated again", allocate(0, 2));
    check("every line keeps its contents", intact());
    for (i = 0; i < LINES; i++)
        free(line[i]);
    p = malloc(10000);
    check("a large block after freeing them all", placed(p, 10000));
    free(p);
    return failed;
}
