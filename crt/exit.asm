; exit - libc16
;
; Written from docs/spec/startup.md and docs/ABI.md.
;
; void exit(int status), __cdecl (symbol _exit): terminates the program
; with INT 21h function 4Ch, AL = the low byte of status. There is no
; atexit and stdio is unbuffered, so there is nothing to run or flush
; first; DOS closes the program's handles. It does not return.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT

        public  _exit
_exit   proc    near
        mov     bx, sp
        mov     al, ss:[bx+2]           ; low byte of status
        mov     ah, 4ch                 ; terminate with return code
        int     21h
_exit   endp

_TEXT   ends

        end
