// mGBA debug API info can be found here:
// https://github.com/mgba-emu/mgba/blob/master/opt/libgba/mgba.c


#ifndef __MGBA__
#define __MGBA__

#include "gba.h"

// mGBA Debug Defines

#define REG_DEBUG_ENABLE (volatile u16 *)0x04FFF780
#define REG_DEBUG_FLAGS  (volatile u16 *)0x04FFF700
#define REG_DEBUG_STRING         (char *)0x04FFF600

#define DEBUG_FATAL 0
#define DEBUG_ERROR 1
#define DEBUG_WARN 2
#define DEBUG_INFO 3
#define DEBUG_DEBUG 4

int mgba_open();
void mgba_close();
void mgba_printf(u32 level, char *str, u32 len);


#endif
