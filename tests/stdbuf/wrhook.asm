; A hook on INT 21h for tests/stdbuf: counts the "write" calls (function
; 40h) to one handle, and can make them write fewer bytes than asked, as
; DOS does on a full disk, or fail with a chosen DOS error, so that the
; tests can count a stream's DOS writes and reach its error paths.
;
;   void __cdecl wrhook_install(void);   hook INT 21h
;   void __cdecl wrhook_remove(void);    unhook it; call before exiting
;   void __cdecl wrhook_watch(int fd);
;       count the writes to fd from now on, from 0, and let them all
;       through as they are (fd -1: none)
;   unsigned __cdecl wrhook_count(void);
;       the writes to fd since wrhook_watch, whatever became of them
;   void __cdecl wrhook_limit(unsigned skip, unsigned max);
;       let skip writes to fd through, then make every one write at most
;       max bytes (0: none, returning AX = 0 without calling DOS)
;   void __cdecl wrhook_fail(unsigned skip, unsigned err);
;       let skip writes to fd through, then fail every one with DOS
;       error err, writing nothing
;
; The state is in the code segment, so the handler needs only CS.

        .8086
        include model.inc

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT

old21   dd      0
watch   dw      0FFFFh          ; the handle
count   dw      0
mode    db      0               ; 0 through, 1 limit, 2 fail
skip    dw      0
arg     dw      0               ; max, or the error

handler:
        cmp     ah, 40h
        jne     chain
        cmp     bx, cs:watch
        jne     chain
        inc     cs:count
        cmp     cs:mode, 0
        je      chain
        cmp     cs:skip, 0
        je      act
        dec     cs:skip
chain:
        jmp     dword ptr cs:old21
act:
        cmp     cs:mode, 2
        je      fail
        cmp     cx, cs:arg
        jbe     chain
        cmp     cs:arg, 0
        je      none
        ; write max bytes, and give the caller its CX back
        push    cx
        mov     cx, cs:arg
        pushf
        call    dword ptr cs:old21
        pop     cx
        retf    2                       ; with the flags DOS returned
none:
        xor     ax, ax
        push    bp
        mov     bp, sp
        and     word ptr [bp + 6], 0FFFEh   ; CF clear in the flags IRET restores
        pop     bp
        iret
fail:
        mov     ax, cs:arg
        push    bp
        mov     bp, sp
        or      word ptr [bp + 6], 1    ; CF set
        pop     bp
        iret

        public  _wrhook_install
_wrhook_install proc CDIST
        push    es
        push    bx
        mov     ax, 3521h
        int     21h
        mov     word ptr cs:old21, bx
        mov     word ptr cs:old21 + 2, es
        push    ds
        push    cs
        pop     ds
        mov     dx, offset handler
        mov     ax, 2521h
        int     21h
        pop     ds
        pop     bx
        pop     es
        ret
_wrhook_install endp

        public  _wrhook_remove
_wrhook_remove proc CDIST
        push    ds
        lds     dx, cs:old21
        mov     ax, 2521h
        int     21h
        pop     ds
        ret
_wrhook_remove endp

        public  _wrhook_watch
_wrhook_watch proc CDIST
        push    bp
        mov     bp, sp
        mov     ax, [bp + ARG0]
        mov     cs:watch, ax
        mov     cs:count, 0
        mov     cs:mode, 0
        pop     bp
        ret
_wrhook_watch endp

        public  _wrhook_count
_wrhook_count proc CDIST
        mov     ax, cs:count
        ret
_wrhook_count endp

; mode in AL, then skip and arg from the arguments
setmode proc    near
        push    bp
        mov     bp, sp
        mov     cs:mode, al
        mov     ax, [bp + ARG0 + 2]
        mov     cs:skip, ax
        mov     ax, [bp + ARG0 + 4]
        mov     cs:arg, ax
        pop     bp
        ret
setmode endp

        public  _wrhook_limit
_wrhook_limit proc CDIST
        mov     al, 1
        call    setmode
        ret
_wrhook_limit endp

        public  _wrhook_fail
_wrhook_fail proc CDIST
        mov     al, 2
        call    setmode
        ret
_wrhook_fail endp

_TEXT   ends

        end
