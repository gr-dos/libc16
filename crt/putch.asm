; putch - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _putch

; int __cdecl putch(int c)
; INT 21h function 06h writes DL, except that DL = 0FFh asks for input:
; that byte is written with function 02h.
_putch  proc    CDIST
        push    bp
        mov     bp, sp
        mov     dl, [bp+ARG0]
        mov     ah, 06h
        cmp     dl, 0ffh
        jne     write
        mov     ah, 02h
write:  int     21h
        mov     al, [bp+ARG0]
        xor     ah, ah
        pop     bp
        ret
_putch  endp

_TEXT   ends

        end
