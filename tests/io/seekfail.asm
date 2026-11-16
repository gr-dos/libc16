; A hook on INT 21h for tests/io and tests/stdio: makes one chosen "move file pointer"
; call (function 42h) fail with DOS error 6, invalid handle, so that the
; tests can reach the error paths of functions that seek.
;
;   void __cdecl seekfail_install(void);    hook INT 21h
;   void __cdecl seekfail_remove(void);     unhook it; call before exiting
;   void __cdecl seekfail(int origin, unsigned skip);
;       let skip calls with AL = origin through, then fail the next one,
;       once; origin -2 counts calls with any origin, -1 fails none
;   void __cdecl seekfail_after(unsigned skip);
;       let skip calls with any origin through, then fail every one until
;       the next seekfail
;   unsigned __cdecl seekfail_count(void);
;       the calls failed since the last seekfail or seekfail_after
;
; The state is in the code segment, so the handler needs only CS.

        .8086
        include model.inc

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT

old21   dd      0
origin  db      0FFh            ; 0FFh none, 0FEh any
sticky  db      0               ; fail every call once skip reaches 0
skip    dw      0
count   dw      0

handler:
        cmp     ah, 42h
        jne     chain
        cmp     cs:origin, 0FFh
        je      chain
        cmp     cs:origin, 0FEh
        je      any
        cmp     al, cs:origin
        jne     chain
any:
        cmp     cs:skip, 0
        je      fail
        dec     cs:skip
chain:
        jmp     dword ptr cs:old21
fail:
        inc     cs:count
        cmp     cs:sticky, 0
        jne     keep
        mov     cs:origin, 0FFh
keep:
        mov     ax, 6
        push    bp
        mov     bp, sp
        or      word ptr [bp + 6], 1    ; CF in the flags IRET restores
        pop     bp
        iret

        public  _seekfail_install
_seekfail_install proc CDIST
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
_seekfail_install endp

        public  _seekfail_remove
_seekfail_remove proc CDIST
        push    ds
        lds     dx, cs:old21
        mov     ax, 2521h
        int     21h
        pop     ds
        ret
_seekfail_remove endp

        public  _seekfail
_seekfail proc CDIST
        push    bp
        mov     bp, sp
        mov     ax, [bp + ARG0 + 2]
        mov     cs:skip, ax
        mov     al, [bp + ARG0]
        mov     cs:origin, al
        mov     cs:sticky, 0
        mov     cs:count, 0
        pop     bp
        ret
_seekfail endp

        public  _seekfail_after
_seekfail_after proc CDIST
        push    bp
        mov     bp, sp
        mov     ax, [bp + ARG0]
        mov     cs:skip, ax
        mov     cs:origin, 0FEh
        mov     cs:sticky, 1
        mov     cs:count, 0
        pop     bp
        ret
_seekfail_after endp

        public  _seekfail_count
_seekfail_count proc CDIST
        mov     ax, cs:count
        ret
_seekfail_count endp

_TEXT   ends

        end
