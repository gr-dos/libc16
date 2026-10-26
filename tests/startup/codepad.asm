; Linked after nullstr.obj: a paragraph-aligned code segment of 16 bytes,
; last of class CODE, so that the code ends at a paragraph and the data
; that follows starts at offset 0 of DGROUP's frame, whatever the size of
; the code before it.

        .8086

CODEPAD_TEXT segment para public 'CODE'
        db      16 dup (90h)
CODEPAD_TEXT ends

        end
