; libc16 64-bit unsigned division core, shared by __U8DQ, __U8DR, __I8DQ
; and __I8DR (crt/u8d.asm, crt/i8d.asm). Private to libc16.
;
; Written from docs/spec/arith.md. Restoring binary division, one quotient
; bit per step: small rather than fast.
;
; In:  DS:DI -> dividend (8 bytes, low word first), DS:SI -> divisor
;      (8 bytes, left unchanged)
; Out: [DI] = quotient, AX:BX:CX:DX = remainder (AX the high word);
;      SI, DI, BP, DS and ES preserved
; A zero divisor gives a quotient of all ones and the dividend as the
; remainder.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __libc16_u8div

__libc16_u8div:
        push    bp
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
        pop     bp
        ret
_TEXT   ends

        end
