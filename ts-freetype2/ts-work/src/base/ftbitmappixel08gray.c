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

#define FT_TS_PIXEL_GRAY_BLACK      0xFF

DECLARE_FT_TS_BITMAP_PIXEL_INIT(8, FT_TS_PIXEL_MODE_GRAY);

DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(8, FT_TS_PIXEL_MODE_GRAY)
{
    pDst->pixel_mode = FT_TS_PIXEL_MODE_GRAY;
    pDst->num_grays  = FT_TS_BITMAP_PIXEL_BLACK + 1;

    pDst->width = pSrc->width;
    pDst->rows  = pSrc->rows;
    pDst->pitch = pDst->width;

    NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(8, FT_TS_PIXEL_MODE_GRAY)(pDst);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(8, FT_TS_PIXEL_MODE_GRAY)
{
    return pBitmap->width;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(8, FT_TS_PIXEL_MODE_GRAY)
{
    pBitmap->width = width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(8, FT_TS_PIXEL_MODE_GRAY)
{
    return pBitmap->rows;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(8, FT_TS_PIXEL_MODE_GRAY)
{
    pBitmap->rows = rows;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(8, FT_TS_PIXEL_MODE_GRAY)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(8, FT_TS_PIXEL_MODE_GRAY)
{
    pBitmap->pitch = pitch;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(8, FT_TS_PIXEL_MODE_GRAY)
{
    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(8, FT_TS_PIXEL_MODE_GRAY)(
        pBitmapSrc, pBufferSrc, 1,
        pBitmapDst, pBufferDst, 1);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(8, FT_TS_PIXEL_MODE_GRAY)
{
    memcpy(*pBufferDst, *pBufferSrc, pBitmapSrc->pitch);

    (*pBufferDst) = (*pBufferDst) + (aheadDst > 0 ? pBitmapDst->pitch : -pBitmapDst->pitch);
    (*pBufferSrc) = (*pBufferSrc) + (aheadSrc > 0 ? pBitmapSrc->pitch : -pBitmapSrc->pitch);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(8, FT_TS_PIXEL_MODE_GRAY)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(8, FT_TS_PIXEL_MODE_GRAY)
{
    pBuffer += pBitmap->pitch;
    return pBuffer;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(8, FT_TS_PIXEL_MODE_GRAY)
{
    return pSrc[xSrc];
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(8, FT_TS_PIXEL_MODE_GRAY)
{
    pSrc[xSrc] = value;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(8, FT_TS_PIXEL_MODE_GRAY)
{
    int pitchSrc = pBitmapSrc->pitch;
    int pitchDst = pBitmapDst->pitch;
    UCHAR value = pSrc[ySrc*pitchSrc + xSrc];
    if (value)
    {
        pDst[yDst*pitchDst + xDst] = value;
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(8, FT_TS_PIXEL_MODE_GRAY)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(8, FT_TS_PIXEL_MODE_GRAY)
{
    if ((pSrc[x/8] & (FT_TS_BITMAP_PIXEL_MASK >> (x % 8 ))) != 0)
    {
         pDst[x] = FT_TS_PIXEL_GRAY_BLACK;
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(8, FT_TS_PIXEL_MODE_GRAY)
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

DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(8, FT_TS_PIXEL_MODE_GRAY)
{
    pBitmap->pitch = pBitmap->width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(8, FT_TS_PIXEL_MODE_GRAY)
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
