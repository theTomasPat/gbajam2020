#include <string.h>
#include "gba.h"
#include "256Palette.h"
#include "Bat.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

// data containers for each type of tile
typedef struct { int data[8];  } TILE4;
typedef struct { int data[16]; } TILE8;

// each "bank" of tiles occupies 16KB, for 32byte tiles, that's 512 per bank
typedef TILE4 CHARBLOCK[512];
typedef TILE8 CHARBLOCK8[256];

#define tile_mem  ( (CHARBLOCK*)0x06000000)
#define tile8_mem ((CHARBLOCK8*)0x06000000)


int main(void)
{
	// copy the palette data to 0x05000200
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

	// copy the sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], BatTiles, BatTilesLen);

	// set OAM OBJ[0] to use the sprite data
	short *OAM_ATTR0 = (short *)0x07000000;
	short *OAM_ATTR1 = (short *)0x07000002;
	short *OAM_ATTR2 = (short *)0x07000004;

	#if 0
	*OAM_ATTR0 = 0;
	*OAM_ATTR1 = 0;
	*OAM_ATTR2 = 0;
	#endif

	*OAM_ATTR0 |= 80; // y coord
	*OAM_ATTR0 |= (1 << 13); // palette config (1=256/1)
	*OAM_ATTR1 |= 120; // x coord
	*OAM_ATTR1 &= ~(1 << 12); // hor flip
	*OAM_ATTR1 &= ~(1 << 13); // vert flip
	*OAM_ATTR1 |= (1 << 14); // OBJ size 0=16x16
	
	// Initialize display control register
	// BG Mode 0
	// OBJ tiling, one-dimensional
	// Screen Display OBJ
    *REG_DISPCNT = (1 << 0) | (1 << 6) | (1 << 12);

	*BG0CNT |= (1 << 7);


    while(1);

    return 0;
}
