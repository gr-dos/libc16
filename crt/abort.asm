; abort - libc16
;
; Written from docs/spec/stdlib.md ("abort") and docs/spec/signal.md
; ("abort").
;
; void __cdecl abort(void): if the communal variable ___libc16_sigabrt
; (crt/sigtab.asm) is not 0, calls it, with no arguments (signal sets it
; to a routine that delivers SIGABRT to a handler). Then writes "abnormal
; program termination" and CR LF to handle 2 with INT 21h 40h, ignoring an
; error, and ends the program with INT 21h 4Ch, exit code 3. Runs no atexit
; function and links no other member; needs no startup.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
CONST   segment word public 'DATA'
CONST   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   CONST, _BSS

ifdef FARCODE
        comm    near ___libc16_sigabrt:dword
else
        comm    near ___libc16_sigabrt:word
endif

CONST   segment
msg     db      'abnormal program termination', 13, 10
MSGLEN  equ     $ - msg
CONST   ends

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP

; DS is DGROUP on entry to a __cdecl function in every model.
        public  _abort
_abort  proc    CDIST
        mov     ax, word ptr ___libc16_sigabrt
ifdef FARCODE
        or      ax, word ptr ___libc16_sigabrt+2
        jz      @F
        call    dword ptr ___libc16_sigabrt
else
        test    ax, ax
        jz      @F
        call    ax
endif
ifdef FARDATA
        push    ss                      ; a handler may change DS
        pop     ds
endif
@@:     mov     dx, offset DGROUP:msg
        mov     cx, MSGLEN
        mov     bx, 2
        mov     ah, 40h
        int     21h
        mov     ax, 4C03h
        int     21h
_abort  endp

_TEXT   ends

        end
