#include <stdint.h>
#include <string.h>
#include "gba.h"
#include "256Palette.h"
#include "Bat.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct
{
	uint16_t attr0;
	uint16_t attr1;
	uint16_t attr2;
	uint16_t fill;
} OBJ_ATTR;

// data containers for each type of tile
typedef struct { uint32_t data[8];  } TILE4;
typedef struct { uint32_t data[16]; } TILE8;

// each "bank" of tiles occupies 16KB, for 32byte tiles, that's 512 per bank
typedef TILE4 CHARBLOCK[512];
typedef TILE8 CHARBLOCK8[256];

#define tile_mem  ( (CHARBLOCK*)0x06000000)
#define tile8_mem ((CHARBLOCK8*)0x06000000)

#define BIT_SET(addr, val, shift) ( *(addr) |= (val << shift) )
#define BIT_CLEAR(addr, shift) ( *addr &= ~(1 << shift) )
#define BIT_CHECK(addr, mask, shift) ( (*(addr) &= (mask)) >> (shift) )
#define BF_SET(addr, val, len, shift) (*(addr) = (*(addr)&~(((1 << len)-1) << (shift))) | ((val) << (shift)))


typedef struct
{
	uint32_t oamIdx;
	uint32_t x:9;
	uint32_t y:8;
	uint32_t velX;
	uint32_t velY;
} __attribute__((aligned (4))) Player;

void OAM_Init()
{
	OBJ_ATTR *obj = (OBJ_ATTR *)OAM_MEM;

	for(int i = 0; i < 128; i++)
	{
		*obj = (OBJ_ATTR){0};
		BIT_SET(&obj->attr0, 1, ATTR0_DISABLE);

		obj++;
	}
}

void UpdateOBJPos(OBJ_ATTR *obj, int x, int y)
{
	BF_SET(&obj->attr1, x, 9, ATTR1_XCOORD_SHIFT);
	BF_SET(&obj->attr0, y, 8, ATTR0_YCOORD_SHIFT);
}

void vid_vsync()
{
	while(*VCOUNT_MEM >= 160); // wait until VDraw
	while(*VCOUNT_MEM <  160);  // wait until VBlank
}


int main(void)
{
	OBJ_ATTR *OAM_objs[128];
	OAM_objs[0] = (OBJ_ATTR *)OAM_MEM;

	int batIndex = 0;

	OAM_Init();

	// copy the palette data to 0x05000200
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

	// copy the sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], BatTiles, BatTilesLen);

	// setup the bat's sprite
	BIT_SET(&OAM_objs[batIndex]->attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[batIndex]->attr0, ATTR0_DISABLE);
	BIT_CLEAR(&OAM_objs[batIndex]->attr1, ATTR1_FLIPHOR);
	BIT_CLEAR(&OAM_objs[batIndex]->attr1, ATTR1_FLIPVERT);
	BIT_SET(&OAM_objs[batIndex]->attr1, 1, ATTR1_OBJSIZE);
	

	// Initialize display control register
	*REG_DISPCNT &= ~(1 << 0b111);
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJMAPPING); // 1D
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJFLAG);

	BIT_SET(BG0CNT, 1, BGXCNT_COLORMODE);

	Player player = (Player){ 0, 120, 80, 0, 0 };

    while(1)
	{
		vid_vsync();

		if( ((*KEYINPUT) & 0b10000) >> KEYPAD_R == 0 )
		{
			BGPAL_MEM[0] = RGB(31, 31, 0);
		}
		else
		{
			BGPAL_MEM[0] = RGB(0, 0, 0);
		}

		// Get the sprite to move
		player.x += 1;
		player.y += 2;
		UpdateOBJPos(OAM_objs[player.oamIdx], player.x, player.y); 
	}

    return 0;
}
