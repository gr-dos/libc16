; Linked after nullodd.obj: a paragraph-aligned code segment of 17 bytes,
; last of class CODE, so that the code ends one byte past a paragraph,
; whatever the size of the code before it: the most padding a
; paragraph-aligned DGROUP needs after it. Its bytes are 90h, so that a
; DGROUP whose frame started inside it would show them at DS:0000.

        .8086

CODEODD_TEXT segment para public 'CODE'
        db      17 dup (90h)
CODEODD_TEXT ends

        end
