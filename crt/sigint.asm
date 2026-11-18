; sigint - libc16, private
;
; Written from docs/spec/signal.md ("INT 23h") and docs/ABI.md, with INT
; 23h and INT 21h 25h and 35h as Ralf Brown's Interrupt List gives them.
;
; void __cdecl __libc16_sigint_on(void): the first time it is called,
; saves the INT 23h vector (INT 21h 35h) and sets it to sigint (25h).
; Later calls do nothing. Changes AX, BX, DX, ES and the flags.
;
; sigint, the INT 23h handler, entered with the registers of the DOS call
; DOS found the Ctrl-C in, acts by the disposition of SIGINT
; (crt/sigtab.asm):
;   SIG_IGN    returns with IRET, which restores the flags
;   SS not DGROUP (a child is running) or SIG_DFL
;              jumps to the saved vector with every register, the flags
;              and the stack as on entry, by an IRET to it
;   a handler  sets the disposition to SIG_DFL, saves the registers a
;              __cdecl function may change, and with DS = DGROUP,
;              interrupts on and the direction flag clear, calls it with
;              SIGINT on the current stack, then restores them and returns
;              with IRET
;
; Below the frame of sigint (BP after mov bp, sp): the entry flags at
; [bp+6], above a 4-byte slot at [bp+2] that holds the disposition, then,
; to jump to the saved vector, that vector.

        .8086
        include model.inc

SIGINT_ equ     2

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

_BSS    segment
        extrn   ___libc16_sigtab:byte
prev    dw      2 dup (?)               ; the vector replaced, offset first
on      db      ?                       ; non-zero once installed
_BSS    ends

DISP    equ     ___libc16_sigtab + (SIGINT_ - 1) * CPTR

_TEXT   segment
        assume  cs:_TEXT, ds:nothing, ss:nothing

sigint  proc    far private
        pushf
        push    ax                      ; [bp+4], slot segment
        push    ax                      ; [bp+2], slot offset
        push    bp
        mov     bp, sp
        push    ds
        push    ax
        mov     ax, DGROUP
        mov     ds, ax
        assume  ds:DGROUP
ifdef FARCODE
        mov     ax, word ptr DISP+2
        mov     [bp+4], ax
        test    ax, ax
        mov     ax, word ptr DISP
        mov     [bp+2], ax
        jnz     notign                  ; a segment: a handler
else
        mov     ax, word ptr DISP
        mov     [bp+2], ax
endif
        cmp     ax, 1
        je      done                    ; SIG_IGN
notign: mov     ax, ss
        cmp     ax, DGROUP
        jne     chain                   ; a child's
ifdef FARCODE
        cmp     word ptr [bp+4], 0
        jne     handler
endif
        cmp     word ptr [bp+2], 0
        je      chain                   ; SIG_DFL
handler:
        xor     ax, ax
        mov     word ptr DISP, ax
ifdef FARCODE
        mov     word ptr DISP+2, ax
endif
        push    bx
        push    cx
        push    dx
        push    es
        sti
        cld
        mov     al, SIGINT_
        push    ax
ifdef FARCODE
        call    dword ptr [bp+2]
else
        call    word ptr [bp+2]
endif
        pop     ax
        pop     es
        pop     dx
        pop     cx
        pop     bx
done:   pop     ax
        pop     ds
        pop     bp
        add     sp, 6
        iret
chain:  mov     ax, prev
        mov     [bp+2], ax
        mov     ax, prev+2
        mov     [bp+4], ax
        pop     ax
        pop     ds
        pop     bp
        iret
sigint  endp

        assume  ds:DGROUP

; DS is DGROUP on entry to a __cdecl function in every model.
        public  ___libc16_sigint_on
___libc16_sigint_on proc CDIST
        cmp     on, 0
        jne     @F
        inc     on
        mov     ax, 3523h
        int     21h
        mov     prev, bx
        mov     prev+2, es
        push    ds
        push    cs
        pop     ds
        mov     dx, offset sigint
        mov     ax, 2523h
        int     21h
        pop     ds
@@:     ret
___libc16_sigint_on endp

_TEXT   ends

        end
