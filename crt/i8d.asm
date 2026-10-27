; libc16 64-bit signed divide: __I8DQ (long long /) and __I8DR
; (long long %).
;
; Written from docs/spec/arith.md (C99 6.5.5: quotients truncate toward
; zero, remainders take the dividend's sign) and the call-site experiments
; recorded in docs/ABI.md ("Compiler support routines"). Divides the
; magnitudes with the unsigned core (crt/u8div.asm), then fixes the sign.
; LLONG_MIN / -1 gives LLONG_MIN.
;
; In:  AX:BX:CX:DX = dividend (AX the high word), SI -> divisor (8 bytes,
;      low word first) in DGROUP, at DS:SI in the small model and SS:SI
;      in the compact and large models; the divisor is left unchanged
; Out: AX:BX:CX:DX = quotient (__I8DQ) or remainder (__I8DR); all else
;      preserved
; Called near, or far in the large model. With far data, DS is set to
; SS (DGROUP) while the divisor and the stack copies are read, and restored.

        .8086
        include model.inc

        extrn   __libc16_u8div:CDIST

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I8DQ, __I8DR

; BP's sign bit is the result's sign; CF is set for the remainder.
__I8DR  proc    CDIST
        push    bp
        mov     bp, ax                  ; the dividend's sign
        stc
        jmp     short divide
__I8DQ  label   CDIST
        push    bp
        mov     bp, ax
ifdef FARDATA
        xor     bp, ss:[si+6]           ; the signs differ; clears CF
else
        xor     bp, [si+6]              ; the signs differ; clears CF
endif
divide:
ifdef FARDATA
        push    ds
        push    ss
        pop     ds
endif
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
ifdef FARDATA
        pop     ds
endif
        pop     bp
        ret
__I8DR  endp

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
