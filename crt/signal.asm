; signal - libc16
;
; Written from docs/spec/signal.md and docs/ABI.md.
;
; void (__cdecl *signal(int sig, void (__cdecl *func)(int)))(int),
; __cdecl: sets the disposition of sig (crt/sigtab.asm) to func and
; returns the previous one; for SIGINT and a func other than SIG_DFL,
; first installs the INT 23h handler (crt/sigint.asm), which does nothing
; once it is installed. A sig outside 1 to 31, or func SIG_ERR, returns
; SIG_ERR with errno EINVAL. Also sets abort's hook, ___libc16_sigabrt, to
; sigabrt. func is a code pointer: near, or far in the large model, at
; ARG0+2, and so is the result (AX, or DX:AX).
;
; sigabrt, called by abort with no arguments: if SIGABRT has a handler,
; sets its disposition to SIG_DFL and calls it with SIGABRT.

        .8086
        include model.inc

SIGINT_  equ    2
SIGABRT_ equ    22
NSIG_    equ    32
EINVAL_  equ    22

SIG_    equ     ARG0
FUNC_   equ     ARG0+2

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

_BSS    segment
        extrn   ___libc16_sigtab:word, _errno:word
_BSS    ends

ifdef FARCODE
        comm    near ___libc16_sigabrt:dword
else
        comm    near ___libc16_sigabrt:word
endif

ABRT    equ     ___libc16_sigtab + (SIGABRT_ - 1) * CPTR

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP
        extrn   ___libc16_sigint_on:CDIST

; DS is DGROUP on entry to a __cdecl function in every model.
        public  _signal
_signal proc    CDIST
        push    bp
        mov     bp, sp
        mov     bx, [bp+SIG_]
        dec     bx
        cmp     bx, NSIG_ - 1
        jae     bad
        mov     ax, [bp+FUNC_]
        mov     cx, ax
ifdef FARCODE
        mov     dx, [bp+FUNC_+2]
        and     cx, dx
endif
        inc     cx
        jz      bad                     ; SIG_ERR, all ones
        mov     word ptr ___libc16_sigabrt, offset sigabrt
ifdef FARCODE
        mov     word ptr ___libc16_sigabrt+2, cs
        shl     bx, 1
endif
        shl     bx, 1
        cmp     bx, (SIGINT_ - 1) * CPTR
        jne     set
        mov     cx, ax
ifdef FARCODE
        or      cx, dx
        push    dx
endif
        jcxz    set1                    ; SIG_DFL
        push    ax
        push    bx
        call    ___libc16_sigint_on
        pop     bx
        pop     ax
set1:
ifdef FARCODE
        pop     dx
endif
set:    xchg    ax, ___libc16_sigtab[bx]
ifdef FARCODE
        xchg    dx, ___libc16_sigtab[bx+2]
endif
        pop     bp
        ret
bad:    mov     _errno, EINVAL_
        mov     ax, -1
        cwd
        pop     bp
        ret
_signal endp

sigabrt proc    CDIST private
        mov     dx, ABRT
ifdef FARCODE
        mov     cx, ABRT+2
        test    cx, cx
        jnz     go                      ; a segment: a handler
endif
        cmp     dx, 1
        jbe     done                    ; SIG_DFL or SIG_IGN
go:
        xor     ax, ax
        mov     ABRT, ax
ifdef FARCODE
        mov     ABRT+2, ax
endif
        mov     al, SIGABRT_
ifdef FARCODE
        push    cx                      ; the handler, under its argument
        push    dx
        push    ax
        mov     bx, sp
        call    dword ptr ss:[bx+2]
        add     sp, 6
else
        push    ax
        call    dx
        pop     ax
endif
done:   ret
sigabrt endp

_TEXT   ends

        end
