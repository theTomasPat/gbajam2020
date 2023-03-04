#include "collision_detection.h"
#include "mgba.h"
#include <stdio.h>

Rectangle
Rectangle_Create(u32 x, u32 y, u32 w, u32 h)
{
    Rectangle Return = {0};

    Return.x = x;
    Return.y = y;
    Return.w = w;
    Return.h = h;

    return Return;
}

u32
CheckCollision_RectRect(Rectangle a, Rectangle b)
{
    u32 collisionX = 0;
    u32 collisionY = 0;

    collisionX = (a.x + a.w >= b.x) && (b.x + b.w >= a.x);
    collisionY = (a.y + a.h >= b.y) && (b.y + b.h >= a.y);

#if 0
    char debug_msg[DEBUG_MSG_LEN];
    snprintf(debug_msg, DEBUG_MSG_LEN, "is collision?: %ld", collisionX && collisionY);
    mgba_printf(DEBUG_DEBUG, debug_msg);
#endif

    return collisionX && collisionY;
}
