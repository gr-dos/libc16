; int86x - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md, like int86.asm: the
; interrupt number is stored into the INT instruction below.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  _int86x

; int __cdecl int86x(int intno, union REGS *in, union REGS *out,
;                    struct SREGS *segs)
; union REGS: ax, bx, cx, dx, si, di, cflag at offsets 0 to 12.
; struct SREGS: es, cs, ss, ds at offsets 0 to 6.
IN_     equ     ARG0+2
OUT_    equ     ARG0+2+DPTR
SEGS_   equ     ARG0+2+2*DPTR

_int86x proc    CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds                      ; [bp-6]: the caller's DS
        push    es                      ; the caller's, restored at the end
        mov     al, [bp+ARG0]
        mov     cs:intnum, al
        jmp     short load              ; the store above may be prefetched
ifdef FARDATA
load:   lds     bx, [bp+SEGS_]
else
load:   mov     bx, [bp+SEGS_]
endif
        mov     es, [bx]
        push    word ptr [bx+6]         ; DS for the interrupt
ifdef FARDATA
        lds     bx, [bp+IN_]
else
        mov     bx, [bp+IN_]
endif
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
        pop     bp                      ; neither POP, MOV nor LDS changes flags
        pushf
        push    ds
        push    bx
ifdef FARDATA
        lds     bx, [bp+OUT_]
else
        mov     ds, [bp-6]
        mov     bx, [bp+OUT_]
endif
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
ifdef FARDATA
        lds     bx, [bp+SEGS_]
else
        mov     bx, [bp+SEGS_]
endif
        mov     [bx], es
        mov     [bx+6], cx
        cld
        pop     es
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
_int86x endp

_TEXT   ends

        end
