#include <stdio.h>

#include "obj_pool.h"


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

    pool->poolIdx += 1;
    if(pool->poolIdx >= pool->length)
    {
        pool->poolIdx = 0;
    }

#if __DEBUG__
    char debug_msg[DEBUG_MSG_LEN];
    snprintf(debug_msg, DEBUG_MSG_LEN, "poolIdx = %ld", Result);
    mgba_printf(DEBUG_DEBUG, debug_msg);
#endif

    return Result;
}

