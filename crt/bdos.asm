; bdos - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _bdos

; int __cdecl bdos(int dosfn, unsigned dosdx, unsigned dosal)
_bdos   proc    CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds
        mov     ah, [bp+ARG0]
        mov     dx, [bp+ARG0+2]
        mov     al, [bp+ARG0+4]
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
