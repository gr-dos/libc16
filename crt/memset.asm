; memset - libc16
;
; Written from docs/spec/string.md ("Copies and fills") and docs/ABI.md.
;
; void *__cdecl memset(void *s, int c, size_t n): stores c converted to
; unsigned char in the n bytes at s, a word at a time with REP STOSW, then
; the odd byte, if any, with STOSB, and returns s. n of 0 stores nothing.
; ES:DI is s (ES = DS in the small model, where DS is not touched); DI is
; kept.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _memset

_memset proc    CDIST
        push    bp
        mov     bp, sp
        push    di
ifdef FARDATA
        les     di, [bp+ARG0]           ; s
        mov     al, [bp+ARG0+4]         ; c, low byte
        mov     cx, [bp+ARG0+6]         ; n
else
        mov     di, [bp+ARG0]
        mov     al, [bp+ARG0+2]
        mov     cx, [bp+ARG0+4]
        mov     dx, ds
        mov     es, dx
endif
        mov     ah, al
        mov     bx, di                  ; the result
        cld
        shr     cx, 1                   ; words; CF = the odd byte
        rep     stosw
        jnc     done
        stosb
done:
        xchg    ax, bx
ifdef FARDATA
        mov     dx, es
endif
        pop     di
        pop     bp
        ret
_memset endp

_TEXT   ends

        end
