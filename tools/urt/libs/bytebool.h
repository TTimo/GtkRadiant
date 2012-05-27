#ifndef __BYTEBOOL__
#define __BYTEBOOL__

// defines boolean and byte types usable in both c and c++ code
// this header is not really meant for direct inclusion,
// it is used by mathlib and cmdlib

typedef enum { qfalse, qtrue } qboolean;
typedef unsigned char byte;

#endif
