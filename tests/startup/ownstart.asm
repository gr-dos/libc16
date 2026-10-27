; The entry point of tests/startup/ownent, in place of the core startup:
; DS and SS to DGROUP, BSS zeroed, the direction flag clear, then
; ownmain_ (default convention), which ends the program through exit.

        .8086

_TEXT   segment byte public 'CODE'
_TEXT   ends
NULLSEG segment word public 'BEGDATA'
        dw      0                       ; keeps C objects off DGROUP:0
NULLSEG ends
_DATA   segment word public 'DATA'
_DATA   ends
_BSS    segment word public 'BSS'
_BSS    ends
STACK   segment para stack 'STACK'
        db      1024 dup (?)
stktop  label   byte
STACK   ends

DGROUP  group   NULLSEG, _DATA, _BSS, STACK

        extrn   ownmain_:near
        extrn   _edata:byte
        extrn   _end:byte

_TEXT   segment
        assume  cs:_TEXT, ds:DGROUP
start:  mov     ax, DGROUP
        mov     ds, ax
        cli
        mov     ss, ax
        mov     sp, offset DGROUP:stktop
        sti
        mov     es, ax
        mov     di, offset DGROUP:_edata
        mov     cx, offset DGROUP:_end
        sub     cx, di
        xor     al, al
        cld
        rep     stosb
        call    ownmain_
        mov     ax, 4c01h               ; not reached: ownmain calls exit
        int     21h
_TEXT   ends

        end     start
