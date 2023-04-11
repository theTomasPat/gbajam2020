#ifndef __FIXEDPOINT__
#define __FIXEDPOINT__

#include <stdint.h>
#include "gba.h"

// Fixed point numbers are 16.16
// largest uint value is 2^16 = 65535
// smallest possible value is 2^-16 = 0.00001526...
typedef i32 fp_t;

#define FP_FRACBITS 16

fp_t Int2FP(i32 i);
i32  FP2Int(fp_t i);
fp_t FP(i32 whole, i32 frac);

// TODO: finish implementing FP arithmetic.
// add and subtract work the same as regular
// integers


#endif
