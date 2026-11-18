; sigtab - libc16, private
;
; Written from docs/spec/signal.md.
;
; ___libc16_sigtab: the dispositions of signals 1 to 31, in that order,
; each a code pointer (CPTR bytes, crt/model.inc): 0 for SIG_DFL, 1 for
; SIG_IGN, else a __cdecl handler. In BSS, so every signal starts at
; SIG_DFL. Linked by signal, raise and sigint.
;
; ___libc16_sigabrt: the routine abort (crt/abort.asm) calls first if it
; is not 0, a code pointer that signal sets. A communal variable, which
; abort declares too, so that abort links nothing.
;
; In the small and compact models a handler is a near pointer, the offset
; of a function in _TEXT, and the first function of the program's first
; module is at offset 0, which is SIG_DFL (C89 7.7 requires SIG_DFL and
; SIG_IGN to compare unequal to the address of any function).
; LIBC16_CODE0, class BEGCODE, holds 2 bytes that no function can be at:
; under the startup's dosseg ordering wlink puts that class before every
; CODE segment, so _TEXT starts at offset 2 (our experiment: a BEGCODE
; segment in an object linked after the one with main put main at 0002;
; a CODE segment, of any name but BEGTEXT, did not). No program can set a
; disposition without linking this member.

        .8086
        include model.inc

_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

ifndef FARCODE
LIBC16_CODE0 segment para public 'BEGCODE'
        db      2 dup (0)
LIBC16_CODE0 ends
endif

ifdef FARCODE
        comm    near ___libc16_sigabrt:dword
else
        comm    near ___libc16_sigabrt:word
endif

_BSS    segment
        public  ___libc16_sigtab
___libc16_sigtab db 31 * CPTR dup (?)
_BSS    ends

        end
