#include <stdlib.h>

#include "animation.h"


Animation
*Animation_Create(u32 frames[], u32 framesLen, u32 fps, u8 loop)
{
	Animation *Return = calloc(1, sizeof(Animation) + sizeof(u32) * framesLen);
    if(!Return) return NULL;

	Return->timer = 0;
	Return->framesLen = framesLen;
	Return->curFrame = 0;
	Return->ticksPerFrame = 60 / fps;
	Return->state = ANIMSTATE_STOPPED;
	Return->loop = loop;

    for(size_t i = 0; i < framesLen; i++)
    {
        Return->frames[i] = frames[i];
    }

	return Return;
}

void
Animation_Destroy(Animation *anim)
{
    free(anim);
}

void
Animation_Update(Animation *anim, u32 ticks)
{
    if(anim->state == ANIMSTATE_STOPPED) return;

    anim->timer += ticks;
    
    anim->curFrame = anim->timer / anim->ticksPerFrame;
    if(anim->curFrame >= anim->framesLen)
    {
        if(anim->loop)
        {
            anim->curFrame = anim->curFrame % anim->framesLen;
        }
        else
        {
            anim->curFrame = anim->framesLen - 1;
            anim->state = ANIMSTATE_STOPPED;
        }
    }
}

void
Animation_Play(Animation *anim)
{
    anim->state = ANIMSTATE_PLAYING;
}

void
Animation_Pause(Animation *anim)
{
    anim->state = ANIMSTATE_STOPPED;
}

void
Animation_Restart(Animation *anim)
{
    anim->timer = 0;
    anim->curFrame = 0;
    anim->state = ANIMSTATE_PLAYING;
}

void
Animation_SetFrame(Animation *anim, u32 frameIndex)
{
    anim->curFrame = anim->frames[frameIndex];
    anim->timer = anim->curFrame * anim->ticksPerFrame;
}
