; libc16 core startup for DOS .EXE programs, in the small, compact and
; large models (crt/model.inc).
;
; Written from docs/spec/startup.md ("Core startup", "Memory models"),
; docs/ABI.md and the DOS .EXE load state and INT 21h functions 4Ah and
; 4Ch as documented in Ralf Brown's Interrupt List. Needs DOS 2.0 and an 8086.
;
; Every program links this member, so it is kept as small as it can be
; while still doing everything the spec requires.

        .8086
        include model.inc
        dosseg

; Segment order (spec, "Segments"). The code segment matches the one wcc
; emits; the data segments are listed in DGROUP order.

_TEXT   segment byte public 'CODE'
_TEXT   ends

; The start of DGROUP, so that no C object is at offset 0, where a pointer
; to it would be a null pointer (spec, "Null pointer"). Under dosseg the
; linker puts class BEGDATA first in DGROUP, even before the DATA
; segments of objects it sees before this one.
LIBC16_NULL segment word public 'BEGDATA'
        dw      0
LIBC16_NULL ends

CONST   segment word public 'DATA'
CONST   ends
CONST2  segment word public 'DATA'
CONST2  ends
_DATA   segment word public 'DATA'
_DATA   ends

; The init table (spec, "Init table"): near pointers, or far ones in the
; large model. All three segments are word aligned so that no padding can
; fall between the start label and the first entry.
LIBC16_INIT_BEGIN segment word public 'DATA'
init_first label word
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
init_limit label word
LIBC16_INIT_END ends

_BSS    segment word public 'BSS'
        public  __psp, ___libc16_argc, ___libc16_argv
__psp           dw ?            ; C _psp: PSP segment
___libc16_argc  dw ?            ; C __libc16_argc: argc for main
ifdef FARDATA
___libc16_argv  dd ?            ; C __libc16_argv: argv for main, offset first
else
___libc16_argv  dw ?            ; C __libc16_argv: argv for main
endif
_BSS    ends

; Default stack; wlink's "option stack=n" resizes it.
STACK   segment para stack 'STACK'
        db      2048 dup (?)
STACK   ends

; An empty segment after STACK, so that its label is the top of the stack
; whatever size the linker gives STACK. It adds no bytes to a program;
; the heap uses the label as its start.
LIBC16_STACK_END segment byte public 'STACK'
        public  ___libc16_stack_top
___libc16_stack_top label byte  ; C __libc16_stack_top: top of the stack
LIBC16_STACK_END ends

DGROUP  group   LIBC16_NULL, CONST, CONST2, _DATA, LIBC16_INIT_BEGIN, \
                LIBC16_INIT, LIBC16_INIT_END, _BSS, STACK, LIBC16_STACK_END

        extrn   main_:CDIST
        extrn   _edata:byte, _end:byte      ; defined by wlink

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP, ss:nothing, es:nothing
        public  _cstart_

; On entry DS = ES = PSP, and SS:SP is the top of STACK as the linker
; placed it.
_cstart_:
        cld                             ; DOS leaves DF unspecified
        mov     dx, ds                  ; DX = PSP until _psp is set
        mov     ax, DGROUP
        mov     ds, ax                  ; step 1

; Step 2: SS = DGROUP with the same linear stack top. The new SP is
; (SS - DGROUP) * 16 + SP; it wraps to 0 only for a full 64 KiB DGROUP.
; An 8088 does not reliably hold off interrupts after a move to SS, so
; they stay disabled until SS and SP agree again.
        mov     bx, ss
        sub     bx, ax
        mov     cl, 4
        shl     bx, cl
        add     bx, sp
        cli
        mov     ss, ax
        mov     sp, bx
        sti

; Step 3: resize the block (ES is still the PSP) to end at the stack top:
; (DGROUP - PSP) paragraphs plus the stack top rounded up to a paragraph.
; Computing that as ((SP - 1) >> 4) + 1 also gives 1000h when SP is 0.
; A shrink cannot fail, so the result is not checked.
        dec     bx
        shr     bx, cl
        inc     bx
        sub     ax, dx
        add     bx, ax
        mov     ah, 4Ah
        int     21h

; Step 4: zero BSS, then record the PSP in it.
        push    ds
        pop     es
        mov     di, offset DGROUP:_edata
        mov     cx, offset DGROUP:_end
        sub     cx, di
        xor     al, al
        rep     stosb
        mov     __psp, dx

; Step 5: call each init entry in link order. Init routines preserve SI
; and SS; with far data they may change DS, so the table is read through
; SS.
        mov     si, offset DGROUP:init_first
next_init:
        cmp     si, offset DGROUP:init_limit
        jae     run_main
ifdef FARCODE
        call    dword ptr ss:[si]
        add     si, 4
elseifdef FARDATA
        call    word ptr ss:[si]
        inc     si
        inc     si
else
        call    word ptr [si]
        inc     si
        inc     si
endif
        jmp     next_init

; Steps 6 and 7: main_ under the register convention, then exit with the
; low byte of its result. With far data DS is DGROUP again first, and
; argv is in CX:BX.
run_main:
ifdef FARDATA
        push    ss
        pop     ds
        mov     ax, ___libc16_argc
        mov     bx, word ptr ___libc16_argv
        mov     cx, word ptr ___libc16_argv+2
else
        mov     ax, ___libc16_argc
        mov     dx, ___libc16_argv
endif
        call    main_
        mov     ah, 4Ch
        int     21h
_TEXT   ends

        end     _cstart_
