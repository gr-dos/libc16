; rand - libc16
;
; Written from docs/spec/stdlib.md ("rand, srand") and docs/ABI.md.
;
; int rand(void), __cdecl (symbol _rand): x = (1664525 x + 1013904223)
; mod 2^32, returning x >> 17. Called near, or far in the large model
; (crt/model.inc).
;
; The state is shared with srand (crt/srand.asm) as a communal variable,
; which each of them declares, so that neither links the other. A
; communal variable is in BSS and starts at 0, so it holds x xor 1: 0
; is the sequence of srand(1). The multiply is done here, with 16-bit
; MULs, rather than by the compiler's 32-bit helper.

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
        public  _rand
_rand   proc    CDIST
; With x = H:L, 1664525 = 0019h:660Dh: the product mod 2^32 is
; L * 660Dh + ((H * 660Dh + L * 19h) mod 2^16) << 16.
        mov     ax, word ptr ___libc16_rand+2   ; H
        mov     cx, 660Dh
        mul     cx
        xchg    ax, bx                  ; BX = H * 660Dh, low word
        mov     ax, word ptr ___libc16_rand
        xor     al, 1                   ; L
        push    ax
        mov     dx, 19h
        mul     dx
        add     bx, ax                  ; + L * 19h, low word
        pop     ax
        mul     cx                      ; DX:AX = L * 660Dh
        add     dx, bx
        add     ax, 0F35Fh              ; + 1013904223 = 3C6Eh:F35Fh
        adc     dx, 3C6Eh
        mov     word ptr ___libc16_rand+2, dx
        xor     al, 1
        mov     word ptr ___libc16_rand, ax
        xchg    ax, dx
        shr     ax, 1                   ; x >> 17
        ret
_rand   endp

_TEXT   ends

        end
