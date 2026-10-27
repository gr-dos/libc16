/* sys/types.h - libc16 (Microsoft C's types for sys/stat.h) */

#ifndef _LIBC16_SYS_TYPES_H
#define _LIBC16_SYS_TYPES_H

#include <_libc16.h>

typedef short dev_t;            /* drive number, A: is 0 */
typedef unsigned short ino_t;
typedef long off_t;

/* Seconds since 1970-01-01 00:00:00 UTC */
#ifndef _LIBC16_TIME_T
#define _LIBC16_TIME_T
typedef long time_t;
#endif

#endif
