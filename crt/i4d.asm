; libc16 32-bit signed divide: __I4D (long / and %).
;
; Written from docs/spec/arith.md (C99 6.5.5: quotients truncate toward
; zero, remainders take the dividend's sign) and the call-site experiments
; recorded in docs/ABI.md ("Compiler support routines"). Divides the
; magnitudes with __U4D and then fixes the signs. LONG_MIN / -1 gives
; LONG_MIN; a zero divisor raises the divide-error trap (INT 0).
;
; In:  DX:AX = dividend, CX:BX = divisor
; Out: DX:AX = quotient, CX:BX = remainder; all else preserved
; Called near, or far in the large model.

        .8086
        include model.inc

        extrn   __U4D:CDIST

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I4D

__I4D   proc    CDIST
        push    si
        push    di
        mov     si, dx                  ; SI sign: the remainder's
        mov     di, dx
        xor     di, cx                  ; DI sign: the quotient's
        or      dx, dx
        jns     dividend_positive
        neg     dx
        neg     ax
        sbb     dx, 0
dividend_positive:
        or      cx, cx
        jns     divisor_positive
        neg     cx
        neg     bx
        sbb     cx, 0
divisor_positive:
        call    __U4D
        or      di, di
        jns     quotient_done
        neg     dx
        neg     ax
        sbb     dx, 0
quotient_done:
        or      si, si
        jns     remainder_done
        neg     cx
        neg     bx
        sbb     cx, 0
remainder_done:
        pop     di
        pop     si
        ret
__I4D   endp
_TEXT   ends

        end
