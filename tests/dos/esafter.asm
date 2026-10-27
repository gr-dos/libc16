; A helper for tests/dos: calls int86x with a chosen ES, so that the tests
; can check that int86x restores the caller's ES (docs/spec/dos.md).
;
;   unsigned __cdecl es_after_int86x(unsigned es, int intno, union REGS *in,
;                                    union REGS *out, struct SREGS *segs);
;       set ES to es, call int86x(intno, in, out, segs), and return ES
;       as int86x left it

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _es_after_int86x
        extrn   _int86x:CDIST

; The arguments after es, copied for int86x: intno and three pointers.
NARGS   equ     2+3*DPTR

_es_after_int86x proc CDIST
        push    bp
        mov     bp, sp
        push    es
        mov     es, [bp+ARG0]
off     =       NARGS
        rept    NARGS/2
off     =       off-2
        push    word ptr [bp+ARG0+2+off]
        endm
        call    _int86x
        add     sp, NARGS
        mov     ax, es
        pop     es
        pop     bp
        ret
_es_after_int86x endp

_TEXT   ends

        end
