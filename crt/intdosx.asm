; intdosx - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md: int86x(0x21, in, out,
; segs). The small model has what wcc made of the C call. With far data
; the arguments are copied in a loop rather than pushed one by one.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _intdosx
        extrn   _int86x:CDIST

; int __cdecl intdosx(union REGS *in, union REGS *out, struct SREGS *segs)
_intdosx proc   CDIST
        push    bp
        mov     bp, sp
ifdef FARDATA
        mov     cx, 3*DPTR/2            ; words of in, out and segs
        lea     bx, [bp+ARG0+3*DPTR]    ; past segs: copy from the last word
copy:   dec     bx
        dec     bx
        push    word ptr ss:[bx]
        loop    copy
        mov     ax, 21h
        push    ax
        push    ss                      ; DS = DGROUP for a __cdecl call
        pop     ds
        call    _int86x
        mov     sp, bp
else
        push    word ptr [bp+ARG0+4]
        push    word ptr [bp+ARG0+2]
        push    word ptr [bp+ARG0]
        mov     ax, 21h
        push    ax
        call    _int86x
        add     sp, 8
endif
        pop     bp
        ret
_intdosx endp

_TEXT   ends

        end
