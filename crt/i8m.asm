; libc16 64-bit multiply: __I8M (long long) and __U8M (unsigned long long).
;
; Written from docs/spec/arith.md and the call-site experiments recorded
; in docs/ABI.md ("Compiler support routines"). The low 64 bits of a
; product are the same for signed and unsigned operands, so both symbols
; name one routine.
;
; In:  AX:BX:CX:DX = a (AX the high word), SI -> b (8 bytes, low word
;      first) in DGROUP, at DS:SI in the small model and SS:SI in the
;      compact and large models; b is left unchanged
; Out: AX:BX:CX:DX = a * b mod 2^64; all else preserved
; Called near, or far in the large model.

        .8086
        include model.inc

_TEXT   segment byte public 'CODE'
        assume  cs:_TEXT
        public  __I8M, __U8M

; Schoolbook multiplication in 16-bit words, keeping only the words of the
; product below 2^64: for each word a[i], r[i+j] += a[i] * b[j] for
; i + j < 4, carrying into the next word. a and r are on the stack.
__I8M   proc    CDIST
__U8M   label   CDIST
        push    bp
        push    di
        push    si
        push    ax                      ; a[3]
        push    bx
        push    cx
        push    dx                      ; a[0]; a is at SP
        xor     ax, ax
        push    ax                      ; r[3]
        push    ax
        push    ax
        push    ax                      ; r[0]; r is at SP
        mov     bp, sp                  ; [bp] = r, [bp+8] = a
        mov     bx, si                  ; [bx+di] = b[j] while DI = 2(i+j)
        xor     di, di                  ; DI = 2i
next_row:
        push    di
        mov     si, [bp+di+8]           ; SI = a[i]
        xor     cx, cx                  ; CX = carry
next_column:                            ; DI = 2(i+j)
ifdef FARDATA
        mov     ax, ss:[bx+di]          ; b[j]
else
        mov     ax, [bx+di]             ; b[j]
endif
        mul     si
        add     ax, cx
        adc     dx, 0
        add     [bp+di], ax             ; cannot carry out of DX: the sum
        adc     dx, 0                   ; is at most 2^32 - 1
        mov     cx, dx
        inc     di
        inc     di
        cmp     di, 8
        jb      next_column
        pop     di
        dec     bx                      ; so that [bx+di] is b[j] for
        dec     bx                      ; the next row, i + 1
        inc     di
        inc     di
        cmp     di, 8
        jb      next_row
        pop     dx                      ; r[0]
        pop     cx
        pop     bx
        pop     ax                      ; r[3]
        add     sp, 8                   ; a
        pop     si
        pop     di
        pop     bp
        ret
__I8M   endp
_TEXT   ends

        end
