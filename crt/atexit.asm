; atexit - libc16
;
; Written from docs/spec/startup.md ("atexit", "Termination table") and
; docs/ABI.md.
;
; int atexit(void (__cdecl *func)(void)), __cdecl (symbol _atexit):
; registers func and returns 0, or returns nonzero once 32 are
; registered. Linking this member adds one routine, run_atexit, to the
; termination table; it calls the registered functions, most recent
; first. Each is removed from the list before it is called, so one that
; calls exit (which walks the table again, calling run_atexit again) or
; registers another leaves a list that the next call carries on with:
; every function is called at most once, and one registered during
; termination is called next.
;
; Function pointers are near, or far in the large model (crt/model.inc).
; The list is in BSS, so it starts empty with or without the core startup.

        .8086
        include model.inc

ATEXIT_MAX equ  32

_TEXT   segment byte public 'CODE'
_TEXT   ends
; The init segments first, as in crt/exit.asm, so that the termination
; table follows the init table whichever module the linker sees first.
LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends
LIBC16_EXIT_BEGIN segment word public 'DATA'
LIBC16_EXIT_BEGIN ends
LIBC16_EXIT segment word public 'DATA'
LIBC16_EXIT ends
LIBC16_EXIT_END segment word public 'DATA'
LIBC16_EXIT_END ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END, \
                LIBC16_EXIT_BEGIN, LIBC16_EXIT, LIBC16_EXIT_END, _BSS

_BSS    segment
count   dw      ?                       ; functions registered
ifdef FARCODE
funcs   dd      ATEXIT_MAX dup (?)      ; offset, then segment
else
funcs   dw      ATEXIT_MAX dup (?)
endif
_BSS    ends

LIBC16_EXIT segment
ifdef FARCODE
        dd      run_atexit
else
        dw      run_atexit
endif
LIBC16_EXIT ends

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP

; DS is DGROUP on entry to a __cdecl function in every model.
        public  _atexit
_atexit proc    CDIST
        mov     bx, sp
ifdef FARCODE
        les     ax, [bx+ARG0-2]         ; ES:AX = func
else
        mov     ax, [bx+ARG0-2]
endif
        mov     bx, count
        cmp     bx, ATEXIT_MAX
        jae     full
        inc     count
        shl     bx, 1
ifdef FARCODE
        shl     bx, 1
        mov     word ptr funcs[bx+2], es
endif
        mov     word ptr funcs[bx], ax
        xor     ax, ax
        ret
full:   xchg    ax, bx                  ; ATEXIT_MAX: nonzero
        ret
_atexit endp

; The termination routine: called with SS = DGROUP, the direction flag
; clear and, in the small model, DS = DGROUP; preserves SI, DI, BP and
; SS (and DS in the small model, as each function does). With far data
; DS is set to DGROUP before each call, as a __cdecl function expects.
run_atexit proc CDIST
again:
ifdef FARDATA
        push    ss
        pop     ds
endif
        mov     bx, count
        dec     bx
        js      done
        mov     count, bx               ; removed before it is called
        shl     bx, 1
ifdef FARCODE
        shl     bx, 1
        call    dword ptr funcs[bx]
else
        call    word ptr funcs[bx]
endif
        jmp     again
done:   ret
run_atexit endp

_TEXT   ends

        end
