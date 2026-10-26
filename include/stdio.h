/* stdio.h - libc16 (C89 7.9, the parts docs/spec/stdio.md implements) */

#ifndef _LIBC16_STDIO_H
#define _LIBC16_STDIO_H

#ifndef _LIBC16_SIZE_T
#define _LIBC16_SIZE_T
typedef unsigned size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define EOF          (-1)
#define BUFSIZ       512
#define FOPEN_MAX    8
#define FILENAME_MAX 80

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* The members are private to libc16. */
typedef struct __libc16_file {
    unsigned char *_ptr;    /* next byte to read from the buffer */
    int _cnt;               /* bytes left in the buffer */
    unsigned char *_base;   /* read buffer */
    unsigned _bufsiz;
    int _fd;
    unsigned char _flag;
    unsigned char _hold;    /* character pushed back by ungetc */
} FILE;

#define _IOREAD  0x01
#define _IOWRT   0x02
#define _IOUNGET 0x04
#define _IOEOF   0x10
#define _IOERR   0x20

extern FILE __cdecl __libc16_stdin;
extern FILE __cdecl __libc16_stdout;
extern FILE __cdecl __libc16_stderr;

#define stdin  (&__libc16_stdin)
#define stdout (&__libc16_stdout)
#define stderr (&__libc16_stderr)

int    __cdecl fclose(FILE *fp);
int    __cdecl feof(FILE *fp);
int    __cdecl ferror(FILE *fp);
int    __cdecl fflush(FILE *fp);
int    __cdecl fgetc(FILE *fp);
char * __cdecl fgets(char *s, int n, FILE *fp);
FILE * __cdecl fopen(const char *path, const char *mode);
int    __cdecl fputc(int c, FILE *fp);
int    __cdecl fputs(const char *s, FILE *fp);
int    __cdecl ungetc(int c, FILE *fp);

#define feof(fp)   (((fp)->_flag & _IOEOF) != 0)
#define ferror(fp) (((fp)->_flag & _IOERR) != 0)

#endif
