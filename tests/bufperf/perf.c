/* The time and DOS writes of buffered and unbuffered output
 * (docs/spec/stdio.md, "Performance"), in every model: an interpreter's
 * print of four items a line, 2,000 lines (as gr-dos/libc16#34 measured
 * perl16 with), one byte at a time, and large sequential writes, each to
 * a file in text mode. tests/stdbuf/wrhook counts the DOS writes.
 *
 * Time is the BIOS tick count (18.2 per second) of the emulated machine
 * (tests/heapperf/scale.c). Each line prints the workload, its writes
 * and ticks unbuffered, then with a buffer of BUFSIZ bytes from setvbuf;
 * the check passes when the buffered run makes at most a 20th of the
 * writes and takes less time. The .expect file matches the numbers with
 * a pattern. */

#include <dos.h>
#include <stdio.h>
#include "check.h"

void __cdecl wrhook_install(void);
void __cdecl wrhook_remove(void);
void __cdecl wrhook_watch(int fd);
unsigned __cdecl wrhook_count(void);

static char big[4096];

static unsigned long ticks(void)
{
    volatile unsigned long __far *t = MK_FP(0x40, 0x6C);
    unsigned long a, b;

    do {
        a = *t;
        b = *t;
    } while (a != b);
    return a;
}

static void print4(FILE *fp)
{
    char num[6];
    unsigned i, k;

    for (i = 0; i < 2000; i++) {
        k = i;
        num[5] = '\0';
        num[4] = (char)('0' + k % 10);
        num[3] = (char)('0' + (k /= 10) % 10);
        num[2] = (char)('0' + (k /= 10) % 10);
        num[1] = (char)('0' + (k /= 10) % 10);
        num[0] = 'n';
        fputs(num, fp);
        fputs(" = ", fp);
        fwrite("value", 1, 5, fp);
        fputs("\n", fp);
    }
}

static void bytes(FILE *fp)
{
    unsigned i;

    for (i = 0; i < 20000; i++)
        putc(i % 64 == 63 ? '\n' : 'a' + i % 26, fp);
}

static void blocks(FILE *fp)
{
    unsigned i;

    for (i = 0; i < 64; i++)
        fwrite(big, 1, sizeof big, fp);
}

static unsigned long run(void (*work)(FILE *), int buffered, unsigned *writes)
{
    FILE *fp = fopen("PERF.TXT", "w");
    unsigned long t0, t1;

    CHECK(fp != NULL);
    if (buffered)
        CHECK(setvbuf(fp, NULL, _IOFBF, BUFSIZ) == 0);
    wrhook_watch(fileno(fp));
    t0 = ticks();
    work(fp);
    CHECK(fclose(fp) == 0);
    t1 = ticks();
    *writes = wrhook_count();
    return t1 - t0;
}

static void measure(const char *name, void (*work)(FILE *))
{
    unsigned long tu, tb;
    unsigned wu, wb;

    tu = run(work, 0, &wu);
    tb = run(work, 1, &wb);
    t_puts(name);
    t_puts(": unbuffered ");
    t_putu(wu);
    t_puts(" writes ");
    t_putu((unsigned)tu);
    t_puts(" ticks, buffered ");
    t_putu(wb);
    t_puts(" writes ");
    t_putu((unsigned)tb);
    t_puts(" ticks\r\n");
    CHECK(wb <= wu / 20 && tb < tu);
}

int main(void)
{
    unsigned i;

    for (i = 0; i < sizeof big; i++)
        big[i] = (char)(i % 80 == 79 ? '\n' : 'a' + i % 26);
    wrhook_install();
    measure("print4", print4);
    measure("bytes", bytes);
    /* large writes gain little: text mode already writes in pieces */
    {
        unsigned long tu, tb;
        unsigned wu, wb;

        tu = run(blocks, 0, &wu);
        tb = run(blocks, 1, &wb);
        t_puts("blocks: unbuffered ");
        t_putu(wu);
        t_puts(" writes ");
        t_putu((unsigned)tu);
        t_puts(" ticks, buffered ");
        t_putu(wb);
        t_puts(" writes ");
        t_putu((unsigned)tb);
        t_puts(" ticks\r\n");
        CHECK(wb <= wu && tb <= tu + 2);
    }
    wrhook_remove();
    return done();
}
