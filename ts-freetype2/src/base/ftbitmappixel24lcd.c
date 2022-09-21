/****************************************************************************
 *
 * ftbitmapbit08.c
 *
 *   FreeType utility functions for bitmaps (body).
 *
 * Copyright (C) 2004-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/**
 TSIT {{{{{{{{{{
 */

#include <freetype/internal/ftdebug.h>

#include <freetype/ftbitmap.h>
#include <freetype/ftbitmapext.h>
#include <freetype/ftbitmappixel.h>
#include <freetype/ftimage.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftsynth.h>

#define PIXEL_LCD_BYTES_PER_PIXEL 3

DECLARE_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD);

DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(24, FT_TS_PIXEL_MODE_LCD)
{
    pDst->pixel_mode = FT_TS_PIXEL_MODE_LCD;
    pDst->num_grays  = FT_TS_BITMAP_PIXEL_BLACK + 1;

    pDst->width = pSrc->width;
    if (   pSrc->pixel_mode == FT_TS_PIXEL_MODE_MONO
        || pSrc->pixel_mode == FT_TS_PIXEL_MODE_GRAY)
    {

        pDst->width *= PIXEL_LCD_BYTES_PER_PIXEL; 
    }

    pDst->rows  = pSrc->rows;
    pDst->pitch = pDst->width;

    NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(24, FT_TS_PIXEL_MODE_LCD)(pDst);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(24, FT_TS_PIXEL_MODE_LCD)
{
    return pBitmap->width / PIXEL_LCD_BYTES_PER_PIXEL;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(24, FT_TS_PIXEL_MODE_LCD)
{
    pBitmap->width = width * PIXEL_LCD_BYTES_PER_PIXEL; 
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(24, FT_TS_PIXEL_MODE_LCD)
{
    return pBitmap->rows;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(24, FT_TS_PIXEL_MODE_LCD)
{
    pBitmap->rows = rows;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(24, FT_TS_PIXEL_MODE_LCD)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(24, FT_TS_PIXEL_MODE_LCD)
{
    pBitmap->pitch = pitch;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(24, FT_TS_PIXEL_MODE_LCD)
{
    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(24, FT_TS_PIXEL_MODE_LCD)(
        pBitmapSrc, pBufferSrc, 1,
        pBitmapDst, pBufferDst, 1);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(24, FT_TS_PIXEL_MODE_LCD)
{
    memcpy(*pBufferDst, *pBufferSrc, pBitmapSrc->pitch);

    (*pBufferDst) = (*pBufferDst) + (aheadDst > 0 ? pBitmapDst->pitch : -pBitmapDst->pitch);
    (*pBufferSrc) = (*pBufferSrc) + (aheadSrc > 0 ? pBitmapSrc->pitch : -pBitmapSrc->pitch);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(24, FT_TS_PIXEL_MODE_LCD)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(24, FT_TS_PIXEL_MODE_LCD)
{
    pBuffer += pBitmap->pitch;
    return pBuffer;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(24, FT_TS_PIXEL_MODE_LCD)
{
    return pSrc[xSrc*PIXEL_LCD_BYTES_PER_PIXEL];
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(24, FT_TS_PIXEL_MODE_LCD)
{
    int i;
    int start;
    if (!value)
    {
        return 0;
    }

    start = xSrc*PIXEL_LCD_BYTES_PER_PIXEL;
    for (i=0; i<PIXEL_LCD_BYTES_PER_PIXEL; i++)
    {
        pSrc[start+i] = value;
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(24, FT_TS_PIXEL_MODE_LCD)
{
    int i;
    int pitchSrc = pBitmapSrc->pitch;
    int pitchDst = pBitmapDst->pitch;
    int startSrc = ySrc*pitchSrc + xSrc*PIXEL_LCD_BYTES_PER_PIXEL;
    int startDst = yDst*pitchDst + xDst*PIXEL_LCD_BYTES_PER_PIXEL;
    for (i=0; i<PIXEL_LCD_BYTES_PER_PIXEL; i++)
    {
        UCHAR value = pSrc[startSrc+i];
        if (value)
        {
            pDst[startDst+i] = value;
        }
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(24, FT_TS_PIXEL_MODE_LCD)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(24, FT_TS_PIXEL_MODE_LCD)
{
    if ((pSrc[x/8] & (FT_TS_BITMAP_PIXEL_MASK >> (x % 8 ))) != 0)
    {
        int start = x*PIXEL_LCD_BYTES_PER_PIXEL;
        memset(pDst + start, FT_TS_BITMAP_PIXEL_BLACK, PIXEL_LCD_BYTES_PER_PIXEL);
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(24, FT_TS_PIXEL_MODE_LCD)
{
    int data = pData[x*PIXEL_LCD_BYTES_PER_PIXEL];
    if (data == 0)
    {
        pBuffer[pos] = ' ';
    }
    else
    {
        data = (data >> 4) & 0x0F;
        if (0 <= data && data <= 9)
        {
            pBuffer[pos] = (UCHAR)(data + '0');
        }
        else
        {
            data -= 10;
            pBuffer[pos] = (UCHAR)(data + 'A');
        }
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(24, FT_TS_PIXEL_MODE_LCD)
{
    if (pBitmap->pixel_mode == FT_TS_PIXEL_MODE_MONO)
    {
        pBitmap->width *= PIXEL_LCD_BYTES_PER_PIXEL;
    }
    pBitmap->pitch  = pBitmap->width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(24, FT_TS_PIXEL_MODE_LCD)
{
    int size = pBitmap->pitch * pBitmap->rows;
    pBitmap->buffer = (UCHAR*)malloc(size);
    memset(pBitmap->buffer, 0, size);
    return size;
}

/**
 TSIT }}}}}}}}}}
 */

/* END */
