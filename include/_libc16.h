/* _libc16.h - private to libc16's headers: the memory model (docs/ABI.md,
 * "Memory models"). Every public header includes it. */

#ifndef _LIBC16__LIBC16_H
#define _LIBC16__LIBC16_H

#if !defined(__SMALL__) && !defined(__COMPACT__) && !defined(__LARGE__)
#error libc16 supports only the small, compact and large models (-ms, -mc, -ml)
#endif

#endif
