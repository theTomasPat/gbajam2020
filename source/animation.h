#ifndef __ANIMATION_H__
#define __ANIMATION_H__

#include "gba.h"

typedef enum AnimState
{
    ANIMSTATE_STOPPED,
    ANIMSTATE_PLAYING
} AnimState;

typedef struct Animation
{
    u32 timer;
    u32 curFrame;
    u32 ticksPerFrame;
    AnimState state;
	// loop: bool, 0-don't loop, 1-loop
    u8 loop;
    u32 framesLen;
    u32 frames[];
} Animation;

Animation *Animation_Create(u32 *frames, u32 framesLen, u32 fps, u8 loop);
void Animation_Destroy(Animation *anim);
void Animation_Update(Animation *anim, u32 ticks);
void Animation_Play(Animation *anim);
void Animation_Pause(Animation *anim);
void Animation_Restart(Animation *anim);
void Animation_SetFrame(Animation *anim, u32 frameIndex);

#endif
