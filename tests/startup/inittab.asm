; Linked with tests/startup/init.c: two entries of the init table
; (docs/spec/startup.md, "Init table"), near or far like the routines
; (far in the large model). The first, here, sets init_order to 1 and,
; with far data, returns with DS = 0, which the startup must not rely on
; (spec, "Memory models"). The second is init.c's C routine.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END

        extrn   c_init_:CDIST
        extrn   _init_order:word        ; in init.c, in DGROUP

; One entry per line: JWasm gives every far pointer in a list such as
; "dd asm_init, c_init_" the frame of the first, _TEXT, which is wrong for
; c_init_ in the large model once code segments do not share a frame.
LIBC16_INIT segment
ifdef FARCODE
        dd      asm_init
        dd      c_init_
else
        dw      asm_init
        dw      c_init_
endif
LIBC16_INIT ends

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:DGROUP

asm_init proc   CDIST
        mov     ss:_init_order, 1
ifdef FARDATA
        xor     ax, ax
        mov     ds, ax                  ; not DGROUP
endif
        ret
asm_init endp

_TEXT   ends

        end
