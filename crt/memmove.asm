; memmove - libc16
;
; Written from docs/spec/string.md ("Copies and fills") and docs/ABI.md.
;
; void *__cdecl memmove(void *dst, const void *src, size_t n): copies n
; bytes from src to dst as if through a temporary, and returns dst.
; Overlapping objects are one object, so their pointers share a segment
; and comparing the offsets is enough. When dst's offset is at most
; src's, the copy is upwards, as memcpy's: REP MOVSW, then the odd byte
; with MOVSB. Otherwise it is downwards with the direction flag set: the
; odd last byte first with MOVSB, then REP MOVSW from the last word down;
; CLD clears the flag again before the return. Each word is read before
; it is written, so a word move is safe at any overlap, even of one byte.
; In the small model ES is set to DS, which is kept; with far data DS is
; not restored. SI and DI are kept.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _memmove

_memmove proc   CDIST
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
        cmp     di, si
        jbe     up
; Downwards: SI and DI to the last byte.
        add     si, cx
        dec     si
        add     di, cx
        dec     di
        std
        shr     cx, 1                   ; words; CF = the odd byte
        jnc     words
        movsb                           ; the last byte
words:
        dec     si                      ; to the last word's low byte
        dec     di
        rep     movsw
        cld
        jmp     short done
up:
        shr     cx, 1
        rep     movsw
        jnc     done
        movsb
done:
        pop     di
        pop     si
        pop     bp
        ret
_memmove endp

_TEXT   ends

        end
