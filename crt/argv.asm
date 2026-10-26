; libc16 argc/argv hook (decision 8). The compiler references __argc from
; a module whose main takes parameters, which links this member; it adds
; __libc16_args (args.c) to the startup's init table. __argc itself is
; only a label for that reference, not a variable.

        .8086

DGROUP  group   XIB, XI, XIE

_TEXT   segment word public 'CODE'
        public  __argc
        extrn   __libc16_args_:near
__argc  label   byte
_TEXT   ends

XIB     segment word public 'DATA'
XIB     ends

XI      segment word public 'DATA'
        dw      __libc16_args_
XI      ends

XIE     segment word public 'DATA'
XIE     ends

        end
