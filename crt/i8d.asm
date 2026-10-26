; libc16 64-bit signed divide: __I8DQ (long long /) and __I8DR
; (long long %).
;
; Written from docs/spec/arith.md (C99 6.5.5: quotients truncate toward
; zero, remainders take the dividend's sign) and the call-site experiments
; recorded in docs/ABI.md ("Compiler support routines"). Divides the
; magnitudes with the unsigned core (crt/u8div.asm), then fixes the sign.
; LLONG_MIN / -1 gives LLONG_MIN.
;
; In:  AX:BX:CX:DX = dividend (AX the high word), DS:SI -> divisor
;      (8 bytes, low word first), which is left unchanged
; Out: AX:BX:CX:DX = quotient (__I8DQ) or remainder (__I8DR); all else
;      preserved

        .8086

        extrn   __libc16_u8div:near

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I8DQ, __I8DR

; BP's sign bit is the result's sign; CF is set for the remainder.
__I8DR:
        push    bp
        mov     bp, ax                  ; the dividend's sign
        stc
        jmp     short divide
__I8DQ:
        push    bp
        mov     bp, ax
        xor     bp, [si+6]              ; the signs differ; clears CF
divide:
        push    si
        push    di
        sbb     di, di
        push    di                      ; -1 for the remainder, 0 for the quotient
        or      ax, ax
        jns     dividend_positive
        call    negate
dividend_positive:
        push    ax                      ; |dividend|, at SP
        push    bx
        push    cx
        push    dx
        mov     di, sp
        mov     ax, [si+6]
        mov     bx, [si+4]
        mov     cx, [si+2]
        mov     dx, [si]
        or      ax, ax
        jns     divisor_positive
        call    negate
divisor_positive:
        push    ax                      ; |divisor|, at SP
        push    bx
        push    cx
        push    dx
        mov     si, sp
        call    __libc16_u8div          ; remainder in registers, quotient at [di]
        add     sp, 8
        cmp     byte ptr [di+8], 0      ; the flag
        jne     remainder
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        jmp     short result
remainder:
        add     sp, 8
result:
        or      bp, bp
        jns     positive
        call    negate
positive:
        pop     di                      ; the flag
        pop     di
        pop     si
        pop     bp
        ret

; AX:BX:CX:DX = -AX:BX:CX:DX, as the complement plus 1
negate:
        not     ax
        not     bx
        not     cx
        neg     dx                      ; CF is set unless DX was 0, which
        cmc                             ; is when the + 1 carries
        adc     cx, 0
        adc     bx, 0
        adc     ax, 0
        ret
_TEXT   ends

        end
