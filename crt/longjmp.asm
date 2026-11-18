; longjmp - libc16
;
; Written from docs/spec/setjmp.md and docs/ABI.md ("setjmp and longjmp").
;
; void __cdecl longjmp(jmp_buf env, int val): returns through the setjmp
; that filled env, with SP, BP, SI, DI and DS as env holds them, AX = val
; (1 if val is 0) and the direction flag clear. All of env is read into
; registers before SP changes, so env may be anywhere, even in the frame
; returned to. With far data DS may be anything on entry: env is reached
; with LDS, and the arguments with an SS override.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _longjmp

ENV     equ     ARG0-2                  ; from SP on entry (no BP frame)
VAL     equ     ARG0-2+DPTR

_longjmp proc   CDIST
        mov     bx, sp
ifdef FARDATA
        mov     ax, ss:[bx+VAL]
        lds     si, ss:[bx+ENV]
else
        mov     ax, [bx+VAL]            ; SS = DS = DGROUP
        mov     si, [bx+ENV]
endif
        cmp     ax, 1                   ; 0 becomes 1
        adc     ax, 0
ifdef FARCODE
        les     bx, [si]                ; return address
else
        mov     bx, [si]                ; return offset; the segment is CS
endif
        mov     dx, [si+4]              ; SP
        mov     bp, [si+6]
        mov     di, [si+10]
        mov     cx, [si+12]             ; DS
        mov     si, [si+8]
        mov     ds, cx
        mov     sp, dx
        cld
ifdef FARCODE
        push    es
        push    bx
        retf
else
        jmp     bx
endif
_longjmp endp

_TEXT   ends

        end
