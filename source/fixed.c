#include "fixed.h"


fp_t Int2FP(i32 i)
{
    return (i<<FP_FRACBITS);
}

i32  FP2Int(fp_t i)
{
    return (i>>FP_FRACBITS);
}

fp_t FP(i32 whole, i32 frac)
{
    return Int2FP(whole) | frac;
}
