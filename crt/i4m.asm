; libc16 32-bit multiply: __I4M (long) and __U4M (unsigned long).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines"). The low 32 bits of a
; product are the same for signed and unsigned operands, so both symbols
; name one routine.
;
; In:  DX:AX = a, CX:BX = b
; Out: DX:AX = a * b mod 2^32; BX and CX changed; all else preserved
; Called near, or far in the large model.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I4M, __U4M

; a * b = alo*blo + ((ahi*blo + alo*bhi) mod 2^16) * 2^16
__I4M   proc    CDIST
__U4M   label   CDIST
        push    si
        xchg    si, ax                  ; SI = alo
        xchg    ax, dx                  ; AX = ahi
        mul     bx                      ; AX = ahi*blo (low word)
        xchg    ax, cx                  ; CX = ahi*blo, AX = bhi
        mul     si                      ; AX = bhi*alo (low word)
        add     cx, ax
        xchg    ax, si                  ; AX = alo
        mul     bx                      ; DX:AX = alo*blo
        add     dx, cx
        pop     si
        ret
__I4M   endp
_TEXT   ends

        end
