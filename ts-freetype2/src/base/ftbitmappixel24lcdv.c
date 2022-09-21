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

/**
 lcdv:
 three rows for one row pixel.

 for code, we suppose three rows as one row.
 pitch is 3 times.
 */
#include <freetype/internal/ftdebug.h>

#include <freetype/ftbitmap.h>
#include <freetype/ftbitmapext.h>
#include <freetype/ftbitmappixel.h>
#include <freetype/ftimage.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftsynth.h>

#define PIXEL_LCDV_BYTES_PER_PIXEL 3

DECLARE_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD_V);

DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pDst->pixel_mode = FT_TS_PIXEL_MODE_LCD_V;
    pDst->num_grays  = FT_TS_BITMAP_PIXEL_BLACK + 1;

    pDst->width = pSrc->width;

    pDst->rows  = pSrc->rows;
    if (   pSrc->pixel_mode == FT_TS_PIXEL_MODE_MONO
        || pSrc->pixel_mode == FT_TS_PIXEL_MODE_GRAY)
    {
        pDst->rows *= PIXEL_LCDV_BYTES_PER_PIXEL; 
    }

    pDst->pitch = pDst->width;

    NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(24, FT_TS_PIXEL_MODE_LCD_V)(pDst);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return pBitmap->width;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pBitmap->width = width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return (pBitmap->rows / PIXEL_LCDV_BYTES_PER_PIXEL);
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pBitmap->rows = (rows * PIXEL_LCDV_BYTES_PER_PIXEL);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pBitmap->pitch = pitch;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(24, FT_TS_PIXEL_MODE_LCD_V)
{
    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(24, FT_TS_PIXEL_MODE_LCD_V)(
        pBitmapSrc, pBufferSrc, 1,
        pBitmapDst, pBufferDst, 1);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(24, FT_TS_PIXEL_MODE_LCD_V)
{
    int i;
    UCHAR* pDataSrc = *pBufferSrc;
    UCHAR* pDataDst = *pBufferDst;
    for (i=0; i<PIXEL_LCDV_BYTES_PER_PIXEL; i++)
    {
        memcpy(pDataDst, pDataSrc, pBitmapSrc->pitch);
        pDataSrc += pBitmapSrc->pitch;
        pDataDst += pBitmapDst->pitch;
    }

    (*pBufferDst) = (*pBufferDst) + (aheadDst > 0 ? pBitmapDst->pitch : -pBitmapDst->pitch)
        * PIXEL_LCDV_BYTES_PER_PIXEL;
    (*pBufferSrc) = (*pBufferSrc) + (aheadSrc > 0 ? pBitmapSrc->pitch : -pBitmapSrc->pitch)
        * PIXEL_LCDV_BYTES_PER_PIXEL;
    return 0;
}


DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return pBitmap->pitch*PIXEL_LCDV_BYTES_PER_PIXEL;
}

DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pBuffer += pBitmap->pitch*PIXEL_LCDV_BYTES_PER_PIXEL;
    return pBuffer;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return pSrc[xSrc];
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(24, FT_TS_PIXEL_MODE_LCD_V)
{
    int i;
    if (!value)
    {
        return 0;
    }
    for (i=0; i<PIXEL_LCDV_BYTES_PER_PIXEL; i++)
    {
        pSrc[xSrc+pBitmapSrc->pitch*i] = value;
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(24, FT_TS_PIXEL_MODE_LCD_V)
{
    int i;
    int pitchSrc = pBitmapSrc->pitch*PIXEL_LCDV_BYTES_PER_PIXEL;
    int pitchDst = pBitmapDst->pitch*PIXEL_LCDV_BYTES_PER_PIXEL;
    int startSrc = ySrc*pitchSrc + xSrc;
    int startDst = yDst*pitchDst + xDst;

    for (i=0; i<PIXEL_LCDV_BYTES_PER_PIXEL; i++)
    {
        pDst[startDst+pBitmapDst->pitch*i] = pSrc[startSrc+pBitmapSrc->pitch*i];
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(24, FT_TS_PIXEL_MODE_LCD_V)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(24, FT_TS_PIXEL_MODE_LCD_V)
{
    if ((pSrc[x/8] & (FT_TS_BITMAP_PIXEL_MASK >> (x % 8 ))) != 0)
    {
        pDst[x]                     = FT_TS_BITMAP_PIXEL_BLACK;
        pDst[x+pBitmapDst->pitch]   = FT_TS_BITMAP_PIXEL_BLACK;
        pDst[x+pBitmapDst->pitch*2] = FT_TS_BITMAP_PIXEL_BLACK;
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(24, FT_TS_PIXEL_MODE_LCD_V)
{
    int data = pData[x];
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

DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(24, FT_TS_PIXEL_MODE_LCD_V)
{
    pBitmap->pitch = pBitmap->width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(24, FT_TS_PIXEL_MODE_LCD_V)
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
