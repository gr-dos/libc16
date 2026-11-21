; memcpy - libc16
;
; Written from docs/spec/string.md ("Copies and fills") and docs/ABI.md.
;
; void *__cdecl memcpy(void *dst, const void *src, size_t n): copies n
; bytes from src to dst a word at a time with REP MOVSW, then the odd
; byte, if any, with MOVSB, and returns dst. n of 0 copies nothing (REP
; with CX = 0). In the small model ES is set to DS, which is kept; with
; far data DS:SI and ES:DI are loaded from the arguments and DS is not
; restored (the ABI lets the callee change it). SI and DI are kept.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _memcpy

_memcpy proc    CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
ifdef FARDATA
        les     di, [bp+ARG0]           ; dst
        lds     si, [bp+ARG0+4]         ; src
        mov     cx, [bp+ARG0+8]         ; n
        mov     dx, es
else
        mov     di, [bp+ARG0]
        mov     si, [bp+ARG0+2]
        mov     cx, [bp+ARG0+4]
        mov     ax, ds
        mov     es, ax
endif
        mov     ax, di                  ; the result
        cld
        shr     cx, 1                   ; words; CF = the odd byte
        rep     movsw
        jnc     done
        movsb
done:
        pop     di
        pop     si
        pop     bp
        ret
_memcpy endp

_TEXT   ends

        end
