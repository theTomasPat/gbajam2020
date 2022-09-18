#include <stdint.h>
#include <string.h>
#include "gba.h"
#include "mgba.h"
#include "fixed.h"
#include "256Palette.h"
#include "Bat.h"



#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
	u16 attr0;
	u16 attr1;
	u16 attr2;
	u16 fill;
} OBJ_ATTR;

// data containers for each type of tile
typedef struct { u32 data[8];  } TILE4;
typedef struct { u32 data[16]; } TILE8;

// each "bank" of tiles occupies 16KB, for 32byte tiles, that's 512 per bank
typedef TILE4 CHARBLOCK[512];
typedef TILE8 CHARBLOCK8[256];

#define tile_mem  ( (CHARBLOCK*)0x06000000)
#define tile8_mem ((CHARBLOCK8*)0x06000000)

#define BIT_SET(addr, val, shift) ( *(addr) |= (val << shift) )
#define BIT_CLEAR(addr, shift) ( *addr &= ~(1 << shift) )
#define BIT_CHECK(addr, mask, shift) ( (*(addr) &= (mask)) >> (shift) )

// TODO: simplify the call for BF_SET, ideally the caller shouldn't have to
// know how many bits are in the field and how much to shift them by
#define BF_SET(addr, val, len, shift) (*(addr) = (*(addr)&~(((1 << len)-1) << (shift))) | ((val) << (shift)))


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

typedef struct {
	u16 prev;
	u16 curr;
} InputState;

// The buttons are 0 if pressed, 1 if not pressed
u16 ButtonPressed(InputState *inputs, u16 button) {
	// mask out the button from the prev and curr button states
	u16 prevPressed = (inputs->prev & button);
	u16 currPressed = (inputs->curr & button);

	// since the logic for button presses is inverted, we need to check the
	// "falling edge" to signal a button press. That means the masked input
	// needs to be a lower value than the masked previous input in order to
	// be considered "just pressed"
	if( currPressed < prevPressed ) return 1;
	return 0;
}
u16 ButtonUp(InputState *inputs, u16 button) {
	return (inputs->curr & button) > 0 ? 1 : 0;
}
u16 ButtonDown(InputState *inputs, u16 button) {
	return (inputs->curr & button) < 1 ? 1 : 0;
}

// Update the buffered input states
void UpdateButtonStates(InputState *inputs) {
	inputs->prev = inputs->curr;
	inputs->curr = *KEYINPUT;
}

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

	if( player->x < screenDim->x ) {
		player->x = 0;
		player->velX = 0;
		outOfBounds = 1;
	}
	if( player->x + player->w > screenDim->w ) {
		player->x = screenDim->y - player->x;
		player->velX = 0;
		outOfBounds = 1;
	}
	if( player->y < screenDim->y ) {
		player->y = 0;
		player->velY = 0;
		outOfBounds = 1;
	}
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

void Vsync() {
	while(*VCOUNT_MEM >= 160);  // wait until VDraw
	while(*VCOUNT_MEM <  160);  // wait until VBlank
}


int main(void) {
	mgba_open();

	// Initialize display control register
	*REG_DISPCNT &= ~(1 << 0b111);
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJMAPPING); // 1D
	BIT_SET(REG_DISPCNT, 1, DISPCNT_OBJFLAG);

	BIT_SET(BG0CNT, 1, BGXCNT_COLORMODE);

	InputState inputs = (InputState){0};

	OBJ_ATTR *OAM_objs[128];
	OAM_objs[0] = (OBJ_ATTR *)OAM_MEM;
	OAM_Init();

	// copy the palette data to 0x05000200
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

	ScreenDim screenDim = (ScreenDim){ 0, 0, 240, 160 };
	Player player = (Player){ 0, 60, 80, 16, 16, 0, 0 };

	// copy the sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], BatTiles, BatTilesLen);

	// setup the bat's sprite
	BIT_SET(&OAM_objs[player.oamIdx]->attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr0, ATTR0_DISABLE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPHOR);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPVERT);
	BIT_SET(&OAM_objs[player.oamIdx]->attr1, 1, ATTR1_OBJSIZE);

	fp_t GravityPerFrame = FP(0, 0x4000);

	// print a debug message, viewable in mGBA
	mgba_printf(DEBUG_DEBUG, "Hey GameBoy", 11);

	// Main loop
    while(1)
	{
		Vsync();
		UpdateButtonStates(&inputs);

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
