; Linked with tests/setjmp/setjmp.c (docs/spec/setjmp.md).
;
; int sj_regs(void) (default convention): calls setjmp with marker values
; in SI, DI and BP, changes them and calls longjmp(env, 7). Returns 0 if
; setjmp then returned 7 with the markers back and SP as after its first
; return, or a nonzero code saying what was wrong.
;
; void __cdecl sj_dirty(jmp_buf env, int val): sets the direction flag,
; changes BX, CX, DX, SI, DI, BP and ES and, with far data, sets DS to 0,
; then jumps to longjmp with its own arguments, as if it had been called
; instead.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
regs_env dw     8 dup (?)
sp_after dw     ?
_BSS    ends

DGROUP  group   _BSS

        extrn   _setjmp:CDIST
        extrn   _longjmp:CDIST

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:DGROUP

        public  sj_regs_
sj_regs_ proc   CDIST
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    bp
        push    ds
        push    es
        push    ss
        pop     ds                      ; DGROUP, as for any __cdecl call
        mov     si, 1234h
        mov     di, 5678h
        mov     bp, 9ABCh
ifdef FARDATA
        push    ss
endif
        mov     ax, offset DGROUP:regs_env
        push    ax
        call    _setjmp
        add     sp, DPTR
        mov     dx, sp                  ; SP after the return
        test    ax, ax
        jnz     back
        mov     ss:sp_after, dx
        xor     si, si
        mov     di, -1
        mov     bp, 1
        mov     ax, 7
        push    ax
ifdef FARDATA
        push    ss
endif
        mov     ax, offset DGROUP:regs_env
        push    ax
        push    ss
        pop     ds
        call    _longjmp
        mov     ax, 1                   ; longjmp returned
        jmp     short done
back:   mov     bx, ax
        mov     ax, 2
        cmp     bx, 7
        jne     done
        inc     ax
        cmp     si, 1234h
        jne     done
        inc     ax
        cmp     di, 5678h
        jne     done
        inc     ax
        cmp     bp, 9ABCh
        jne     done
        inc     ax
        cmp     dx, ss:sp_after
        jne     done
        xor     ax, ax
done:  pop     es
        pop     ds
        pop     bp
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        ret
sj_regs_ endp

        public  _sj_dirty
_sj_dirty proc  CDIST
        std
        mov     bx, -1
        mov     cx, -1
        mov     dx, -1
        mov     si, -1
        mov     di, -1
        mov     bp, -1
ifdef FARDATA
        xor     ax, ax
        mov     ds, ax
        mov     es, ax
else
        mov     ax, -1
        mov     es, ax
endif
        jmp     _longjmp
_sj_dirty endp

_TEXT   ends

        end
