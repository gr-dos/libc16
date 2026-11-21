; _fmemcpy - libc16
;
; Written from docs/spec/string.md ("Copies and fills") and docs/ABI.md.
;
; void __far *__cdecl _fmemcpy(void __far *dst, const void __far *src,
; size_t n): memcpy with far pointers in every model, so the objects need
; not be in DGROUP. Copies a word at a time with REP MOVSW, then the odd
; byte with MOVSB, from DS:SI to ES:DI, and returns dst in DX:AX. In the
; small model DS is restored; with far data it need not be. SI and DI are
; kept.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __fmemcpy

__fmemcpy proc  CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
ifndef FARDATA
        push    ds
endif
        les     di, [bp+ARG0]           ; dst
        lds     si, [bp+ARG0+4]         ; src
        mov     cx, [bp+ARG0+8]         ; n
        mov     ax, di
        mov     dx, es
        cld
        shr     cx, 1                   ; words; CF = the odd byte
        rep     movsw
        jnc     done
        movsb
done:
ifndef FARDATA
        pop     ds
endif
        pop     di
        pop     si
        pop     bp
        ret
__fmemcpy endp

_TEXT   ends

        end
