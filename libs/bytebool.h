#ifndef __BYTEBOOL__
#define __BYTEBOOL__

// bool is a C++ type
// if we are compiling for C, use an enum

// this header is not really meant for direct inclusion,
// it is used by mathlib and cmdlib

#ifdef __cplusplus
typedef bool qboolean;
#define qtrue true
#define qfalse false
#else
typedef enum { qfalse, qtrue } qboolean;
#endif

typedef unsigned char byte;

#endif
