; libc16 64-bit unsigned division core, shared by __U8DQ, __U8DR, __I8DQ
; and __I8DR (crt/u8d.asm, crt/i8d.asm). Private to libc16.
;
; Written from docs/spec/arith.md. Restoring binary division, one quotient
; bit per step: small rather than fast.
;
; In:  DI -> dividend (8 bytes, low word first), SI -> divisor (8 bytes,
;      left unchanged), both at DS in the small model and at SS in the
;      compact and large models
; Out: [DI] = quotient, AX:BX:CX:DX = remainder (AX the high word);
;      SI, DI, BP, DS and ES preserved
; Called near, or far in the large model.
; A zero divisor gives a quotient of all ones and the dividend as the
; remainder.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __libc16_u8div

__libc16_u8div proc CDIST
        push    bp
ifdef FARDATA
        push    ds
        push    ss
        pop     ds
endif
        xor     ax, ax                  ; AX:BX:CX:DX = partial remainder
        xor     bx, bx
        xor     cx, cx
        cwd
        mov     bp, 64                  ; bit count
next_bit:
        shl     word ptr [di], 1        ; next dividend bit into the
        rcl     word ptr [di+2], 1      ; remainder; [di] bit 0 is free for
        rcl     word ptr [di+4], 1      ; the quotient bit
        rcl     word ptr [di+6], 1
        rcl     dx, 1
        rcl     cx, 1
        rcl     bx, 1
        rcl     ax, 1
        jc      subtract                ; 65-bit remainder: above the divisor
        cmp     ax, [si+6]
        jne     compared
        cmp     bx, [si+4]
        jne     compared
        cmp     cx, [si+2]
        jne     compared
        cmp     dx, [si]
compared:
        jb      shifted
subtract:
        sub     dx, [si]
        sbb     cx, [si+2]
        sbb     bx, [si+4]
        sbb     ax, [si+6]
        inc     word ptr [di]           ; quotient bit 1
shifted:
        dec     bp
        jnz     next_bit
ifdef FARDATA
        pop     ds
endif
        pop     bp
        ret
__libc16_u8div endp
_TEXT   ends

        end
