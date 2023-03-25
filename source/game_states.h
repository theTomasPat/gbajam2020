#ifndef __GAME_STATES_H__
#define __GAME_STATES_H__

#include <stdio.h>

#include "gba.h"
#include "collision_detection.h"
#include "fixed.h"
#include "obj_pool.h"
#include "random.h"
#include "bit_control.h"

#define ARR_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct ScreenDim {
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} ScreenDim;

#define OBJMAXX 511
#define OBJMAXY 255
i32 WrapY(i32 y);
i32 WrapX(i32 x);

typedef struct Player {
	u32 oamIdx;
	i32 x;
	i32 y;
    Rectangle bounding_box;
	fp_t velX;
	fp_t velY;
} __attribute__((aligned (4))) Player;

u16 PlayerCollideBorder(Player *player, ScreenDim *screenDim);
Player Player_Create(u32 oamIdx, u32 x, u32 y, Rectangle bounding_box, fp_t velX, fp_t velY);

void UpdateOBJPos(OBJ_ATTR *obj, int x, int y);

typedef struct ObstacleTile {
    u32 oamIdx;
	i32 y;
	u32 gapSize;
    u32 active;
} ObstacleTile;

#define OBSTACLES_MAX 4
#define OBSTACLE_START_X 275
#define OBSTACLE_TILE_SIZE 32
//#define MAX_TILES_LEN 6
#define MAX_TILES_LEN 12
typedef struct Obstacle {
	i32 x;
	u32 y;
	u32 gapSize;
    u32 active;
    u32 countedScore;
    Rectangle bounding_box_top;
    Rectangle bounding_box_btm;
    ObstacleTile tiles[MAX_TILES_LEN];
} Obstacle;
Obstacle ObstacleCreate(OBJ_ATTR* OAM_objs, OBJPool* obstaclePool, xorshift32_state* randState);
void Obstacle_Clear(Obstacle* obstacle);

void OAM_OBJClear(i32 idx);


typedef struct {
} SplashScreenState;

typedef struct {
    InputState inputs;
    Player player;
    OBJPool obstaclePool;
    ScreenDim screenDim;
    u32 frameCounter;
    u32 score;
    u32 scoreCounterOAMIdxs[4];
    i32 obstacleIdx;
    u32 onTitleScreen;
    fp_t bgHOffset;
    fp_t bgHOffsetRate;
    fp_t GravityPerFrame;
    xorshift32_state randState;
    Obstacle obstacles[OBSTACLES_MAX];
} GameScreenState;

typedef enum {
    GAMESTATE_SPLASHSCREENINIT,
    GAMESTATE_SPLASHSCREEN,
    GAMESTATE_SPLASHSCREENDEINIT,
    GAMESTATE_GAMEINIT,
    GAMESTATE_TITLESCREEN,
    GAMESTATE_GAMESCREEN,
    GAMESTATE_GAMEOVER,
    GAMESTATE_GAMESCREENDEINIT,
    GAMESTATE_COUNT
} GameStates;

GameStates gameState_SplashScreenInit(SplashScreenState *state);
GameStates gameState_SplashScreen(SplashScreenState *state);
GameStates gameState_GameInit(GameScreenState *state);
GameStates gameState_TitleScreen(GameScreenState *state);
GameStates gameState_GameScreen(GameScreenState *state);
GameStates gameState_GameOver(GameScreenState *state);
GameStates gameState_GameScreenDeinit(GameScreenState *state);


#endif
