; int86 - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md. The interrupt number is
; only known at run time, so it is stored into the INT instruction below.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _int86

; int __cdecl int86(int intno, union REGS *in, union REGS *out)
; union REGS: ax, bx, cx, dx, si, di, cflag at offsets 0 to 12.
_int86  proc    near
        push    bp
        mov     bp, sp
        push    si
        push    di
        mov     al, [bp+4]
        mov     cs:intnum, al
        jmp     short load              ; the store above may be prefetched
load:   mov     bx, [bp+6]
        mov     ax, [bx]
        mov     cx, [bx+4]
        mov     dx, [bx+6]
        mov     si, [bx+8]
        mov     di, [bx+10]
        mov     bx, [bx+2]
        push    ds
        push    bp
        db      0cdh                    ; INT intnum
intnum  db      0
        pop     bp                      ; neither POP nor MOV changes flags
        pop     ds
        push    bx
        mov     bx, [bp+8]
        mov     [bx], ax
        pop     word ptr [bx+2]
        mov     [bx+4], cx
        mov     [bx+6], dx
        mov     [bx+8], si
        mov     [bx+10], di
        sbb     cx, cx
        and     cx, 1
        mov     [bx+12], cx
        cld
        pop     di
        pop     si
        pop     bp
        ret
_int86  endp

_TEXT   ends

        end
