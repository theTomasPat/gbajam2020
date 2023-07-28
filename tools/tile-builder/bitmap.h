#ifndef __BITMAP_H
#define __BITMAP_H

#include <stdio.h>
#include <stdint.h>


typedef enum
{
    BMPREAD_SUCCESS,
    BMPREAD_FILE_OPEN_ERROR,
    BMPREAD_NOT_BITMAP,
    BMPREAD_INCORRECT_DIMENSIONS,
    BMPREAD_INCORRECT_BITDEPTH,
    BMPREAD_INCORRECT_COMPRESSION
} BmpReadStatus_t;

enum BITMAP_HEADER_SIZE
{
    BITMAPINFOHEADER_SIZE = 40, 
    BITMAPV3HEADER_SIZE = 56, 
    BITMAPV4HEADER_SIZE = 108, 
    BITMAPV5HEADER_SIZE = 124 
};

static char *BITMAP_ERROR_MSG[] =
{
    "Bitmap read successfully",
    "The given file couldn't be read",
    "The given file isn't a valid bitmap",
    "The bitmap has incorrect dimensions.\nWidth and height must be 8, 16, 32, or 64",
    "The bitmap has an incorrect bit depth.\nIt must be 8 bpp",
    "The bitmap must be uncompressed"
};

#define BITMAP_MAX_DIMENSION 64

typedef struct
{
    uint16_t magicNumber;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixelArrayOffset;
} bmp_file_header_t;

typedef struct
{
    uint32_t headerSize;
    int32_t bitmapWidth;
    int32_t bitmapHeight;
    uint16_t numColorPlanes;
    uint16_t colorDepth;
    uint32_t compressionMethod;
    uint32_t pixArrayBytes;
    int32_t horizontalPpm;
    int32_t verticalPpm;
    uint32_t numPaletteColors;
    uint32_t numImportantColors;
} bmp_info_header_t;

typedef struct
{
    char filename[128];
    bmp_file_header_t fileHeader;
    bmp_info_header_t infoHeader;
    uint32_t colorTable[256];
    // TODO: consider making this a fixed-size array
    uint8_t *pixelArray;
} bitmap_image_t;

BmpReadStatus_t Bitmap_Read(bitmap_image_t *bitmap, const char *filename);
//bitmap_image_t *Bitmap_Create(uint32_t numPixels);
void Bitmap_Destroy(bitmap_image_t *bitmap);

#endif
