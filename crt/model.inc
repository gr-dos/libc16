; model.inc - the memory model of an assembly member (docs/ABI.md, "Memory
; models"). The Makefile assembles every member three times, with
; -D__SMALL__, -D__COMPACT__ or -D__LARGE__, the macros wcc predefines for
; -ms, -mc and -ml.
;
;   FARCODE   defined in the large model: functions are far, called with a
;             far call and returning with RETF
;   FARDATA   defined in the compact and large models: a data pointer is a
;             segment and an offset, 4 bytes, offset in the low word
;   CDIST     the distance of a function, for PROC and EXTRN: near or far
;   ARG0      the offset from BP of a function's first stack argument,
;             after PUSH BP / MOV BP,SP
;   DPTR      the size of a data pointer, 2 or 4 bytes

ifdef __LARGE__
FARCODE equ     1
FARDATA equ     1
elseifdef __COMPACT__
FARDATA equ     1
elseifndef __SMALL__
        .err    <assemble with -D__SMALL__, -D__COMPACT__ or -D__LARGE__>
endif

ifdef FARCODE
CDIST   textequ <far>
ARG0    equ     6
else
CDIST   textequ <near>
ARG0    equ     4
endif

ifdef FARDATA
DPTR    equ     4
else
DPTR    equ     2
endif
