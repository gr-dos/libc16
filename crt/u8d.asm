; libc16 64-bit unsigned divide: __U8DQ (unsigned long long /) and
; __U8DR (unsigned long long %).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines"). The division itself is
; the shared core, crt/u8div.asm.
;
; In:  AX:BX:CX:DX = dividend (AX the high word), DS:SI -> divisor
;      (8 bytes, low word first), which is left unchanged
; Out: AX:BX:CX:DX = quotient (__U8DQ) or remainder (__U8DR); all else
;      preserved

        .8086

        extrn   __libc16_u8div:near

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __U8DQ, __U8DR

__U8DQ:
        push    di
        push    ax                      ; the dividend, at SP, becomes
        push    bx                      ; the quotient
        push    cx
        push    dx
        mov     di, sp
        call    __libc16_u8div
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     di
        ret

__U8DR:
        push    di
        push    ax
        push    bx
        push    cx
        push    dx
        mov     di, sp
        call    __libc16_u8div
        add     sp, 8
        pop     di
        ret
_TEXT   ends

        end
