; libc16 argv hook.
;
; Written from docs/spec/startup.md ("argv hook") and docs/ABI.md. A main
; that takes parameters makes wcc reference __argc; defining it here links
; this member, which adds the command-tail parser to the init table so
; that the core startup never references the parser itself (decision 8).
; The entry is a near pointer, or a far one in the large model
; (crt/model.inc).

        .8086
        include model.inc

; The three init segments, declared in order so that their order holds
; even if the linker sees this module before the core startup.
LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END

        extrn   __libc16_args_:CDIST

LIBC16_INIT segment
        public  __argc
__argc  label   byte            ; no storage: exists only to be referenced
ifdef FARCODE
        dd      __libc16_args_          ; offset, then segment
else
        dw      __libc16_args_
endif
LIBC16_INIT ends

        end
