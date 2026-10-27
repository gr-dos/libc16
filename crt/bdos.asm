; bdos - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _bdos

; int __cdecl bdos(int dosfn, unsigned dosdx, unsigned dosal)
_bdos   proc    near
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds
        mov     ah, [bp+4]
        mov     dx, [bp+6]
        mov     al, [bp+8]
        push    bp
        int     21h
        pop     bp
        pop     ds
        pop     di
        pop     si
        cld
        pop     bp
        ret
_bdos   endp

_TEXT   ends

        end
