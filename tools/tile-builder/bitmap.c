#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"

BmpReadStatus_t Bitmap_Read(bitmap_image_t *bitmap, const char *filename)
{
    char *filenameWithoutPath = strrchr(filename, '/');
    strncpy(bitmap->filename, (filenameWithoutPath) ? filenameWithoutPath+1 : filename, 128);
    char *period = strchr(bitmap->filename, '.');
    if(period != NULL)
    {
        *period = '\0';
    }

    // open file
    FILE *bitmap_file = fopen(filename, "rb");
    if(!bitmap_file)
    {
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_FILE_OPEN_ERROR]);
        return BMPREAD_FILE_OPEN_ERROR;
    }

    // create bitmap object
    //bitmap_image_t *bitmap = malloc(sizeof(bitmap_image_t));
    //if(!bitmap) return NULL;

    // read file header
    fread(&bitmap->fileHeader.magicNumber, sizeof(uint16_t), 1, bitmap_file);
    if(bitmap->fileHeader.magicNumber != 0x4d42)
    {
        fclose(bitmap_file);
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_NOT_BITMAP]);
        return BMPREAD_NOT_BITMAP;
    }

    fread(&bitmap->fileHeader.fileSize, sizeof(uint32_t), 1, bitmap_file);
    fread(&bitmap->fileHeader.reserved1, sizeof(uint16_t), 1, bitmap_file);
    fread(&bitmap->fileHeader.reserved2, sizeof(uint16_t), 1, bitmap_file);
    fread(&bitmap->fileHeader.pixelArrayOffset, sizeof(uint32_t), 1, bitmap_file);

    // read info header
    fread(&bitmap->infoHeader.headerSize, sizeof(uint32_t), 1, bitmap_file);
    // headerSize == 40; BITMAPINFOHEADER
    // headerSize == 56; BITMAPV3HEADER
    // headerSize == 108; BITMAPV4HEADER
    // headerSize == 124; BITMAPV5HEADER

    fread(&bitmap->infoHeader.bitmapWidth, sizeof(int32_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.bitmapHeight, sizeof(int32_t), 1, bitmap_file);

    // check width and height match GBA's sprite sizes
    int width = bitmap->infoHeader.bitmapWidth;
    if(width <= 0 || width > BITMAP_MAX_DIMENSION || bitmap->infoHeader.bitmapWidth % 8 != 0)
    {
        fclose(bitmap_file);
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_INCORRECT_DIMENSIONS]);
        return BMPREAD_INCORRECT_DIMENSIONS;
    }
    int height = bitmap->infoHeader.bitmapHeight;
    if(height <= 0 || height > BITMAP_MAX_DIMENSION || bitmap->infoHeader.bitmapHeight % 8 != 0)
    {
        fclose(bitmap_file);
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_INCORRECT_DIMENSIONS]);
        return BMPREAD_INCORRECT_DIMENSIONS;
    }

    fread(&bitmap->infoHeader.numColorPlanes, sizeof(uint16_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.colorDepth, sizeof(uint16_t), 1, bitmap_file);

    if(bitmap->infoHeader.colorDepth != 8)
    {
        fclose(bitmap_file);
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_INCORRECT_BITDEPTH]);
        return BMPREAD_INCORRECT_BITDEPTH;
    }

    fread(&bitmap->infoHeader.compressionMethod, sizeof(uint32_t), 1, bitmap_file);
    if(bitmap->infoHeader.compressionMethod != 0)
    {
        fclose(bitmap_file);
        printf("%s\n", BITMAP_ERROR_MSG[BMPREAD_INCORRECT_COMPRESSION]);
        return BMPREAD_INCORRECT_COMPRESSION;
    }

    fread(&bitmap->infoHeader.pixArrayBytes, sizeof(uint32_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.horizontalPpm, sizeof(int32_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.verticalPpm, sizeof(int32_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.numPaletteColors, sizeof(uint32_t), 1, bitmap_file);
    fread(&bitmap->infoHeader.numImportantColors, sizeof(uint32_t), 1, bitmap_file);

    // up to this point, we've read 40 bytes of the info header
    // as long as the bitmap's compression method is set to 0, those 40 bytes
    // are sufficient to finish processing the bitmap
    //
    // if the bitmap has a v3 header, burn through 16 bytes before reading the color table
    // if the bitmap has a v4 header, burn through 68 bytes before reading the color table
    // if the bitmap has a v5 header, burn through 84 bytes before reading the color table
    fseek(bitmap_file, bitmap->infoHeader.headerSize - 40, SEEK_CUR);

    // read the color table
    fread(bitmap->colorTable, sizeof(uint32_t), bitmap->infoHeader.numPaletteColors, bitmap_file);

    // allocate memory for pixel array
    bitmap->pixelArray = (uint8_t *)malloc(sizeof(uint8_t) * (bitmap->infoHeader.bitmapWidth * bitmap->infoHeader.bitmapHeight));

    // read pixel array
    int stride = bitmap->infoHeader.pixArrayBytes / bitmap->infoHeader.bitmapHeight;
    int paddingBytesLen = stride - bitmap->infoHeader.bitmapWidth;
    int row = bitmap->infoHeader.bitmapHeight - 1;
    uint8_t *pixArrayOffset = bitmap->pixelArray + (bitmap->infoHeader.bitmapWidth * row);
    uint8_t tmpBuffer[4];
    while(1)
    {
        // read the pixel values
        if(fread(pixArrayOffset, sizeof(uint8_t), bitmap->infoHeader.bitmapWidth, bitmap_file) == 0)
            break;
        pixArrayOffset -= bitmap->infoHeader.bitmapWidth;

        if(paddingBytesLen == 0)
        {
            continue;
        }
        else
        {
            // seek forward over the padding bytes
            if(fread(tmpBuffer, sizeof(uint8_t), paddingBytesLen, bitmap_file) == 0)
                break;
        }
    }

    // close file
    fclose(bitmap_file);

    // return bitmap object
    return BMPREAD_SUCCESS;
}

void Bitmap_Destroy(bitmap_image_t *bitmap)
{
    // deallocate pixel array memory
    free(bitmap->pixelArray);

    // deallocate bitmap object
    free(bitmap);
}
