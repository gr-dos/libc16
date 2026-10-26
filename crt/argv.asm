; libc16 argc/argv hook (decision 8). The compiler references __argc from
; a module whose main takes parameters, which links this member; it adds
; __libc16_args (args.c) to the startup's init table. __argc itself is
; only a label for that reference, not a variable.

        .8086

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END

_TEXT   segment word public 'CODE'
        public  __argc
        extrn   __libc16_args_:near
__argc  label   byte
_TEXT   ends

LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends

LIBC16_INIT segment word public 'DATA'
        dw      __libc16_args_
LIBC16_INIT ends

LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends

        end
