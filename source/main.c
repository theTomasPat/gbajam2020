#include <stdint.h>
#include <string.h>

#include "gba.h"
#include "mgba.h"
#include "bit_control.h"
#include "fixed.h"
#include "256Palette.h"
#include "Robo.h"


#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))



// TODO: define a bounding box for the sprite
//       define an origin point for the sprite?
typedef struct {
	u32 oamIdx;
	u32 x:9;
	u32 y:8;
	u16 w;
	u16 h;
	fp_t velX;
	fp_t velY;
} __attribute__((aligned (4))) Player;

typedef struct {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} ScreenDim;


void OAM_Init() {
	OBJ_ATTR *obj = (OBJ_ATTR *)OAM_MEM;

	for(int i = 0; i < 128; i++)
	{
		*obj = (OBJ_ATTR){0};
		BIT_SET(&obj->attr0, 1, ATTR0_DISABLE);

		obj++;
	}
}

u16 CollideBorder(Player *player, ScreenDim *screenDim) {
	u16 outOfBounds = 0;

	// check left
	if( player->x < screenDim->x ) {
		player->x = 0;
		player->velX = 0;
		outOfBounds = 1;
	}

	// check right
	if( player->x + player->w > screenDim->w ) {
		player->x = screenDim->y - player->x;
		player->velX = 0;
		outOfBounds = 1;
	}

	// check top
	if( player->y < screenDim->y ) {
		player->y = 0;
		player->velY = 0;
		outOfBounds = 1;
	}

	// check bottom
	if( player->y + player->h > screenDim->h ) {
		player->y = screenDim->h - player->h;
		player->velY = 0;
		outOfBounds = 1;
	}

	if( outOfBounds > 0 )
		return 1;
	return 0;
}

void UpdateOBJPos(OBJ_ATTR *obj, int x, int y) {
	BF_SET(&obj->attr1, x, 9, ATTR1_XCOORD_SHIFT);
	BF_SET(&obj->attr0, y, 8, ATTR0_YCOORD_SHIFT);
}

int main(void) {
	mgba_open();

	// Initialize display control register
	*REG_DISPCNT = 0;
	BIT_CLEAR(REG_DISPCNT, DISPCNT_BGMODE_SHIFT); // set BGMode 0
	BIT_SET(REG_DISPCNT, 1, DISPCNT_BG0FLAG_SHIFT); // turn on BG0
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJMAPPING_SHIFT); // 1D mapping
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJFLAG_SHIFT); // show OBJs

	// Initialize BG0's attributes
	u32 bgMapBaseBlock = 28;
	u32 bgCharBaseBlock = 0;
	*BG0CNT = 0;
	BIT_SET(BG0CNT, 1, BGXCNT_COLORMODE); // 256 color palette
	BIT_SET(BG0CNT, bgCharBaseBlock, BGXCNT_CHARBASEBLOCK);  // select bg tile base block
	BIT_SET(BG0CNT, bgMapBaseBlock, 8); // select bg map base block
	BIT_SET(BG0CNT, 3, 14); // select map size (64x64 tiles, or 4x 32x32-tile screens)

	InputState inputs = (InputState){0};

	OBJ_ATTR *OAM_objs[128];
	OAM_objs[0] = (OBJ_ATTR *)OAM_MEM;
	OAM_Init();

	// copy the palette data to the BG and OBJ palettes
	memcpy(BGPAL_MEM, Pal256, PalLen256);
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

	ScreenDim screenDim = (ScreenDim){ 0, 0, 240, 160 };
	Player player = (Player){ 0, 60, 80, 32, 32, 0, 0 };
	
	// copy the player sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], RoboTiles, RoboTilesLen);

	// setup the robot's sprite
	BIT_SET(&OAM_objs[player.oamIdx]->attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr0, ATTR0_DISABLE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPHOR);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPVERT);
	BIT_SET(&OAM_objs[player.oamIdx]->attr1, 2, ATTR1_OBJSIZE);

	
	// dummy BG tile art
	char smileyTile[64] = {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 1, 0, 0, 1, 0, 0,
		1, 0, 1, 0, 0, 1, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 1,
		0, 1, 0, 0, 0, 0, 1, 0,
		0, 0, 1, 1, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	memcpy(&tile8_mem[0][1], &smileyTile, 64);
	
	// Create a basic tilemap
	BG_TxtMode_Tile smileyTileIdx = (1 << 0);
	*BG_TxtMode_Screens[0] = 0;
	BG_TxtMode_Screens[bgMapBaseBlock + 0][0] = smileyTileIdx;
	BG_TxtMode_Screens[bgMapBaseBlock + 1][0] = smileyTileIdx;
	BG_TxtMode_Screens[bgMapBaseBlock + 2][0] = smileyTileIdx;
	BG_TxtMode_Screens[bgMapBaseBlock + 3][0] = smileyTileIdx;


	fp_t GravityPerFrame = FP(0, 0x4000);

	// print a debug message, viewable in mGBA
	mgba_printf(DEBUG_DEBUG, "Hey GameBoy", 11);

	u16 bgHOffset = 0;

	// Main loop
    while(1)
	{
		Vsync();
		UpdateButtonStates(&inputs);

		// scroll the BG
		bgHOffset += 1;
		if(bgHOffset > 511) bgHOffset -= 511;
		*BG0HOFS = bgHOffset;

		if( ButtonPressed(&inputs, KEYPAD_A) )
		{
			player.velY = INT2FP(-4);
		}

		// Get the sprite to move
		player.velY += GravityPerFrame;
		if( (s16)player.y + FP2INT(player.velY) > 0 ) {
			player.y += FP2INT(player.velY);
		}
		else {
			player.y = 0;
		}
		CollideBorder(&player, &screenDim);
		UpdateOBJPos(OAM_objs[player.oamIdx], player.x, player.y); 
	}

	// disable mGBA debugging
	mgba_close();

    return 0;
}
