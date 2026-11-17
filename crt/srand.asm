; srand - libc16
;
; Written from docs/spec/stdlib.md ("rand, srand") and docs/ABI.md.
;
; void srand(unsigned seed), __cdecl (symbol _srand): sets rand's state x
; to seed, zero-extended. The state is the communal variable rand
; (crt/rand.asm) declares too, holding x xor 1. Called near, or far in the
; large model (crt/model.inc), so seed is at [SP+2] or [SP+4].

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

        comm    near ___libc16_rand:dword       ; x xor 1, low word first

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP

; DS is DGROUP on entry to a __cdecl function in every model.
        public  _srand
_srand  proc    CDIST
        mov     bx, sp
        mov     ax, ss:[bx+ARG0-2]
        xor     al, 1
        mov     word ptr ___libc16_rand, ax
        xor     ax, ax
        mov     word ptr ___libc16_rand+2, ax
        ret
_srand  endp

_TEXT   ends

        end
