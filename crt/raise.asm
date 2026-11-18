; raise - libc16
;
; Written from docs/spec/signal.md and docs/ABI.md.
;
; int __cdecl raise(int sig): delivers sig by its disposition
; (crt/sigtab.asm) and returns 0. SIG_IGN does nothing; a handler has the
; disposition set to SIG_DFL and is called with sig; SIG_DFL ends the
; program with INT 21h 4Ch, exit code 3, running nothing else. A sig
; outside 1 to 31 returns -1 with errno EINVAL.

        .8086
        include model.inc

NSIG_   equ     32
EINVAL_ equ     22

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

_BSS    segment
        extrn   ___libc16_sigtab:word, _errno:word
_BSS    ends

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP

; DS is DGROUP on entry to a __cdecl function in every model.
        public  _raise
_raise  proc    CDIST
        mov     bx, sp
        mov     ax, ss:[bx+ARG0-2]      ; sig
        mov     bx, ax
        dec     bx
        cmp     bx, NSIG_ - 1
        jae     bad
ifdef FARCODE
        shl     bx, 1
endif
        shl     bx, 1
        mov     dx, ___libc16_sigtab[bx]
ifdef FARCODE
        mov     cx, ___libc16_sigtab[bx+2]
        test    cx, cx
        jnz     go                      ; a segment: a handler
endif
        cmp     dx, 1
        ja      go
        je      ok                      ; SIG_IGN
        mov     ax, 4C03h               ; SIG_DFL
        int     21h
go:     mov     ___libc16_sigtab[bx], 0
ifdef FARCODE
        mov     ___libc16_sigtab[bx+2], 0
        push    cx                      ; the handler, under its argument
        push    dx
        push    ax
        mov     bx, sp
        call    dword ptr ss:[bx+2]
        add     sp, 6
else
        push    ax
        call    dx
        pop     ax
endif
ok:     xor     ax, ax
        ret
bad:    mov     _errno, EINVAL_
        mov     ax, -1
        ret
_raise  endp

_TEXT   ends

        end
