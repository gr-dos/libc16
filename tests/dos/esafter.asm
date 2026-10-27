; A helper for tests/dos: calls int86x with a chosen ES, so that the tests
; can check that int86x restores the caller's ES (docs/spec/dos.md).
;
;   unsigned __cdecl es_after_int86x(unsigned es, int intno, union REGS *in,
;                                    union REGS *out, struct SREGS *segs);
;       set ES to es, call int86x(intno, in, out, segs), and return ES
;       as int86x left it

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _es_after_int86x
        extrn   _int86x:near

_es_after_int86x proc near
        push    bp
        mov     bp, sp
        push    es
        mov     es, [bp+4]
        push    word ptr [bp+12]
        push    word ptr [bp+10]
        push    word ptr [bp+8]
        push    word ptr [bp+6]
        call    _int86x
        add     sp, 8
        mov     ax, es
        pop     es
        pop     bp
        ret
_es_after_int86x endp

_TEXT   ends

        end
