; libc16 32-bit unsigned divide: __U4D (unsigned long / and %).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines").
;
; In:  DX:AX = dividend, CX:BX = divisor
; Out: DX:AX = quotient, CX:BX = remainder; all else preserved
; A zero divisor raises the divide-error trap (INT 0).

        .8086

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __U4D

__U4D:
        jcxz    short16

; Divisor of 17 bits or more: the quotient fits in 16 bits. The dividend's
; high word starts as the partial remainder (it is below the divisor), and
; the 16 bits of AX are shifted in one at a time, restoring division. AX
; collects the quotient as its dividend bits leave it.
        push    si
        push    di
        mov     si, dx                  ; DI:SI = partial remainder
        xor     di, di
        mov     dx, 16                  ; bit count
next_bit:
        shl     ax, 1
        rcl     si, 1
        rcl     di, 1
        jc      subtract                ; 33-bit remainder: above the divisor
        cmp     di, cx
        jne     compared
        cmp     si, bx
compared:
        jb      shifted
subtract:
        sub     si, bx
        sbb     di, cx
        inc     ax                      ; quotient bit 1
shifted:
        dec     dx
        jnz     next_bit                ; DX = 0, the quotient's high word
        mov     bx, si
        mov     cx, di
        pop     di
        pop     si
        ret

; 16-bit divisor in BX: two 32-by-16 divides, high word first.
short16:
        mov     cx, ax                  ; CX = dividend low word
        xchg    ax, dx                  ; AX = high word
        xor     dx, dx
        div     bx                      ; AX = quotient high, DX = remainder
        xchg    ax, cx                  ; CX = quotient high, AX = low word
        div     bx                      ; AX = quotient low, DX = remainder
        mov     bx, dx
        mov     dx, cx
        xor     cx, cx
        ret
_TEXT   ends

        end
