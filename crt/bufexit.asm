; bufexit - libc16 (docs/spec/stdio.md, "Buffering"; docs/spec/startup.md,
; "Termination table")
;
; The termination routine of buffered output: one entry in the
; termination table, linked with the engine (crt/bufio.c), which reads
; ___libc16_bufdone. The routine sets ___libc16_bufdone, so that no
; stream gets a buffer from then on, and calls __libc16_buffini, which
; writes every stream's pending output and makes the streams unbuffered.
; It may run more than once (exit from a later routine walks the table
; again); a second run finds nothing to write.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
; The init segments first, as in crt/atexit.asm, so that the termination
; table follows the init table whichever module the linker sees first.
LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends
LIBC16_EXIT_BEGIN segment word public 'DATA'
LIBC16_EXIT_BEGIN ends
LIBC16_EXIT segment word public 'DATA'
LIBC16_EXIT ends
LIBC16_EXIT_END segment word public 'DATA'
LIBC16_EXIT_END ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END, \
                LIBC16_EXIT_BEGIN, LIBC16_EXIT, LIBC16_EXIT_END, _BSS

        extrn   __libc16_buffini_:CDIST

_BSS    segment
        public  ___libc16_bufdone
___libc16_bufdone db    ?
_BSS    ends

LIBC16_EXIT segment
ifdef FARCODE
        dd      bufexit
else
        dw      bufexit
endif
LIBC16_EXIT ends

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:DGROUP

; Called with SS = DGROUP, the direction flag clear and, in the small
; model, DS = DGROUP. C code expects DS = DGROUP in every model, and
; preserves SI, DI, BP and SS (and DS in the small model).
bufexit proc    CDIST
ifdef FARDATA
        push    ss
        pop     ds
endif
        mov     ss:___libc16_bufdone, 1
        call    __libc16_buffini_
        ret
bufexit endp

_TEXT   ends

        end
