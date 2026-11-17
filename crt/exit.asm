; exit - libc16
;
; Written from docs/spec/startup.md ("Termination", "Termination table")
; and docs/ABI.md.
;
; void exit(int status), __cdecl (symbol _exit): calls each entry of the
; termination table in table order, then terminates the program with
; INT 21h function 4Ch, AL = the low byte of status. It does not return.
; Called near, or far in the large model (crt/model.inc), so status is at
; [SP+2] or [SP+4].
;
; It references nothing but the table's bounds, which it declares itself,
; so it works with or without the core startup. A routine that calls exit
; starts a new walk from the table's start with the new status.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
_TEXT   ends

; The three termination segments, in order, so that the bounds hold
; whichever module the linker sees first; after the (empty) init segments,
; so that the termination table follows the init table (spec, "Segments")
; even when the linker sees this module before the core startup.
LIBC16_INIT_BEGIN segment word public 'DATA'
LIBC16_INIT_BEGIN ends
LIBC16_INIT segment word public 'DATA'
LIBC16_INIT ends
LIBC16_INIT_END segment word public 'DATA'
LIBC16_INIT_END ends
LIBC16_EXIT_BEGIN segment word public 'DATA'
exit_first label word
LIBC16_EXIT_BEGIN ends
LIBC16_EXIT segment word public 'DATA'
LIBC16_EXIT ends
LIBC16_EXIT_END segment word public 'DATA'
exit_limit label word
LIBC16_EXIT_END ends

DGROUP  group   LIBC16_INIT_BEGIN, LIBC16_INIT, LIBC16_INIT_END, \
                LIBC16_EXIT_BEGIN, LIBC16_EXIT, LIBC16_EXIT_END

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP

        public  _exit
_exit   proc    CDIST
        mov     bx, sp
        mov     di, ss:[bx+ARG0-2]      ; status (no BP pushed), kept in DI
        mov     si, offset DGROUP:exit_first

; The routines preserve SI, DI, BP and SS. In the small model DS is
; DGROUP on entry and they preserve it; with far data they may change it,
; so the table is read through SS.
next:   cmp     si, offset DGROUP:exit_limit
        jae     done
ifdef FARCODE
        call    dword ptr ss:[si]
        add     si, 4
elseifdef FARDATA
        call    word ptr ss:[si]
        inc     si
        inc     si
else
        call    word ptr [si]
        inc     si
        inc     si
endif
        jmp     next

done:   xchg    ax, di                  ; AL = low byte of status
        mov     ah, 4ch                 ; terminate with return code
        int     21h
_exit   endp

_TEXT   ends

        end
