#include "random.h"

// The state array must be initialized to not be all zero
// in the first four words
// period of 2^192 - 2^32
u32
xorwow(xorwow_state *state)
{
	// Algorithm "xorwow" from p.5 of Marsaglia, "Xorshifts RNGs"
	u32 t = state->x[4];

	u32 s = state->x[0]; // Perform a contrived 32-bit shift
	state->x[4] = state->x[3];
	state->x[3] = state->x[2];
	state->x[2] = state->x[1];
	state->x[1] = s;

	t ^= t >> 2;
	t ^= t << 1;
	t ^= s ^ (s << 4);
	state->x[0] = t;
	state->counter += 362437;

	return t + state->counter;
}

// The state must be initialized to non-zero
// period of 2^32 - 1
u32
xorshift32(xorshift32_state *state)
{
	// Algorithm "xor" from p.4 of Marsaglia, "Xorshift RNGs"
	u32 x = state->a;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->a = x;
}

u32
xorshift32_range(xorshift32_state* state, u32 min, u32 max)
{
    u32 Result = min + (xorshift32(state) % (max - min));
    return Result;
}
