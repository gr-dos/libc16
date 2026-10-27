; int86x - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md, like int86.asm: the
; interrupt number is stored into the INT instruction below.

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _int86x

; int __cdecl int86x(int intno, union REGS *in, union REGS *out,
;                    struct SREGS *segs)
; union REGS: ax, bx, cx, dx, si, di, cflag at offsets 0 to 12.
; struct SREGS: es, cs, ss, ds at offsets 0 to 6.
_int86x proc    near
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds                      ; [bp-6]: DGROUP
        mov     al, [bp+4]
        mov     cs:intnum, al
        jmp     short load              ; the store above may be prefetched
load:   mov     bx, [bp+10]
        mov     es, [bx]
        push    word ptr [bx+6]         ; DS for the interrupt
        mov     bx, [bp+6]
        mov     ax, [bx]
        mov     cx, [bx+4]
        mov     dx, [bx+6]
        mov     si, [bx+8]
        mov     di, [bx+10]
        mov     bx, [bx+2]
        pop     ds
        push    bp
        db      0cdh                    ; INT intnum
intnum  db      0
        pop     bp                      ; neither POP nor MOV changes flags
        pushf
        push    ds
        push    bx
        mov     ds, [bp-6]
        mov     bx, [bp+8]
        mov     [bx], ax
        pop     word ptr [bx+2]
        mov     [bx+4], cx
        mov     [bx+6], dx
        mov     [bx+8], si
        mov     [bx+10], di
        pop     cx                      ; the interrupt's DS
        pop     dx                      ; its flags
        and     dx, 1                   ; carry
        mov     [bx+12], dx
        mov     bx, [bp+10]
        mov     [bx], es
        mov     [bx+6], cx
        cld
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_int86x endp

_TEXT   ends

        end
