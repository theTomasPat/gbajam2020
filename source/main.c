#include <stdint.h>
#include <string.h>
#include "gba.h"
#include "256Palette.h"
#include "Bat.h"

#define LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
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

// TODO: simplify the call for BF_SET, ideally the caller shouldn't have to
// know how many bits are in the field and how much to shift them by
#define BF_SET(addr, val, len, shift) (*(addr) = (*(addr)&~(((1 << len)-1) << (shift))) | ((val) << (shift)))


typedef struct {
	uint32_t oamIdx;
	uint32_t x:9;
	uint32_t y:8;
	uint16_t w;
	uint16_t h;
	uint32_t velX;
	uint32_t velY;
} __attribute__((aligned (4))) Player;

typedef struct {
	uint16_t x;
	uint16_t y;
	uint16_t w;
	uint16_t h;
} ScreenDim;

// TODO: cache the input states so we can detect the different button actions
typedef struct {
	uint16_t prev;
	uint16_t curr;
} InputState;

// The buttons are 0 if pressed, 1 if not pressed
uint16_t ButtonPressed(InputState *inputs, uint16_t button) {
	// mask out the button from the prev and curr button states
	uint16_t prevPressed = (inputs->prev & button);
	uint16_t currPressed = (inputs->curr & button);

	// since the logic for button presses is inverted, we need to check the
	// "falling edge" to signal a button press. That means the masked input
	// needs to be a lower value than the masked previous input in order to
	// be considered "just pressed"
	if( currPressed < prevPressed ) return 1;
	return 0;
}
uint16_t ButtonUp(InputState *inputs, uint16_t button) {
	return (inputs->curr & button) > 0 ? 1 : 0;
}
uint16_t ButtonDown(InputState *inputs, uint16_t button) {
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

uint16_t CollideBorder(Player *player, ScreenDim *screenDim) {
	uint16_t outOfBounds = 0;

	if( player->x < screenDim->x ) {
		player->x = 0;
		outOfBounds = 1;
	}
	if( player->x + player->w > screenDim->w ) {
		player->x = screenDim->y - player->x;
		outOfBounds = 1;
	}
	if( player->y < screenDim->y ) {
		player->y = 0;
		outOfBounds = 1;
	}
	if( player->y + player->h > screenDim->h ) {
		player->y = screenDim->h - player->h;
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
	Player player = (Player){ 0, 120, 80, 16, 16, 0, 0 };

	// copy the sprite data to 0x06010000
	memcpy(&tile8_mem[4][0], BatTiles, BatTilesLen);

	// setup the bat's sprite
	BIT_SET(&OAM_objs[player.oamIdx]->attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr0, ATTR0_DISABLE);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPHOR);
	BIT_CLEAR(&OAM_objs[player.oamIdx]->attr1, ATTR1_FLIPVERT);
	BIT_SET(&OAM_objs[player.oamIdx]->attr1, 1, ATTR1_OBJSIZE);

	// Main loop
    while(1)
	{
		Vsync();
		UpdateButtonStates(&inputs);

		if( ButtonDown(&inputs, KEYPAD_A) )
		{
			BGPAL_MEM[0] = RGB(31, 31, 0);
		}
		else
		{
			BGPAL_MEM[0] = RGB(0, 0, 0);
		}

		// Get the sprite to move
		player.y += 1;
		CollideBorder(&player, &screenDim);
		UpdateOBJPos(OAM_objs[player.oamIdx], player.x, player.y); 
	}

    return 0;
}
