; A hook on INT 21h for tests/io: makes one chosen "move file pointer"
; call (function 42h) fail with DOS error 6, invalid handle, so that the
; tests can reach the error paths of functions that seek.
;
;   void __cdecl seekfail_install(void);    hook INT 21h
;   void __cdecl seekfail_remove(void);     unhook it; call before exiting
;   void __cdecl seekfail(int origin, unsigned skip);
;       let skip calls with AL = origin through, then fail the next one,
;       once; origin -1 fails none
;
; The state is in the code segment, so the handler needs only CS.

        .8086

_TEXT   segment word public 'CODE'
        assume  cs:_TEXT

old21   dd      0
origin  db      0FFh
skip    dw      0

handler:
        cmp     ah, 42h
        jne     chain
        cmp     al, cs:origin
        jne     chain
        cmp     cs:skip, 0
        je      fail
        dec     cs:skip
chain:
        jmp     dword ptr cs:old21
fail:
        mov     cs:origin, 0FFh
        mov     ax, 6
        push    bp
        mov     bp, sp
        or      word ptr [bp + 6], 1    ; CF in the flags IRET restores
        pop     bp
        iret

        public  _seekfail_install
_seekfail_install proc near
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
_seekfail_remove proc near
        push    ds
        lds     dx, cs:old21
        mov     ax, 2521h
        int     21h
        pop     ds
        ret
_seekfail_remove endp

        public  _seekfail
_seekfail proc near
        push    bp
        mov     bp, sp
        mov     ax, [bp + 6]
        mov     cs:skip, ax
        mov     al, [bp + 4]
        mov     cs:origin, al
        pop     bp
        ret
_seekfail endp

_TEXT   ends

        end
