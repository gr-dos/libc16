; dosexec - libc16
;
; Written from docs/spec/process.md and docs/ABI.md.
;
; int __cdecl __libc16_dosexec(const char *name, unsigned env,
;                              const char *tail)
; Runs name with INT 21h 4B00h. The parameter block, built on the stack,
; has the environment segment env, the command tail tail and a blank FCB
; for both FCBs. Returns the child's exit code (AL of INT 21h 4Dh, 0 to
; 255), or minus the DOS error code if 4B00h fails, with 3 (path not
; found) and 15 (invalid drive) as 2 (file not found): the errors on which
; the spawn functions try the next name, all ENOENT.
;
; Under DOS 2.x 4B00h returns with every register but CS and IP changed,
; SS and SP included. SS is DGROUP (docs/ABI.md), so SP is saved in
; DGROUP and both are found again through the DGROUP fixup; BP, SI, DI
; and DS are restored from the stack.

        .8086
        include model.inc

NAME_   equ     ARG0
ENV_    equ     ARG0+DPTR
TAIL_   equ     ARG0+DPTR+2

_TEXT   segment byte public 'CODE'
_TEXT   ends
_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

_BSS    segment
savesp  dw      ?
_BSS    ends

_TEXT   segment
        assume  cs:_TEXT
        public  ___libc16_dosexec

; drive 0 and a blank name
blank   db      0, 11 dup (' '), 4 dup (0)

___libc16_dosexec proc CDIST
        push    bp
        mov     bp, sp
        push    si
        push    di
        push    ds
        ; the parameter block: env, tail, FCB 1, FCB 2
        mov     ax, offset blank
        push    cs
        push    ax
        push    cs
        push    ax
ifdef FARDATA
        push    word ptr [bp+TAIL_+2]
else
        push    ds
endif
        push    word ptr [bp+TAIL_]
        push    word ptr [bp+ENV_]
        mov     bx, sp
        push    ss
        pop     es
ifdef FARDATA
        lds     dx, [bp+NAME_]
else
        mov     dx, [bp+NAME_]
endif
        mov     ss:savesp, sp
        mov     ax, 4b00h
        int     21h
        mov     bx, DGROUP
        cli
        mov     ss, bx
        mov     sp, ss:savesp
        sti
        cld
        jc      fail
        mov     ah, 4dh
        int     21h
        xor     ah, ah
        jmp     short done
fail:   cmp     ax, 3
        je      nofile
        cmp     ax, 15
        jne     error
nofile: mov     ax, 2
error:  neg     ax
done:   add     sp, 14
        pop     ds
        pop     di
        pop     si
        pop     bp
        ret
___libc16_dosexec endp

_TEXT   ends

        end
