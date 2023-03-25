#ifndef __OBJPOOL_H__
#define __OBJPOOL_H__

#include <stdlib.h>

#include "gba.h"
#include "mgba.h"

typedef struct OBJPool {
    i32* indexes;
    i32 length;
    i32 poolIdx;
} OBJPool;

OBJPool OBJPool_Create(i32 oamStartIdx, i32 len);
i32 OBJPool_GetNextIdx(OBJPool* pool);

#endif
