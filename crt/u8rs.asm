; libc16 64-bit logical right shift: __U8RS (unsigned long long >>).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines").
;
; In:  AX:BX:CX:DX = a (AX the high word), SI = n
; Out: AX:BX:CX:DX = a >> n, filled with zeros; all else preserved

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __U8RS

__U8RS:
        push    si
        or      si, si
        jz      done
next_bit:
        shr     ax, 1
        rcr     bx, 1
        rcr     cx, 1
        rcr     dx, 1
        dec     si
        jnz     next_bit
done:
        pop     si
        ret
_TEXT   ends

        end
