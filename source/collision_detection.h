#ifndef __COLLISION_DETECTION_H__
#define __COLLISION_DETECTION_H__

#include "gba.h"


typedef struct Rectangle {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
} Rectangle;

Rectangle Rectangle_Create(u32 x, u32 y, u32 w, u32 h);

u32 CheckCollision_RectRect(Rectangle a, Rectangle b);

#endif
