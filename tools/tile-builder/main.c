// The goal for this program is to take in a list of Bitmap images
// and pack the pixel values into a 1D array (DISPCNT bit 6) that
// can be read into VRAM, keeping track of the offsets and sprite
// sizes (OAM Attr1 Obj Size)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"

// number of 8x8 OBJ tiles that fit into VRAM
#define MAX_BITMAPS 512

#define TILE_WIDTH 8
#define FILENAME_MAX_LEN 128

typedef struct
{
    char name[128];
    uint32_t charNameIdx;
    uint8_t attr0ObjShape;
    uint8_t attr1ObjSize;
} SpriteRecord_t;

typedef struct
{
    uint8_t shape;
    uint8_t size;
} OBJShapeSize_t;


OBJShapeSize_t GetShapeSize(uint8_t width, uint8_t height)
{
    OBJShapeSize_t shapeSize = {0};

    switch(width)
    {
        case 8:
            switch(height)
            {
                case 8:
                    shapeSize.shape = 0;
                    shapeSize.size = 0;
                    break;
                case 16:
                    shapeSize.shape = 2;
                    shapeSize.size = 0;
                    break;
                case 32:
                    shapeSize.shape = 2;
                    shapeSize.size = 1;
                    break;
            }
            break;
        case 16:
            switch(height)
            {
                case 8:
                    shapeSize.shape = 1;
                    shapeSize.size = 0;
                    break;
                case 16:
                    shapeSize.shape = 0;
                    shapeSize.size = 1;
                    break;
                case 32:
                    shapeSize.shape = 2;
                    shapeSize.size = 2;
                    break;
            }
            break;
        case 32:
            switch(height)
            {
                case 8:
                    shapeSize.shape = 1;
                    shapeSize.size = 1;
                    break;
                case 16:
                    shapeSize.shape = 1;
                    shapeSize.size = 2;
                    break;
                case 32:
                    shapeSize.shape = 0;
                    shapeSize.size = 2;
                    break;
                case 64:
                    shapeSize.shape = 2;
                    shapeSize.size = 3;
                    break;
            }
            break;
        case 64:
            switch(height)
            {
                case 32:
                    shapeSize.shape = 1;
                    shapeSize.size = 3;
                    break;
                case 64:
                    shapeSize.shape = 0;
                    shapeSize.size = 3;
                    break;
            }
            break;
    }

    return shapeSize;
}

void WriteFiles(SpriteRecord_t spriteRecords[], uint8_t compiledData[], uint32_t compiledDataLen, uint32_t bitmapsLen)
{
    FILE *outHeaderFile;
    FILE *outImplFile;

    outHeaderFile = fopen("out.h", "w");
    outImplFile = fopen("out.c", "w");

    // write .h file
    fprintf(outHeaderFile, "#ifndef __TILES_H__\n");
    fprintf(outHeaderFile, "#define __TILES_H__\n");
    fprintf(outHeaderFile, "\n\n");
    for(uint32_t i = 0; i < bitmapsLen; i++)
    {
        fprintf(outHeaderFile, "#define SPRITE_%s_CHARNAME %d\n", spriteRecords[i].name, spriteRecords[i].charNameIdx);
        fprintf(outHeaderFile, "#define SPRITE_%s_OBJSHAPE %d\n", spriteRecords[i].name, spriteRecords[i].attr0ObjShape);
        fprintf(outHeaderFile, "#define SPRITE_%s_OBJSIZE %d\n", spriteRecords[i].name, spriteRecords[i].attr1ObjSize);
        fprintf(outHeaderFile, "\n");
    }
    fprintf(outHeaderFile, "\n");
    fprintf(outHeaderFile, "#define SPRITETILES_LEN %d\n", compiledDataLen);
    fprintf(outHeaderFile, "extern const unsigned char SpriteTiles[SPRITETILES_LEN] __attribute__((aligned(4))) __attribute__((visibility(\"hidden\")))");
    fprintf(outHeaderFile, "\n\n");
    fprintf(outHeaderFile, "#endif");



    // write .c file
    fprintf(outImplFile, "#define SPRITETILES_LEN %d\n", compiledDataLen);
    fprintf(outImplFile, "const unsigned char SpriteTiles[SPRITETILES_LEN] __attribute__((aligned(4))) __attribute__((visibility(\"hidden\"))) = {\n\t");
    for(uint32_t i = 0; i < compiledDataLen; i++)
    {
        if(i % 16 == 0 && i != 0)
        {
            fprintf(outImplFile, "\n\t");

        }
        fprintf(outImplFile, "0x%.2x", compiledData[i]);
        if(i != compiledDataLen - 1)
        {
            fprintf(outImplFile, ", ");
        }
    }
    fprintf(outImplFile, "\n};");

    fclose(outHeaderFile);
    fclose(outImplFile);
}

typedef struct
{
    unsigned int x;
    unsigned int y;
} XY_t;

XY_t IdxToXY(unsigned int idx, unsigned int imageWidth)
{
    //XY result = {0};
    //result.y = idx / imageWidth;
    //result.x = idx % imageWidth;
    //return result;
    return (XY_t){ idx % imageWidth, idx / imageWidth };
}

int XYToIdx(XY_t position, int imageWidth)
{
    return (position.y * imageWidth) + position.x;
}

int main(int argc, char **argv)
{
    // there needs to be at least one image provided
    if(argc < 2)
    {
        printf("Usage: main bitmap1.bmp bitmap2.bmp ...\n");
        exit(EXIT_FAILURE);
    }

    bitmap_image_t *bitmaps[MAX_BITMAPS] = {0};
    uint8_t bitmapsLen = argc - 1;

    for(int i = 0; i < bitmapsLen; i++)
    {
        bitmaps[i] = malloc(sizeof(bitmap_image_t));
        BmpReadStatus_t status = Bitmap_Read(bitmaps[i], argv[i + 1]);

        printf("%s::%s\n", argv[i + 1], BITMAP_ERROR_MSG[status]);
        if(status != BMPREAD_SUCCESS)
        {
            exit(EXIT_FAILURE);
        }
    }

    // process the bitmaps
    SpriteRecord_t spriteRecords[MAX_BITMAPS] = {0};
    uint8_t compiledData[32768]; // 32k of memory for OBJ tiles
    uint32_t compiledDataLen = 0; // used as an offset into `compiledData` for the bitmap currently being worked on
    uint32_t compiledDataCursor = 0;
    for(size_t bitmapIdx = 0; bitmapIdx < bitmapsLen; bitmapIdx++)
    {
        uint32_t bitmapWidth = bitmaps[bitmapIdx]->infoHeader.bitmapWidth;
        uint32_t bitmapHeight = bitmaps[bitmapIdx]->infoHeader.bitmapHeight;

        // setup the sprite record for this bitmap
        strncpy(spriteRecords[bitmapIdx].name, bitmaps[bitmapIdx]->filename, FILENAME_MAX_LEN);
        //spriteRecords[bitmapIdx].name = bitmaps[bitmapIdx]->filename;
        spriteRecords[bitmapIdx].charNameIdx = compiledDataLen / 64;

        OBJShapeSize_t shapeSize = GetShapeSize(bitmapWidth, bitmapHeight);
        spriteRecords[bitmapIdx].attr0ObjShape = shapeSize.shape;
        spriteRecords[bitmapIdx].attr1ObjSize = shapeSize.size;

        uint32_t bitmapRowCursor = 0;
        uint8_t bitmapNumTiles = (bitmapWidth * bitmapHeight) / 64;
        for(uint8_t bitmapTileIdx = 0; bitmapTileIdx < bitmapNumTiles; bitmapTileIdx++)
        {
            // tileIdx -> tile coords -> pixel coords -> pixel idx
            XY_t tileCoords = IdxToXY(bitmapTileIdx, bitmapWidth / TILE_WIDTH);
            XY_t pixelCoords = {tileCoords.x * TILE_WIDTH, tileCoords.y * TILE_WIDTH};
            bitmapRowCursor = XYToIdx(pixelCoords, bitmapWidth);

            for(uint8_t iterateY = 0; iterateY < TILE_WIDTH; iterateY++)
            {
                for(uint8_t iterateX = 0; iterateX < TILE_WIDTH; iterateX++)
                {
                    compiledData[compiledDataCursor] = bitmaps[bitmapIdx]->pixelArray[bitmapRowCursor + iterateX];
                    compiledDataCursor++;
                }

                bitmapRowCursor += bitmapWidth;
            }
        }

        // since the bmp's bitdepth is set to 8bpp, bitmapWidth and bitmapHeight
        // have a 1:1 ratio of pixels to bytes
        compiledDataLen += bitmapWidth * bitmapHeight;

        // the bitmap shouldn't be necessary anymore. free it
        Bitmap_Destroy(bitmaps[bitmapIdx]);
    }

    // write the output files
    WriteFiles(spriteRecords, compiledData, compiledDataLen, bitmapsLen);

    return 0;
}

