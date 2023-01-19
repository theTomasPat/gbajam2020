#ifndef __RANDOM_H__
#define __RANDOM_H__

// algorithms taken from:
// https://en.wikipedia.org/wiki/Xorshift

#include "gba.h"

typedef struct
{
	u32 x[5];
	u32 counter;
} xorwow_state;

u32 xorwow(xorwow_state *state);

typedef struct
{
	u32 a;
} xorshift32_state;

u32 xorshift32(xorshift32_state* state);
u32 xorshift32_range(xorshift32_state* state, u32 min, u32 max);


#endif
