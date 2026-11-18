; setjmp - libc16
;
; Written from docs/spec/setjmp.md and docs/ABI.md ("setjmp and longjmp").
;
; int __cdecl setjmp(jmp_buf env): saves the return address (offset, then
; segment: CS in the small and compact models), SP as the return leaves
; it, BP, SI, DI and DS in env, sets its reserved word to 0 and returns 0.
; Changes only AX and the flags. env is near in the small model, where it
; is in DGROUP (= DS), and far with far data.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _setjmp

_setjmp proc    CDIST
        push    bp
        mov     bp, sp
        push    es
        push    di
ifdef FARDATA
        les     di, [bp+ARG0]
else
        push    ds
        pop     es
        mov     di, [bp+ARG0]
endif
        cld
        mov     ax, [bp+2]              ; return offset
        stosw
ifdef FARCODE
        mov     ax, [bp+4]              ; return segment
else
        mov     ax, cs
endif
        stosw
        lea     ax, [bp+ARG0]           ; SP after the return
        stosw
        mov     ax, [bp]                ; caller's BP
        stosw
        mov     ax, si
        stosw
        mov     ax, [bp-4]              ; caller's DI
        stosw
        mov     ax, ds
        stosw
        xor     ax, ax                  ; reserved
        stosw
        pop     di
        pop     es
        pop     bp
        ret
_setjmp endp

_TEXT   ends

        end
