#ifndef __BIT_CONTROL__
#define __BIT_CONTROL__


#define BIT_SET(addr, shift) ( *(addr) |= (1 << shift) )
#define BIT_CLEAR(addr, shift) ( *addr &= ~(1 << shift) )
#define BIT_CHECK(addr, mask, shift) ( (*(addr) &= (mask)) >> (shift) )

// TODO: simplify the call for BF_SET, ideally the caller shouldn't have to
// know how many bits are in the field and how much to shift them by
#define BF_SET(addr, val, len, shift) (*(addr) = (*(addr)&~(((1 << len)-1) << (shift))) | ((val) << (shift)))


#endif
