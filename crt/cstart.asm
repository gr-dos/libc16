; libc16 core startup for small-model DOS .EXE programs (DESIGN.md §6,
; decision 8). The compiler references _cstart_ from the module that
; defines main, which links this member and nothing else.
;
; DOS starts the program with DS = ES = PSP and SS:SP at the top of STACK.
; The startup
;   - points DS and SS at DGROUP, keeping the SP the linker chose, so that
;     `option stack=` still sets the stack size;
;   - shrinks the program's memory block to the top of the stack;
;   - zeroes BSS, _edata to _end (both defined by wlink), and records the
;     PSP segment in _psp;
;   - calls each init routine in the LIBC16_INIT table;
;   - calls main_ (register convention: AX = argc, DX = argv, both 0 unless
;     the argv member is linked) and terminates with its return value as
;     the exit status.
;
; Init routines are near procedures that take no arguments and preserve
; SI, DI, BP, DS and SS; a C function with the default convention does.
; A member adds one by contributing a near pointer to LIBC16_INIT,
; declaring LIBC16_INIT_BEGIN, LIBC16_INIT and LIBC16_INIT_END in that
; order so the table stays between its two markers whichever member the
; linker reads first.
;
; Needs DOS 2.0: INT 21h functions 4Ah and 4Ch.

        .8086
        dosseg

DGROUP  group   CONST, CONST2, _DATA
DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END
DGROUP  group   _BSS, STACK

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT, ds:DGROUP

        public  _cstart_
        extrn   main_:near
        extrn   _edata:byte, _end:byte

_cstart_:
        mov     ax, DGROUP
        mov     ds, ax

        ; SP relative to DGROUP: (SS - DGROUP) * 16 + SP. 0 stands for
        ; 10000h, a stack that ends exactly at the top of a 64 KiB DGROUP.
        mov     bx, ss
        sub     bx, ax
        mov     cl, 4
        shl     bx, cl
        add     bx, sp
        cli
        mov     ss, ax
        mov     sp, bx
        sti

        ; Keep the paragraphs from the PSP to the top of the stack:
        ; DGROUP - PSP + ((SP - 1) >> 4) + 1, which handles SP = 0 too.
        dec     bx
        shr     bx, cl
        inc     bx
        add     bx, ax
        mov     dx, es
        sub     bx, dx
        mov     ah, 4Ah
        int     21h

        push    ds
        pop     es
        cld
        mov     di, offset DGROUP:_edata
        mov     cx, offset DGROUP:_end
        sub     cx, di
        xor     ax, ax
        rep     stosb
        mov     __psp, dx

        mov     si, offset DGROUP:init_begin
init:   cmp     si, offset DGROUP:init_end
        jae     run
        call    word ptr [si]
        inc     si
        inc     si
        jmp     init

run:    mov     ax, ___libc16_argc
        mov     dx, ___libc16_argv
        call    main_
        mov     ah, 4Ch
        int     21h

_TEXT   ends

CONST   segment word public 'DATA'
CONST   ends

CONST2  segment word public 'DATA'
CONST2  ends

_DATA   segment word public 'DATA'
_DATA   ends

LIBC16_INIT_BEGIN segment word public 'DATA'
init_begin label word
LIBC16_INIT_BEGIN ends

LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends

LIBC16_INIT_END segment word public 'DATA'
init_end label word
LIBC16_INIT_END ends

_BSS    segment word public 'BSS'
        public  __psp, ___libc16_argc, ___libc16_argv
__psp           dw ?            ; unsigned _psp
___libc16_argc  dw ?            ; int __libc16_argc, set by the argv member
___libc16_argv  dw ?            ; char **__libc16_argv, likewise
_BSS    ends

STACK   segment para stack 'STACK'
        db      2048 dup (?)
STACK   ends

        end     _cstart_
