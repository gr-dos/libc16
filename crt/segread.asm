; segread - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _segread

; void __cdecl segread(struct SREGS *segs)
; struct SREGS: es, cs, ss, ds at offsets 0 to 6.
_segread proc   CDIST
        push    bp
        mov     bp, sp
ifdef FARDATA
        push    es
        push    es
        les     bx, [bp+ARG0]
        pop     word ptr es:[bx]        ; the caller's ES
        mov     es:[bx+2], cs
        mov     es:[bx+4], ss
        mov     es:[bx+6], ds
        pop     es
else
        mov     bx, [bp+ARG0]
        mov     [bx], es
        mov     [bx+2], cs
        mov     [bx+4], ss
        mov     [bx+6], ds
endif
        pop     bp
        ret
_segread endp

_TEXT   ends

        end
