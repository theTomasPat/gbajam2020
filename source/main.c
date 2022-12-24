#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "mgba.h"
#include "bit_control.h"
#include "fixed.h"
#include "256Palette.h"
#include "Robo.h"
#include "ObstacleTop_End.h"
#include "ObstacleTop_Tile.h"


#define ARR_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

#ifdef __DEBUG__
#define ASSERT(expr) if(!(expr)) { *(uint8_t*)0x4000301 = 1; }
#else
#define ASSERT(expr)
#endif

#define OBJMAXX 511
#define OBJMAXY 255


// TODO: define a bounding box for the sprite
//       define an origin point for the sprite?
typedef struct Player {
	u32 oamIdx;
	u32 x:9;
	u32 y:8;
	u16 w;
	u16 h;
	fp_t velX;
	fp_t velY;
} __attribute__((aligned (4))) Player;

typedef struct ScreenDim {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} ScreenDim;

typedef struct ObstacleTile {
    u32 oamIdx;
	i32 y;
	u32 gapSize;
    u32 active;
} ObstacleTile;

#define MAX_TILES_LEN 4
typedef struct Obstacle {
	i32 x;
	i32 y;
	u32 gapSize;
    u32 active;
    ObstacleTile tilesTop[MAX_TILES_LEN];
    ObstacleTile tilesBottom[MAX_TILES_LEN];
} Obstacle;

typedef struct OBJPool {
    i32* indexes;
    i32 length;
    i32 poolIdx;
} OBJPool;

OBJPool OBJPool_Create(i32 oamStartIdx, i32 len)
{
    // since there's only 128 OAM objs available,
    // make sure the pool doesn't go beyond that
    ASSERT(oamStartIdx + len <= 128);

    OBJPool Result = {0};
    Result.indexes = malloc(sizeof(i32) * len);
    for(size_t i = 0; i < len; i++)
    {
        Result.indexes[i] = oamStartIdx + i;
    }
    Result.length = len;
    Result.poolIdx = 0;

    return Result;
}

// return the current index number and then internally
// increment it in the given struct
i32 OBJPool_GetNextIdx(OBJPool* pool)
{
    i32 Result = pool->poolIdx;

    pool->poolIdx++;
    if(pool->poolIdx >= pool->length)
    {
        pool->poolIdx = 0;
    }

    return Result;
}


// Go through all 128 OAM entries and for each of them,
// zero them out and disable them. This prevents a mess of
// sprites from appearing at 0,0 on initialization
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
	BF_SET(&obj->attr1, x, ATTR1_XCOORD_LEN, ATTR1_XCOORD_SHIFT);
	BF_SET(&obj->attr0, y, ATTR0_YCOORD_LEN, ATTR0_YCOORD_SHIFT);
}

int main(void) {
#ifdef __DEBUG__
	mgba_open();
    char debug_msg[DEBUG_MSG_LEN];
#endif

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
	BIT_SET(BG0CNT, bgMapBaseBlock, BGXCNT_SCRNBASEBLOCK); // select bg map base block
	BIT_SET(BG0CNT, 2, BGXCNT_SCREENSIZE); // select map size (64x32 tiles, or 2x 32x32-tile screens, side-by-side)

    // setup important scene objects
	InputState inputs = (InputState){0};
	ScreenDim screenDim = (ScreenDim){ 0, 0, 240, 160 };
	Player player = (Player){ 0, 60, 80, 32, 32, 0, 0 };
    OBJPool obstaclePool = OBJPool_Create(118, 10);

	// copy the palette data to the BG and OBJ palettes
	memcpy(BGPAL_MEM, Pal256, PalLen256);
	memcpy(OBJPAL_MEM, Pal256, PalLen256);

    // copy all sprite data into VRAM
	memcpy(&tile8_mem[4][0], RoboTiles, RoboTilesLen);
	memcpy(&tile8_mem[4][16], ObstacleTop_EndTiles, ObstacleTop_EndTilesLen);
	memcpy(&tile8_mem[4][32], ObstacleTop_TileTiles, ObstacleTop_TileTilesLen);

    // initialize OAM items
	OBJ_ATTR *OAM_objs;
	OAM_objs = (OBJ_ATTR *)OAM_MEM;
	OAM_Init();

	// setup the robot's sprite
	BIT_SET(&OAM_objs[player.oamIdx].attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[player.oamIdx].attr0, ATTR0_DISABLE);
	BIT_SET(&OAM_objs[player.oamIdx].attr1, 2, ATTR1_OBJSIZE);
	BIT_SET(&OAM_objs[player.oamIdx].attr2, 0, ATTR2_CHARNAME);

#ifdef __DEBUG__
    // test the OBJPool system by making sure the index numbers
    // wrap properly within the pool and also by filling the
    // OAM objs using the pool
    for(size_t i = 0; i < 15; i++)
    {
        i32 poolIdx = OBJPool_GetNextIdx(&obstaclePool);
        i32 objIdx = obstaclePool.indexes[poolIdx];
        snprintf(debug_msg, DEBUG_MSG_LEN,
                "obstaclePool[%ld]: %ld",
                poolIdx, objIdx);
        mgba_printf(DEBUG_DEBUG, debug_msg);
        BIT_SET(&OAM_objs[objIdx].attr0, 1, ATTR0_COLORMODE);
        BIT_SET(&OAM_objs[objIdx].attr1, 2, ATTR1_OBJSIZE);
        BIT_SET(&OAM_objs[objIdx].attr2, 0, ATTR2_CHARNAME);
    }
#endif

	// dummy BG tile art
	// the number corresponds to the color idx in the palette
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
	//BG_TxtMode_Screens[bgMapBaseBlock + 2][0] = smileyTileIdx;
	//BG_TxtMode_Screens[bgMapBaseBlock + 3][0] = smileyTileIdx;


	fp_t GravityPerFrame = FP(0, 0x4000);

#ifdef __DEBUG__
	// print a debug message, viewable in mGBA
	mgba_printf(DEBUG_DEBUG, "Hey GameBoy");
#endif

	fp_t bgHOffset = 0;
	fp_t bgHOffsetRate = FP(0, 0x4000);

	// create an obstacle
#define OBSTACLE_START_X 275
	Obstacle obstacle = {0};
	obstacle.x = OBSTACLE_START_X;
	obstacle.y = 75;
	obstacle.gapSize = 64;
    obstacle.active = 1;
    // TODO: create the obstacle tiles using `obstaclePool`
    obstacle.tilesTop[0].oamIdx = 2;
    obstacle.tilesTop[0].y = obstacle.y - (obstacle.gapSize / 2) - 32; // 32 is sprite height
    obstacle.tilesTop[0].active = 1;
    obstacle.tilesBottom[0].oamIdx = 3;
    obstacle.tilesBottom[0].y = obstacle.y + (obstacle.gapSize / 2);
    obstacle.tilesBottom[0].active = 1;


    // TODO: create OAM objs for the top half of the obstacles;
    //       one for the end of the obstacle at:
    //           obstacle.y - (obstacle.gap / 2 ) - <sprite height>
    //       calculate the number of tiling sprites for the rest
    //       of the top-half, each at:
    //           <sprite height = 32>
    //           obstacle.y - (obstacle.gap / 2) - (<sprite height> * i)
    // TODO: create OAM objs for the btm half of the obstacles;
    //       one for the end of the obstacle at:
    //           obstacle.y + <sprite height>
    //       calculate the number of tiling sprites for the rest
    //       of the btm-half, each at:
    //           <sprite height = 32>
    //           obstacle.y + (obstacle.gap / 2) + (<sprite height> * i)
    //       optionally, use the same art on the bottom as the top,
    //           but flipped vertically

    // calculate the number of tiles needed on the top
    // the division will truncate the number which means that we need to add 1 to make sure
    //     there's a tile that will reach the edge of the screen
    i32 numTilesToTopBorder = obstacle.tilesTop[0].y / 32 + 1;
#ifdef __DEBUG__
    snprintf(debug_msg, DEBUG_MSG_LEN, "Number top tiles: %ld\n", numTilesToTopBorder);
    mgba_printf(DEBUG_DEBUG, debug_msg);
#endif

    // calculate the number of tiles needed on the btm
    i32 numTilesToBtmBorder = (SCREEN_HEIGHT - (obstacle.y + (obstacle.gapSize / 2) + 32)) / 32 + 1;
#ifdef __DEBUG__
    snprintf(debug_msg, DEBUG_MSG_LEN, "Number btm tiles: %ld\n", numTilesToBtmBorder);
    mgba_printf(DEBUG_DEBUG, debug_msg);
#endif

    // TODO: setup the OAM objs for each of the obstacle tiles
	// TODO: generalize adding sprites to avoid maintaining hardcoded values
	// setup the obstacle sprite in OAM
    // obstacle tile #1
	BIT_SET(&OAM_objs[2].attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[2].attr0, ATTR0_DISABLE);
	BIT_SET(&OAM_objs[2].attr1, 2, ATTR1_OBJSIZE);
	// TODO: the starting tile is #16 in the tile set but the char name for
	// this sprite needs to be 32... why? what is a "char name" (attr2)?
	BIT_SET(&OAM_objs[2].attr2, 32, ATTR2_CHARNAME);
    // obstacle tile #2
	BIT_SET(&OAM_objs[3].attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[3].attr0, ATTR0_DISABLE);
	BIT_SET(&OAM_objs[3].attr1, 2, ATTR1_OBJSIZE);
	BIT_SET(&OAM_objs[3].attr2, 32, ATTR2_CHARNAME);
	BIT_SET(&OAM_objs[3].attr1, 1, ATTR1_FLIPVERT);

    // test sprite to see if it wraps from btm to top
	BIT_SET(&OAM_objs[4].attr0, 1, ATTR0_COLORMODE);
	BIT_CLEAR(&OAM_objs[4].attr0, ATTR0_DISABLE);
	BIT_SET(&OAM_objs[4].attr1, 2, ATTR1_OBJSIZE);
	BIT_SET(&OAM_objs[4].attr2, 0, ATTR2_CHARNAME);
    i32 testSpriteY = 0;


	// Main loop
    while(1)
	{
		Vsync();
		UpdateButtonStates(&inputs);

		// scroll the BG
		// BG0HOFS is write-only so we need an extra variable (bgHOffset)
		// to keep track of where the BG should be and assign that to
		// the register
		bgHOffset += bgHOffsetRate;
		if(bgHOffset > FP(511,0)) { bgHOffset -= FP(511,0); }
		*BG0HOFS = FP2INT(bgHOffset);

		// move player
		player.velY += GravityPerFrame;
		if(ButtonPressed(&inputs, KEYPAD_A))
		{
			player.velY = INT2FP(-3);
		}
		if( (i16)player.y + FP2INT(player.velY) > 0 ) {
			player.y += FP2INT(player.velY);
		}
		else {
			player.y = 0;
		}
		CollideBorder(&player, &screenDim);

		// update the player sprite
		UpdateOBJPos(
            &OAM_objs[player.oamIdx],
            player.x,
            player.y
        ); 

		// update the obstacle
		// TODO: use fp_t for obstacle coords?
        // TODO: create routines for initializing and deactivatingobstacles
		obstacle.x -= 1;
        if(obstacle.x <= -32)
        {
            obstacle.x = OBSTACLE_START_X;
        }
        // in order to get the obstacle to move off the left side,
        // the obj must wrap back in from the right side of the map
        i32 wrapX = obstacle.x <= 0 ? (OBJMAXX + obstacle.x) : obstacle.x;
        for(i32 i = 0; i < MAX_TILES_LEN; i++)
        {
            if(obstacle.tilesTop[i].active != 0)
            {
                UpdateOBJPos(
                        &OAM_objs[obstacle.tilesTop[i].oamIdx],
                        wrapX,
                        obstacle.tilesTop[i].y
                        );
            }
            if(obstacle.tilesBottom[i].active != 0)
            {
                UpdateOBJPos(
                        &OAM_objs[obstacle.tilesBottom[i].oamIdx],
                        wrapX,
                        obstacle.tilesBottom[i].y
                        );
            }
        }

        // test item to check vertical wrapping
        testSpriteY = testSpriteY >= OBJMAXY ? 0 : testSpriteY + 1;
        UpdateOBJPos(
                &OAM_objs[4],
                20,
                testSpriteY
                );
	}

#ifdef __DEBUG__
	mgba_close();
#endif

    return 0;
}
