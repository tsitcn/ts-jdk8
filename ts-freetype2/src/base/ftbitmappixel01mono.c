/****************************************************************************
 *
 * ftbitmapbit01.c
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

#define FT_TS_MONO_BLACK      0x01

DECLARE_FT_TS_BITMAP_PIXEL_INIT(1, FT_TS_PIXEL_MODE_MONO);

DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(1, FT_TS_PIXEL_MODE_MONO)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(1, FT_TS_PIXEL_MODE_MONO)
{
    return pBitmap->width;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(1, FT_TS_PIXEL_MODE_MONO)
{
    pBitmap->width = width;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(1, FT_TS_PIXEL_MODE_MONO)
{
    return pBitmap->rows;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(1, FT_TS_PIXEL_MODE_MONO)
{
    pBitmap->rows = rows;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(1, FT_TS_PIXEL_MODE_MONO)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(1, FT_TS_PIXEL_MODE_MONO)
{
    pBitmap->pitch = pitch;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(1, FT_TS_PIXEL_MODE_MONO)
{
    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(1, FT_TS_PIXEL_MODE_MONO)(
        pBitmapSrc, pBufferSrc, 1,
        pBitmapDst, pBufferDst, 1);

    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(1, FT_TS_PIXEL_MODE_MONO)
{
    memcpy(*pBufferDst, *pBufferSrc, pBitmapSrc->pitch);

    (*pBufferDst) = (*pBufferDst) + (aheadDst > 0 ? pBitmapDst->pitch : -pBitmapDst->pitch);
    (*pBufferSrc) = (*pBufferSrc) + (aheadSrc > 0 ? pBitmapSrc->pitch : -pBitmapSrc->pitch);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(1, FT_TS_PIXEL_MODE_MONO)
{
    return pBitmap->pitch;
}

DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(1, FT_TS_PIXEL_MODE_MONO)
{
    pBuffer += pBitmap->pitch;
    return pBuffer;
}

DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(1, FT_TS_PIXEL_MODE_MONO)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(1, FT_TS_PIXEL_MODE_MONO)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(1, FT_TS_PIXEL_MODE_MONO)
{
    int pitchSrc = pBitmapSrc->pitch;
    int pitchDst = pBitmapDst->pitch;
    int srcPosByte;
    int srcPosBit;
    int srcBitValue;

    srcPosByte  = ySrc * pitchSrc + xSrc / 8;
    srcPosBit   = xSrc % 8;

    srcBitValue = (pSrc[srcPosByte] & (FT_TS_BITMAP_PIXEL_MASK >> srcPosBit)) != 0 ? 1 : 0;
    if (srcBitValue)
    {
        int dstPosByte    = yDst * pitchDst + xDst / 8;
        int dstPosBit     = xDst % 8;
        pDst[dstPosByte] |= (FT_TS_BITMAP_PIXEL_MASK >> dstPosBit);
    }
    return 0;
}

  /**
   * Swap a bit.
   */
  static void FT_TS_Bitmap_Swap_SrcToDst_Position_Bit(
      UCHAR* pSrc, int srcPosByte, int srcPosBit,
      UCHAR* pDst, int dstPosByte, int dstPosBit)
  {
      int srcBitValue;
      int dstBitValue;

      srcBitValue = pSrc[srcPosByte] & (FT_TS_BITMAP_PIXEL_MASK >> srcPosBit);
      dstBitValue = pDst[dstPosByte] & (FT_TS_BITMAP_PIXEL_MASK >> dstPosBit);

      if (srcBitValue == dstBitValue)
      {
          return;
      }

      if (srcBitValue)
      {
          pDst[dstPosByte] |=   FT_TS_BITMAP_PIXEL_MASK >> dstPosBit;
      }
      else
      {
          pDst[dstPosByte] &= ~(FT_TS_BITMAP_PIXEL_MASK >> dstPosBit);
      }
      if (dstBitValue)
      {
          pSrc[srcPosByte] |=   FT_TS_BITMAP_PIXEL_MASK >> srcPosBit;
      }
      else
      {
          pSrc[srcPosByte] &= ~(FT_TS_BITMAP_PIXEL_MASK >> srcPosBit);
      }
  }

DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(1, FT_TS_PIXEL_MODE_MONO)
{
    int pitchSrc = pBitmapSrc->pitch;
    int pitchDst = pBitmapDst->pitch;
    int srcPosByte;
    int srcPosBit;
    int dstPosByte;
    int dstPosBit;

    srcPosByte = ySrc *pitchSrc  + xSrc  / 8;
    srcPosBit  = xSrc % 8;

    dstPosByte = yDst*pitchDst + xDst / 8;
    dstPosBit  = xDst%8;

    FT_TS_Bitmap_Swap_SrcToDst_Position_Bit(
          pSrc, srcPosByte, srcPosBit,
          pDst, dstPosByte, dstPosBit);
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(1, FT_TS_PIXEL_MODE_MONO)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(1, FT_TS_PIXEL_MODE_MONO)
{
    if ((pData[x/8] & (FT_TS_BITMAP_PIXEL_MASK >> (x % 8 ))) != 0)
    {
        pBuffer[pos] = '1';
    }
    else
    {
        pBuffer[pos] = ' ';
    }
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(1, FT_TS_PIXEL_MODE_MONO)
{
    pBitmap->pitch = (pBitmap->width + 7)/8;
    return 0;
}

DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(1, FT_TS_PIXEL_MODE_MONO)
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
