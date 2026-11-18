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

        .8086
        include model.inc

_BSS    segment word public 'BSS'
_BSS    ends

DGROUP  group   _BSS

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
