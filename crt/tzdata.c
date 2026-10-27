/* timezone, daylight and tzname - libc16 (docs/spec/time.md) */

#include <time.h>
#include "time16.h"

char __libc16_tzstd[16] = "UTC";
char __libc16_tzdst[16];
char __libc16_tzdone;

long __cdecl timezone;
int __cdecl daylight;
char * __cdecl tzname[2] = { __libc16_tzstd, __libc16_tzdst };
