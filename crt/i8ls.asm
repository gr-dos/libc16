; libc16 64-bit left shift: __I8LS (long long <<) and __U8LS
; (unsigned long long <<).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines"). A left shift is the same
; for signed and unsigned operands, so both symbols name one routine.
;
; In:  AX:BX:CX:DX = a (AX the high word), SI = n
; Out: AX:BX:CX:DX = a << n; all else preserved
; Called near, or far in the large model.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I8LS, __U8LS

__I8LS  proc    CDIST
__U8LS  label   CDIST
        push    si
        or      si, si
        jz      done
next_bit:
        shl     dx, 1
        rcl     cx, 1
        rcl     bx, 1
        rcl     ax, 1
        dec     si
        jnz     next_bit
done:
        pop     si
        ret
__I8LS  endp
_TEXT   ends

        end
