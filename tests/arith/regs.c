/* Each arithmetic helper preserves SI, DI, BP and ES (docs/ABI.md,
 * "Compiler support routines"), and the 64-bit ones leave the operand at
 * DS:SI unchanged. Each check loads marker values, calls the helper by its
 * symbol and returns the bits that changed, so 0 is a pass. The operands
 * take the 32-bit divides through both of their paths. */

#include "testio.h"

/* Marker values, set before the call and compared after it. */
#define MARK \
    "push bp" \
    "mov ax, 1234h" \
    "mov es, ax" \
    "mov di, 0A5A5h" \
    "mov bp, 5A5Ah"

#define CHANGED \
    "mov ax, es" \
    "xor ax, 1234h" \
    "xor di, 0A5A5h" \
    "or ax, di" \
    "xor bp, 5A5Ah" \
    "or ax, bp" \
    "pop bp"

/* 32-bit: DX:AX = 12345678h, CX:BX as given (whole instructions); SI is a marker too. */
#define BODY32(call, hi, lo) \
    MARK \
    "mov si, 0C3C3h" \
    "mov dx, 1234h" \
    "mov ax, 5678h" \
    hi \
    lo \
    call \
    "xor si, 0C3C3h" \
    "mov cx, si" \
    CHANGED \
    "or ax, cx"

/* 64-bit: AX:BX:CX:DX = 0123456789ABCDEFh, SI = the operand (as given),
 * or the shift count. */
#define BODY64(call) \
    MARK \
    "push si" \
    "mov ax, 0123h" \
    "mov bx, 4567h" \
    "mov cx, 89ABh" \
    "mov dx, 0CDEFh" \
    call \
    "pop cx" \
    "xor cx, si" \
    CHANGED \
    "or ax, cx"

extern void I4M(void);
#pragma aux I4M "__I4M"
extern void U4M(void);
#pragma aux U4M "__U4M"
extern void I4D(void);
#pragma aux I4D "__I4D"
extern void U4D(void);
#pragma aux U4D "__U4D"
extern void I8M(void);
#pragma aux I8M "__I8M"
extern void U8M(void);
#pragma aux U8M "__U8M"
extern void I8DQ(void);
#pragma aux I8DQ "__I8DQ"
extern void I8DR(void);
#pragma aux I8DR "__I8DR"
extern void U8DQ(void);
#pragma aux U8DQ "__U8DQ"
extern void U8DR(void);
#pragma aux U8DR "__U8DR"
extern void I8LS(void);
#pragma aux I8LS "__I8LS"
extern void U8LS(void);
#pragma aux U8LS "__U8LS"
extern void I8RS(void);
#pragma aux I8RS "__I8RS"
extern void U8RS(void);
#pragma aux U8RS "__U8RS"

static unsigned r_i4m(void);
#pragma aux r_i4m = BODY32("call near ptr I4M", "mov cx, 0FEDCh", "mov bx, 0BA98h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4m(void);
#pragma aux r_u4m = BODY32("call near ptr U4M", "mov cx, 0FEDCh", "mov bx, 0BA98h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_i4d_short(void);
#pragma aux r_i4d_short = BODY32("call near ptr I4D", "mov cx, 0", "mov bx, 0Ah") value [ax] modify [ax bx cx dx si di es];
static unsigned r_i4d_long(void);
#pragma aux r_i4d_long = BODY32("call near ptr I4D", "mov cx, 0FFFFh", "mov bx, 0FFF0h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4d_short(void);
#pragma aux r_u4d_short = BODY32("call near ptr U4D", "mov cx, 0", "mov bx, 0Ah") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4d_long(void);
#pragma aux r_u4d_long = BODY32("call near ptr U4D", "mov cx, 0FFFFh", "mov bx, 0FFF0h") value [ax] modify [ax bx cx dx si di es];

static unsigned r_i8m(const void *b);
#pragma aux r_i8m = BODY64("call near ptr I8M") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8m(const void *b);
#pragma aux r_u8m = BODY64("call near ptr U8M") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8dq(const void *b);
#pragma aux r_i8dq = BODY64("call near ptr I8DQ") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8dr(const void *b);
#pragma aux r_i8dr = BODY64("call near ptr I8DR") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8dq(const void *b);
#pragma aux r_u8dq = BODY64("call near ptr U8DQ") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8dr(const void *b);
#pragma aux r_u8dr = BODY64("call near ptr U8DR") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8ls(unsigned n);
#pragma aux r_i8ls = BODY64("call near ptr I8LS") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8ls(unsigned n);
#pragma aux r_u8ls = BODY64("call near ptr U8LS") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8rs(unsigned n);
#pragma aux r_i8rs = BODY64("call near ptr I8RS") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8rs(unsigned n);
#pragma aux r_u8rs = BODY64("call near ptr U8RS") parm [si] value [ax] modify [ax bx cx dx si di es];

static int failed;

static void check(const char *name, unsigned changed)
{
    t_puts(name);
    t_puts(changed ? " changed registers\r\n" : " ok\r\n");
    if (changed)
        failed = 1;
}

/* The 64-bit operand: negative, so that the signed divides negate it. */
static unsigned b[4];

static void check64(const char *name, unsigned changed)
{
    if (b[0] != 0x3210 || b[1] != 0x7654 || b[2] != 0xBA98 || b[3] != 0xFEDC) {
        t_puts(name);
        t_puts(" changed its operand\r\n");
        failed = 1;
    }
    check(name, changed);
}

int main(void)
{
    b[0] = 0x3210;
    b[1] = 0x7654;
    b[2] = 0xBA98;
    b[3] = 0xFEDC;
    check("__I4M", r_i4m());
    check("__U4M", r_u4m());
    check("__I4D short", r_i4d_short());
    check("__I4D long", r_i4d_long());
    check("__U4D short", r_u4d_short());
    check("__U4D long", r_u4d_long());
    check64("__I8M", r_i8m(b));
    check64("__U8M", r_u8m(b));
    check64("__I8DQ", r_i8dq(b));
    check64("__I8DR", r_i8dr(b));
    check64("__U8DQ", r_u8dq(b));
    check64("__U8DR", r_u8dr(b));
    check("__I8LS", r_i8ls(13));
    check("__U8LS", r_u8ls(13));
    check("__I8RS", r_i8rs(13));
    check("__U8RS", r_u8rs(13));
    return failed;
}
