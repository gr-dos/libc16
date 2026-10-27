; kbhit - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _kbhit

; int __cdecl kbhit(void)
; INT 21h function 0Bh: AL = 0FFh if a character is waiting, else 0.
_kbhit  proc    near
        mov     ah, 0bh
        int     21h
        xor     ah, ah
        ret
_kbhit  endp

_TEXT   ends

        end
