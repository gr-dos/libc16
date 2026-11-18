; Linked with tests/signal/signal.c (docs/spec/signal.md). All __cdecl.
;
; void sg_set_prev(void): sets INT 23h (INT 21h 25h) to sg_prev, the
; test's own handler, which counts its calls in sg_prev_hits and returns
; with IRET, changing nothing else.
;
; unsigned sg_int23(void): sets SI, DI, BP, ES and DS to marker values (DS
; and ES not DGROUP), sets the direction flag and issues INT 23h. Returns 0
; if the markers and the direction flag are as they were after the IRET,
; else a bit for each one that is not (1 SI, 2 DI, 4 BP, 8 ES, 10h DS,
; 20h direction flag).
;
; void sg_int23_ss(void): issues INT 23h with SS one paragraph higher than
; DGROUP and SP 16 less, the same memory, so that SS is not DGROUP.
;
; unsigned sg_flags(void): the flags.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

_BSS    segment
        public  _sg_prev_hits
_sg_prev_hits dw ?
_BSS    ends

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:nothing

sg_prev proc    far
        push    ds
        push    ax
        mov     ax, DGROUP
        mov     ds, ax
        inc     word ptr ds:_sg_prev_hits
        pop     ax
        pop     ds
        iret
sg_prev endp

        public  _sg_set_prev
_sg_set_prev proc CDIST
        push    ds
        push    cs
        pop     ds
        mov     dx, offset sg_prev
        mov     ax, 2523h
        int     21h
        pop     ds
        ret
_sg_set_prev endp

        public  _sg_int23
_sg_int23 proc  CDIST
        push    bp
        push    si
        push    di
        push    ds
        push    es
        mov     si, 1234h
        mov     di, 5678h
        mov     bp, 9ABCh
        mov     ax, 4321h
        mov     es, ax
        mov     ax, 1111h
        mov     ds, ax
        std
        int     23h
        pushf
        pop     bx
        cld
        xor     ax, ax
        cmp     si, 1234h
        je      @F
        or      al, 1
@@:     cmp     di, 5678h
        je      @F
        or      al, 2
@@:     cmp     bp, 9ABCh
        je      @F
        or      al, 4
@@:     mov     cx, es
        cmp     cx, 4321h
        je      @F
        or      al, 8
@@:     mov     cx, ds
        cmp     cx, 1111h
        je      @F
        or      al, 10h
@@:     test    bx, 400h
        jnz     @F
        or      al, 20h
@@:     pop     es
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_sg_int23 endp

        public  _sg_int23_ss
_sg_int23_ss proc CDIST
        push    bp
        push    si
        push    di
        mov     ax, ss
        inc     ax
        mov     dx, sp
        sub     dx, 16
        cli
        mov     ss, ax
        mov     sp, dx
        sti
        int     23h
        mov     ax, ss
        dec     ax
        mov     dx, sp
        add     dx, 16
        cli
        mov     ss, ax
        mov     sp, dx
        sti
        pop     di
        pop     si
        pop     bp
        ret
_sg_int23_ss endp

        public  _sg_flags
_sg_flags proc  CDIST
        pushf
        pop     ax
        ret
_sg_flags endp

_TEXT   ends

        end
