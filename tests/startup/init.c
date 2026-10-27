/* Init routines of the program's own (docs/spec/startup.md, "Init
 * table"), from tests/startup/inittab.asm: an assembly one, which with
 * far data returns with DS = 0, then c_init, a default-convention C
 * function. Both are called near, or far in the large model, in link
 * order, before main, which is called with DS = DGROUP. Prints a line
 * for each check. */

#include "testio.h"

void c_init(void);

int init_order;                 /* 1 after the assembly routine, 2 after c_init */
static int c_init_saw;          /* init_order when c_init ran */

static unsigned get_ds(void);
#pragma aux get_ds = \
    "mov ax, ds" \
    value [ax];

static unsigned get_ss(void);
#pragma aux get_ss = \
    "mov ax, ss" \
    value [ax];

void c_init(void)
{
    c_init_saw = init_order;
    init_order = 2;
}

int main(void)
{
    /* DS first, before any other code of main can load it. */
    int ds_ok = get_ds() == get_ss();

    t_puts("order ");
    t_puts(c_init_saw == 1 && init_order == 2 ? "ok" : "wrong");
    t_puts("\r\nds ");
    t_puts(ds_ok ? "ok" : "wrong");
    t_puts("\r\n");
    return 0;
}
