/* stdarg.h - libc16 (C89 7.8)
 *
 * Arguments of a __cdecl function are on the stack in 2-byte slots, the
 * first one lowest. */

#ifndef _LIBC16_STDARG_H
#define _LIBC16_STDARG_H

typedef char *va_list;

#define _LIBC16_VA_SIZE(t) ((sizeof(t) + 1) & ~1)
#define va_start(ap, last) ((ap) = (char *)&(last) + _LIBC16_VA_SIZE(last))
#define va_arg(ap, t) (*(t *)(((ap) += _LIBC16_VA_SIZE(t)) - _LIBC16_VA_SIZE(t)))
#define va_end(ap) ((void)0)

#endif
