/* The checks of heap.c in a program linked with option stack=8192
 * (Makefile): the heap starts above the larger stack, and survives 6 KiB
 * of it being used. */

#define STACK 8192
#define DEPTH 6144
#include "heap.c"
