/* Each arithmetic helper preserves SI, DI, BP and ES (docs/ABI.md,
 * "Compiler support routines"), and the 64-bit ones also preserve DS, give
 * the right result and leave the operand at [SI] unchanged. Each check
 * loads marker values, calls the helper by its symbol and returns the bits
 * that changed, so 0 is a pass. The operands take the 32-bit divides
 * through both of their paths.
 *
 * Each call takes the distance of the helper's declaration, the model's
 * default: near in the small and compact models, far in the large model
 * (docs/spec/arith.md, "Memory models").
 *
 * With far data the 64-bit operand is at SS:SI, and the caller need not
 * load DS first, so the 64-bit checks call with DS one paragraph above
 * DGROUP. DS:SI is then 16 bytes past the operand, at a decoy that gives
 * different results (b, below). In the small model DS stays DGROUP. */

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

#if defined(__COMPACT__) || defined(__LARGE__)
/* DS = DGROUP + 1 for the call; afterwards AX = 0 if it was preserved. */
#define DS_SET \
    "push ds" \
    "mov ax, ss" \
    "inc ax" \
    "mov ds, ax"
#define DS_CHANGED \
    "mov ax, ds" \
    "mov dx, ss" \
    "sub ax, dx" \
    "dec ax" \
    "pop ds"
#else
#define DS_SET
#define DS_CHANGED \
    "mov ax, ds" \
    "mov dx, ss" \
    "sub ax, dx"
#endif

/* 64-bit: AX:BX:CX:DX = 0123456789ABCDEFh, SI = the operand's offset (as
 * given) or the shift count; r3 to r0 XOR the expected result out of AX,
 * BX, CX and DX (whole instructions). */
#define BODY64(call, r3, r2, r1, r0) \
    MARK \
    DS_SET \
    "push si" \
    "mov ax, 0123h" \
    "mov bx, 4567h" \
    "mov cx, 89ABh" \
    "mov dx, 0CDEFh" \
    call \
    r3 \
    r2 \
    "or ax, bx" \
    r1 \
    "or ax, cx" \
    r0 \
    "or ax, dx" \
    "pop cx" \
    "xor cx, si" \
    "or cx, ax" \
    DS_CHANGED \
    "or cx, ax" \
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
#pragma aux r_i4m = BODY32("call I4M", "mov cx, 0FEDCh", "mov bx, 0BA98h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4m(void);
#pragma aux r_u4m = BODY32("call U4M", "mov cx, 0FEDCh", "mov bx, 0BA98h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_i4d_short(void);
#pragma aux r_i4d_short = BODY32("call I4D", "mov cx, 0", "mov bx, 0Ah") value [ax] modify [ax bx cx dx si di es];
static unsigned r_i4d_long(void);
#pragma aux r_i4d_long = BODY32("call I4D", "mov cx, 0FFFFh", "mov bx, 0FFF0h") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4d_short(void);
#pragma aux r_u4d_short = BODY32("call U4D", "mov cx, 0", "mov bx, 0Ah") value [ax] modify [ax bx cx dx si di es];
static unsigned r_u4d_long(void);
#pragma aux r_u4d_long = BODY32("call U4D", "mov cx, 0FFFFh", "mov bx, 0FFF0h") value [ax] modify [ax bx cx dx si di es];

static unsigned r_i8m(unsigned b);
#pragma aux r_i8m = BODY64("call I8M", "xor ax, 2236h", "xor bx, 0D88Fh", "xor cx, 0E561h", "xor dx, 8CF0h") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8m(unsigned b);
#pragma aux r_u8m = BODY64("call U8M", "xor ax, 2236h", "xor bx, 0D88Fh", "xor cx, 0E561h", "xor dx, 8CF0h") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8dq(unsigned b);
#pragma aux r_i8dq = BODY64("call I8DQ", "xor ax, 0", "xor bx, 0", "xor cx, 0", "xor dx, 0") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8dr(unsigned b);
#pragma aux r_i8dr = BODY64("call I8DR", "xor ax, 0123h", "xor bx, 4567h", "xor cx, 89ABh", "xor dx, 0CDEFh") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8dq(unsigned b);
#pragma aux r_u8dq = BODY64("call U8DQ", "xor ax, 0", "xor bx, 0", "xor cx, 0", "xor dx, 0") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8dr(unsigned b);
#pragma aux r_u8dr = BODY64("call U8DR", "xor ax, 0123h", "xor bx, 4567h", "xor cx, 89ABh", "xor dx, 0CDEFh") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8ls(unsigned n);
#pragma aux r_i8ls = BODY64("call I8LS", "xor ax, 68ACh", "xor bx, 0F135h", "xor cx, 79BDh", "xor dx, 0E000h") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8ls(unsigned n);
#pragma aux r_u8ls = BODY64("call U8LS", "xor ax, 68ACh", "xor bx, 0F135h", "xor cx, 79BDh", "xor dx, 0E000h") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_i8rs(unsigned n);
#pragma aux r_i8rs = BODY64("call I8RS", "xor ax, 0", "xor bx, 091Ah", "xor cx, 2B3Ch", "xor dx, 4D5Eh") parm [si] value [ax] modify [ax bx cx dx si di es];
static unsigned r_u8rs(unsigned n);
#pragma aux r_u8rs = BODY64("call U8RS", "xor ax, 0", "xor bx, 091Ah", "xor cx, 2B3Ch", "xor dx, 4D5Eh") parm [si] value [ax] modify [ax bx cx dx si di es];

static int failed;

static void check(const char *name, unsigned changed)
{
    t_puts(name);
    t_puts(changed ? " changed registers or gave a wrong result\r\n" : " ok\r\n");
    if (changed)
        failed = 1;
}

/* The 64-bit operand, b[0] to b[3]: negative, so that the signed divides
 * negate it. b[8] to b[11] are the decoy, 7, 16 bytes past it. */
static unsigned b[12];

/* The offset of the operand, which the checks pass in SI. */
#define OFF(p) ((unsigned)(unsigned long)(p))

static void check64(const char *name, unsigned changed)
{
    if (b[0] != 0x3210 || b[1] != 0x7654 || b[2] != 0xBA98 || b[3] != 0xFEDC
            || b[8] != 7 || b[9] != 0 || b[10] != 0 || b[11] != 0) {
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
    b[8] = 7;
    check("__I4M", r_i4m());
    check("__U4M", r_u4m());
    check("__I4D short", r_i4d_short());
    check("__I4D long", r_i4d_long());
    check("__U4D short", r_u4d_short());
    check("__U4D long", r_u4d_long());
    check64("__I8M", r_i8m(OFF(b)));
    check64("__U8M", r_u8m(OFF(b)));
    check64("__I8DQ", r_i8dq(OFF(b)));
    check64("__I8DR", r_i8dr(OFF(b)));
    check64("__U8DQ", r_u8dq(OFF(b)));
    check64("__U8DR", r_u8dr(OFF(b)));
    check("__I8LS", r_i8ls(13));
    check("__U8LS", r_u8ls(13));
    check("__I8RS", r_i8rs(13));
    check("__U8RS", r_u8rs(13));
    return failed;
}
