; segread - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _segread

; void __cdecl segread(struct SREGS *segs)
; struct SREGS: es, cs, ss, ds at offsets 0 to 6.
_segread proc   near
        push    bp
        mov     bp, sp
        mov     bx, [bp+4]
        mov     [bx], es
        mov     [bx+2], cs
        mov     [bx+4], ss
        mov     [bx+6], ds
        pop     bp
        ret
_segread endp

_TEXT   ends

        end
