#include <string.h>
#include "gba.h"
#include "256Palette.h"
#include "Bat.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct
{
	short attr0;
	short attr1;
	short attr2;
	short fill;
} OBJ_ATTR;

// data containers for each type of tile
typedef struct { int data[8];  } TILE4;
typedef struct { int data[16]; } TILE8;

// each "bank" of tiles occupies 16KB, for 32byte tiles, that's 512 per bank
typedef TILE4 CHARBLOCK[512];
typedef TILE8 CHARBLOCK8[256];

#define tile_mem  ( (CHARBLOCK*)0x06000000)
#define tile8_mem ((CHARBLOCK8*)0x06000000)

#define BitSet(addr,val,shift) ( *addr |= (val << shift) )
#define BitClear(addr, shift) ( *addr &= ~(1 << shift) )

void OAM_Init()
{
	OBJ_ATTR *obj = (OBJ_ATTR *)OAM_MEM;

	for(int i = 0; i < 128; i++)
	{
		*obj = (OBJ_ATTR){0};
		BitSet(&obj->attr0, 1, ATTR0_DISABLE);

		obj++;
	}
}

int main(void)
{
	OBJ_ATTR *OAM_chars[128];
	OAM_chars[0] = (OBJ_ATTR *)OAM_MEM;

	int batIndex = 0;

	OAM_Init();

	// copy the palette data to 0x05000200
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

	// copy the sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], BatTiles, BatTilesLen);

	// setup the bat's sprite
	BitSet(&OAM_chars[batIndex]->attr0, 80, ATTR0_YCOORD);
	BitSet(&OAM_chars[batIndex]->attr0, 1, ATTR0_COLORMODE);
	BitClear(&OAM_chars[batIndex]->attr0, ATTR0_DISABLE);
	BitSet(&OAM_chars[batIndex]->attr1, 120, ATTR1_XCOORD);
	BitClear(&OAM_chars[batIndex]->attr1, ATTR1_FLIPHOR);
	BitClear(&OAM_chars[batIndex]->attr1, ATTR1_FLIPVERT);
	BitSet(&OAM_chars[batIndex]->attr1, 1, ATTR1_OBJSIZE);
	

	// Initialize display control register
	*REG_DISPCNT &= ~(1 << 0b111);
	BitSet(REG_DISPCNT, 1, DISPCNT_OBJMAPPING); // 1D
	BitSet(REG_DISPCNT, 1, DISPCNT_OBJFLAG);

	BitSet(BG0CNT, 1, BGXCNT_COLORMODE);


    while(1)
	{
		if( ((*KEYINPUT) & 0b10000) >> KEYPAD_R == 0 )
		{
			BGPAL_MEM[0] = RGB(31, 31, 0);
		}
		else
		{
			BGPAL_MEM[0] = RGB(0, 0, 0);
		}
	}

    return 0;
}
