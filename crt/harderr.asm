; _harderr - libc16
;
; Written from docs/spec/dos.md and docs/ABI.md, and the INT 24h entry and
; exit conditions in Ralf Brown's Interrupt List. DOS calls INT 24h on its
; own stack, but C code needs SS = DGROUP (docs/ABI.md, "Memory models"),
; so the handler runs on a stack of this member's own in DGROUP.

        .8086
        include model.inc

HSIZE   equ     512                     ; bytes of stack for the handler

_BSS    segment word public 'BSS'
hstack  db      HSIZE dup (?)
htop    label   byte
_BSS    ends

DGROUP  group   _BSS

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT, ds:nothing
        public  __harderr

handler dd      0                       ; the application's handler
dos_sp  dw      0                       ; DOS's stack while it runs
dos_ss  dw      0

; INT 24h: AX = deverror, DI = errcode, BP:SI = device header. Calls
; handler(deverror, errcode, devhdr) and returns its result to DOS in AL.
crit:   push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    bp
        push    ds
        push    es
        mov     cs:dos_sp, sp
        mov     cs:dos_ss, ss
        mov     bx, DGROUP
        cli
        mov     ss, bx
        mov     sp, offset DGROUP:htop
        sti
        mov     ds, bx
        cld
        push    bp                      ; devhdr, segment
        push    si                      ; and offset
        push    di                      ; errcode
        push    ax                      ; deverror
        call    dword ptr cs:handler
        cli
        mov     ss, cs:dos_ss
        mov     sp, cs:dos_sp
        sti
        pop     es
        pop     ds
        pop     bp
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        iret

; void __cdecl _harderr(int (__cdecl __far *handler)(unsigned deverror,
;                       unsigned errcode, unsigned __far *devhdr))
__harderr proc  CDIST
        push    bp
        mov     bp, sp
        mov     ax, [bp+ARG0]
        mov     word ptr cs:handler, ax
        mov     ax, [bp+ARG0+2]
        mov     word ptr cs:handler+2, ax
        push    ds
        push    cs
        pop     ds
        mov     dx, offset crit
        mov     ax, 2524h
        int     21h
        pop     ds
        pop     bp
        ret
__harderr endp

_TEXT   ends

        end
