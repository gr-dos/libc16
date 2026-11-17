; Linked with tests/startup/fini.c and finiex.c: two entries of the
; termination table (docs/spec/startup.md, "Termination table"), near or
; far like the routines (far in the large model). The first, here, records
; that it ran in fini_order and, with far data, returns with DS = 0, which
; the startup and exit must not rely on. The second is the C routine
; c_fini, which prints what it saw.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
LIBC16_EXIT_BEGIN segment word public 'DATA'
LIBC16_EXIT_BEGIN ends
LIBC16_EXIT segment word public 'DATA'
LIBC16_EXIT ends
LIBC16_EXIT_END segment word public 'DATA'
LIBC16_EXIT_END ends

DGROUP  group   LIBC16_EXIT_BEGIN, LIBC16_EXIT, LIBC16_EXIT_END

        extrn   c_fini_:CDIST
        extrn   _fini_order:word        ; in fini.c or finiex.c, in DGROUP

; One entry per line (see tests/startup/inittab.asm).
LIBC16_EXIT segment
ifdef FARCODE
        dd      asm_fini
        dd      c_fini_
else
        dw      asm_fini
        dw      c_fini_
endif
LIBC16_EXIT ends

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:DGROUP

; fini_order = fini_order * 10 + 1, so that a second call shows
asm_fini proc   CDIST
        mov     ax, ss:_fini_order
        mov     cx, 10
        mul     cx
        inc     ax
        mov     ss:_fini_order, ax
ifdef FARDATA
        xor     ax, ax
        mov     ds, ax                  ; not DGROUP
endif
        ret
asm_fini endp

_TEXT   ends

        end
