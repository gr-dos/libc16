; int86 - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md. The interrupt number is
; only known at run time, so it is stored into the INT instruction below.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _int86

; int __cdecl int86(int intno, union REGS *in, union REGS *out)
; union REGS: ax, bx, cx, dx, si, di, cflag at offsets 0 to 12.
; With far data, the pointers are read with LDS and LES; DS is restored
; for the interrupt and ES is kept as the caller had it.
IN_     equ     ARG0+2
OUT_    equ     ARG0+2+DPTR

_int86  proc    CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
ifdef FARDATA
        push    es
endif
        mov     al, [bp+ARG0]
        mov     cs:intnum, al
        jmp     short load              ; the store above may be prefetched
ifdef FARDATA
load:   push    ds
        lds     bx, [bp+IN_]
else
load:   mov     bx, [bp+IN_]
endif
        mov     ax, [bx]
        mov     cx, [bx+4]
        mov     dx, [bx+6]
        mov     si, [bx+8]
        mov     di, [bx+10]
        mov     bx, [bx+2]
ifdef FARDATA
        pop     ds
endif
        push    ds
        push    bp
        db      0cdh                    ; INT intnum
intnum  db      0
        pop     bp                      ; neither POP, MOV nor LES changes flags
        pop     ds
        push    bx
ifdef FARDATA
        les     bx, [bp+OUT_]
        mov     es:[bx], ax
        pop     word ptr es:[bx+2]
        mov     es:[bx+4], cx
        mov     es:[bx+6], dx
        mov     es:[bx+8], si
        mov     es:[bx+10], di
        sbb     cx, cx
        and     cx, 1
        mov     es:[bx+12], cx
        cld
        pop     es
else
        mov     bx, [bp+OUT_]
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
endif
        pop     di
        pop     si
        pop     bp
        ret
_int86  endp

_TEXT   ends

        end
