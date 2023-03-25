#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gba.h"
#include "mgba.h"
#include "bit_control.h"
#include "fixed.h"
#include "random.h"
#include "collision_detection.h"
#include "game_states.h"


int main(void)
{

#ifdef __DEBUG__
	mgba_open();
#endif

    // pointers for the different game states
    GameScreenState *gameScreenState;
    SplashScreenState *splashScreenState;

    GameStates gameState = GAMESTATE_SPLASHSCREENINIT;
    while(1)
    {
        switch(gameState)
        {
            case GAMESTATE_SPLASHSCREENINIT:
                splashScreenState = (SplashScreenState *)malloc(sizeof(SplashScreenState));
                gameState = gameState_SplashScreenInit(splashScreenState);
                break;
            case GAMESTATE_SPLASHSCREEN:
                gameState = gameState_SplashScreen(splashScreenState);
                break;
            case GAMESTATE_SPLASHSCREENDEINIT:
                free(splashScreenState);
                gameState = GAMESTATE_GAMEINIT;
                break;
            case GAMESTATE_GAMEINIT:
                gameScreenState = (GameScreenState *)malloc(sizeof(GameScreenState));
                gameState = gameState_GameInit(gameScreenState);
                break;
            case GAMESTATE_TITLESCREEN:
                gameState = gameState_TitleScreen(gameScreenState);
                break;
            case GAMESTATE_GAMESCREEN:
                gameState = gameState_GameScreen(gameScreenState);
                break;
            case GAMESTATE_GAMEOVER:
                gameState = gameState_GameOver(gameScreenState);
                break;
            case GAMESTATE_GAMESCREENDEINIT:
                gameState = gameState_GameScreenDeinit(gameScreenState);
                free(gameScreenState);
                break;
            default:
                ASSERT(0);
                break;
        }
    }

    /* TODO: animation controller
     *     - player animation
     *     - title screen (finger pressing button)
     *     - tweening?
	 */

#ifdef __DEBUG__
	mgba_close();
#endif

    return 0;
}
