; libc16 64-bit arithmetic right shift: __I8RS (long long >>).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines").
;
; In:  AX:BX:CX:DX = a (AX the high word), SI = n
; Out: AX:BX:CX:DX = a >> n, filled with the sign bit; all else preserved
; Called near, or far in the large model.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I8RS

__I8RS  proc    CDIST
        push    si
        or      si, si
        jz      done
next_bit:
        sar     ax, 1
        rcr     bx, 1
        rcr     cx, 1
        rcr     dx, 1
        dec     si
        jnz     next_bit
done:
        pop     si
        ret
__I8RS  endp
_TEXT   ends

        end
