; getch - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _getch

; int __cdecl getch(void)
; INT 21h function 08h: AL = the character, not echoed.
_getch  proc    CDIST
        mov     ah, 08h
        int     21h
        xor     ah, ah
        ret
_getch  endp

_TEXT   ends

        end
