#include <stdint.h>
#include "gba.h"


#ifndef __FIXEDPOINT__
#define __FIXEDPOINT__


// Fixed point numbers are 16.16
// largest uint value is 2^16 = 65535
// smallest possible value is 2^-16 = 0.00001526...

typedef s32 fp_t;
typedef u32 fpu_t;

#define FP_FRACBITS 16

#define FP2INT(x) ((x) >> (FP_FRACBITS))
#define INT2FP(x) ((x) << (FP_FRACBITS))
#define FP(x, y)  ((INT2FP(x)) | (y))

// TODO: finish implementing FP arithmetic.
// add and subtract work the same as regular
// integers


#endif
