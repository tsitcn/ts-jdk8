/****************************************************************************
 *
 * ftbitmapext.c
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


#include <freetype/internal/ftdebug.h>

#include <freetype/ftbitmap.h>
#include <freetype/ftbitmapext.h>
#include <freetype/ftbitmaprotate.h>
#include <freetype/ftbitmappixel.h>

#include <freetype/ftimage.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftsynth.h>


/**
 TSIT {{{{{{{{{{
 */

#define FT_TS_BITMAP_DUMP_STRUCTURE(pBitmap)          {FT_TS_Bitmap_Pixel_MN pPixelFuncs4Dump = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap); \
                                                       DEBUG_FORMAT("bitmap=(0x%02X, 0x%02X, %2d, %2d, %2d)", \
                                                            pBitmap->num_grays, pBitmap->pixel_mode, \
                                                            pPixelFuncs4Dump->get_width(pBitmap),    \
                                                            pPixelFuncs4Dump->get_rows( pBitmap),    \
                                                            pPixelFuncs4Dump->get_pitch(pBitmap); }
#define FT_TS_BITMAP_DUMP_SLOT(slot)                  DEBUG_FORMAT("slot=(%2d, %2d)", slot->bitmap_left, slot->bitmap_top)
#define FT_TS_BITMAP_DUMP(pBitmap)                    FT_TS_Bitmap_Dump(pBitmap, __FUNC__, __LINE__)


/* #define FT_TS_FONT_BITMAP_TO_FILE */


/* static const FT_TS_Bitmap  null_bitmap_ext = { 0, 0, 0, NULL, 0, 0, 0, NULL }; */



  static void FT_TS_Bitmap_Dump(FT_TS_Bitmap*  pBitmap, char* pFunc, int lineNo)
  {
      UCHAR  buffer[8192] = {0};
      int    pos = 0;

      UINT   x, y;
      UCHAR* pData = pBitmap->buffer;
      UINT   width;
      UINT   rows;

      FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      width = pPixelFuncs->get_width(pBitmap);
      rows  = pPixelFuncs->get_rows (pBitmap);
      DEBUG_FORMAT("(%s,%d)width=%d, rows=%d, pitch=%d", pFunc, lineNo, width, rows,
          pPixelFuncs->get_pitch(pBitmap));

      buffer[pos++] = '\n';

      for ( y = 0; y < rows; y++ )
      {
          for (x = 0; x < width; x++)
          {
              pPixelFuncs->dump_data(pBitmap, pData, x, buffer, pos);
              pos ++;
          }
          pData = pPixelFuncs->move_row(pBitmap, pData);
          buffer[pos++] = '\n';
      }

      DEBUG_TEXT(buffer);
  }

  static FT_TS_Bitmap_Pixel_MN* FT_TS_Bitmap_Get_PixelFuncsByLoadFlags(const int flags, const int gray)
  {
    if (flags & FT_TS_LOAD_TARGET_LCD)
    {
        return NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD);
    }
    if (flags & FT_TS_LOAD_TARGET_LCD_V)
    {
        return NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD_V);
    }
    if (gray)
    {
        return NAME_FT_TS_BITMAP_PIXEL_INIT( 8, FT_TS_PIXEL_MODE_GRAY);
    }
    return NAME_FT_TS_BITMAP_PIXEL_INIT( 1, FT_TS_PIXEL_MODE_MONO);
  }

  FT_TS_Bitmap_Pixel_MN* FT_TS_Bitmap_Get_PixelFuncsByMode(const int pixel_mode)
  {
      switch (pixel_mode)
      {
          case FT_TS_PIXEL_MODE_GRAY:
              return NAME_FT_TS_BITMAP_PIXEL_INIT(8, FT_TS_PIXEL_MODE_GRAY);

          case FT_TS_PIXEL_MODE_LCD:
              return NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD);

          case FT_TS_PIXEL_MODE_LCD_V:
              return NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD_V);

          case  FT_TS_PIXEL_MODE_MONO:
          default:
              return NAME_FT_TS_BITMAP_PIXEL_INIT( 1, FT_TS_PIXEL_MODE_MONO);
      }
  }

  FT_TS_Bitmap_Pixel_MN* FT_TS_Bitmap_Get_PixelFuncsByBitmap(const FT_TS_Bitmap*  pBitmap)
  {
      return FT_TS_Bitmap_Get_PixelFuncsByMode(pBitmap->pixel_mode);
  }

  static FT_TS_Bitmap_Rotate_90N* FT_TS_Bitmap_Get_RotateFuncs(const int degree)
  {
      switch (degree/90)
      {
          case 1:
              return NAME_FT_TS_BITMAP_ROTATE_INIT_90(1);

          case 2:
              return NAME_FT_TS_BITMAP_ROTATE_INIT_90(2);

          case 3:
              return NAME_FT_TS_BITMAP_ROTATE_INIT_90(3);

          case 0:
          default:
              return NAME_FT_TS_BITMAP_ROTATE_INIT_90(0);
      }
  }
  
  static int FT_TS_Bitmap_FloatToInt(float value)
  {
      if (value == 0)
      {
          return 0;
      }
      if (value > 0)
      {
          return (int)(value+0.5F);
      }
      return (int)(value-0.5F);
  }


  int FT_TS_Bitmap_Init_Buffer(FT_TS_Bitmap*  pBitmap)
  {
      FT_TS_Bitmap_Pixel_MN* pPixelFuncs;
      (FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap))->init_pitch(pBitmap);

      pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      pPixelFuncs->init_buffer(pBitmap);

      return 0;
  }

  void FT_TS_Bitmap_Change_Buffer(FT_TS_Bitmap* pDst, FT_TS_Bitmap* pSrc)
  {
      if (!pSrc->buffer)
      {
          return;
      }

      free(pDst->buffer);
      pDst->buffer = pSrc->buffer;

      /** make it clear */
      pSrc->buffer = NULL;
  }

  /**
   Flip {{{{{{{{{{
   */
  
  static void FT_TS_Bitmap_FlipL2R(FT_TS_Bitmap* pDstBitmap, const FT_TS_Bitmap* pSrcBitmap,
      FT_TS_GlyphSlot slot)
  {
      UCHAR*  pDst = pDstBitmap->buffer;
      UCHAR*  pSrc = pSrcBitmap->buffer;
      int    width;
      int     rows;
      int    x;
      int    y;
      FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pSrcBitmap);

      width = pPixelFuncs->get_width(pSrcBitmap);
      rows  = pPixelFuncs->get_rows (pSrcBitmap);

      for (y = 0; y < rows; y++)
      {
          for (x = 0; x < width; x++)
          {
              pPixelFuncs->copy_xy(
                  pSrcBitmap, pSrc,         x, y,
                  pDstBitmap, pDst, width-1-x, y);
          }
      }

      slot->bitmap_left = 0 - (width + slot->bitmap_left);
  }

  static void FT_TS_Bitmap_FlipT2B(FT_TS_Bitmap* pDstBitmap, const FT_TS_Bitmap* pSrcBitmap,
      FT_TS_GlyphSlot slot)
  {
      UCHAR*  pDst = pDstBitmap->buffer;
      UCHAR*  pSrc = pSrcBitmap->buffer;
      int     rows;
      int     pitch_full;
      int     y;

      FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pSrcBitmap);

      rows       = pPixelFuncs->get_rows (pSrcBitmap);
      pitch_full = pPixelFuncs->get_pitch_full(pSrcBitmap);

      pDst += pitch_full*(rows-1);
      for (y = 0; y < rows; y++)
      {
          pPixelFuncs->copy_row2(
              pSrcBitmap, (&pSrc),  1,
              pDstBitmap, (&pDst), -1);
      }

      if (rows >= slot->bitmap_top)
      {
          slot->bitmap_top = -slot->bitmap_top + rows;
      }
      else
      {
          slot->bitmap_top = -slot->bitmap_top;
      }
  }

  /**
   * If have L2R/T2B same time, L2R first, then T2B.
   */
  static UCHAR* FT_TS_Bitmap_Flip(FT_TS_Bitmap* pBitmap, FT_TS_GlyphSlot slot, int flags)
  {
      FT_TS_Bitmap oFliped;
      FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      int size;

      int width;
      int rows;

      if (   !FT_TS_CHECK_FLIP_L2R(flags)
          && !FT_TS_CHECK_FLIP_T2B(flags))
      {
          return pBitmap->buffer;
      }


      width = pPixelFuncs->get_width(pBitmap);
      rows  = pPixelFuncs->get_rows (pBitmap);

#ifdef FT_TS_FONT_BITMAP_TO_FILE
      FT_TS_BITMAP_DUMP(pBitmap);
#endif

      memcpy(&oFliped, pBitmap, sizeof(FT_TS_Bitmap));
      oFliped.buffer = NULL;
      pPixelFuncs->init_buffer((&oFliped));

      if (FT_TS_CHECK_FLIP_L2R(flags))
      {
          memset(oFliped.buffer, 0, size);

          FT_TS_Bitmap_FlipL2R(&oFliped, pBitmap, slot);

          memcpy(pBitmap->buffer, oFliped.buffer, size);
      }

      if (FT_TS_CHECK_FLIP_T2B(flags))
      {
          memset(oFliped.buffer, 0, size);

          FT_TS_Bitmap_FlipT2B(&oFliped, pBitmap, slot);
      }

      FT_TS_Bitmap_Change_Buffer(pBitmap, &oFliped);

#ifdef FT_TS_FONT_BITMAP_TO_FILE
      FT_TS_BITMAP_DUMP(pBitmap);
#endif

      return pBitmap->buffer;
  }

  /**
   Fip }}}}}}}}}}
   */

  /**
   Rotate {{{{{{{{{{
   */

  static void FT_TS_Bitmap_Rotate_Degree(FT_TS_Bitmap* pDstBitmap, const FT_TS_Bitmap* pSrcBitmap,
      const FT_TS_Bitmap_Rotate_90N* pFuncs, const FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
  {
      UCHAR*   pDst = pDstBitmap->buffer;
      UCHAR*   pSrc = pSrcBitmap->buffer;
      int src_width;
      int src_rows;

      int xSrc;
      int ySrc;
      int dstCoords[2] = {0};

      src_width = pPixelFuncs->get_width(pSrcBitmap);
      src_rows  = pPixelFuncs->get_rows (pSrcBitmap);

      for (ySrc=0; ySrc<src_rows; ySrc++)
      {
          for (xSrc=0; xSrc<src_width; xSrc++)
          {
              pFuncs->rotate_dst_coords(dstCoords, xSrc, ySrc, src_width, src_rows, pPixelFuncs);

              pPixelFuncs->copy_xy(
                    pSrcBitmap, pSrc, xSrc,         ySrc,
                    pDstBitmap, pDst, dstCoords[0], dstCoords[1]);
        }
    }
  }

  static UCHAR* FT_TS_Bitmap_Rotate(FT_TS_Bitmap* pBitmap, int degree, FT_TS_GlyphSlot slot, const int flags)
  {
      int size;
      FT_TS_Bitmap oRotated = {0};
      FT_TS_Bitmap_Pixel_MN*   pPixelFuncs;
      FT_TS_Bitmap_Rotate_90N* pRotateFuncs;
      if (degree == 0)
      {
          return pBitmap->buffer;
      }

      pPixelFuncs  = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      pRotateFuncs = FT_TS_Bitmap_Get_RotateFuncs(degree);
      pRotateFuncs->init_4_rotate(&oRotated, pBitmap, pPixelFuncs);

#ifdef FT_TS_FONT_BITMAP_TO_FILE
      FT_TS_BITMAP_DUMP(pBitmap);
#endif

      pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      pPixelFuncs->init_buffer((&oRotated));

      FT_TS_Bitmap_Rotate_Degree(&oRotated, pBitmap, pRotateFuncs, pPixelFuncs);

      if (slot != NULL)
      {
          pRotateFuncs->rotate_slot_position(slot, pBitmap, flags, pPixelFuncs);
      }

      pPixelFuncs->set_width(pBitmap, pPixelFuncs->get_width((&oRotated)));
      pPixelFuncs->set_rows( pBitmap, pPixelFuncs->get_rows( (&oRotated)));
      pPixelFuncs->set_pitch(pBitmap, pPixelFuncs->get_pitch((&oRotated)));

      FT_TS_Bitmap_Change_Buffer(pBitmap, &oRotated);

#ifdef FT_TS_FONT_BITMAP_TO_FILE
      FT_TS_BITMAP_DUMP(pBitmap);
#endif

      return pBitmap->buffer;
  }

  /**
   Rotate }}}}}}}}}}
   */

  /**
   Italic {{{{{{{{{{
   */

static int FT_TS_Bitmap_Italic_Hor_Init(FT_TS_Bitmap* pDst,
                                        const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot,
                                        FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
{
    int size;
    int rows;
    int italic;

    size = slot->bitmap_top >= 0 ? slot->bitmap_top : (-slot->bitmap_top);
    rows = pPixelFuncs->get_rows(pSrc);
    if (rows > size)
    {
        size = rows;
    }
    italic      = FT_TS_Bitmap_FloatToInt(size*oblique);

    pPixelFuncs->set_width(pDst, pPixelFuncs->get_width(pSrc) + italic);
    pPixelFuncs->set_rows( pDst, pPixelFuncs->get_rows(pSrc));

    FT_TS_Bitmap_Init_Buffer(pDst);

    return italic;
}

static int FT_TS_Bitmap_Italic_Hor_SlotOffset(const int degree, const int to_bottom,
                                              const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot,
                                              FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
{
    int size;
    int rows;
    int slotOffset;

    if (degree == 90 && !to_bottom || degree == 180)
    {
        size = -slot->bitmap_top;
    }
    else
    {
        size =  slot->bitmap_top;
    }

    rows = pPixelFuncs->get_rows(pSrc);
    if (rows > size)
    {
        int below  = rows - slot->bitmap_top - 1;
        slotOffset = FT_TS_Bitmap_FloatToInt(below*oblique);
    }
    else if (degree == 180)
    {
        slotOffset = FT_TS_Bitmap_FloatToInt(size*oblique);
    }
    else
    {
        slotOffset = 0;
    }

    return slotOffset;
}

/**
 经过深入研究，目前的斜体方案是这样的。
 以基线为准，使用四舍五入法(暂定。可以肯定不是收尾法）。
 1、基线之上，向右倾斜。
 2、基线本身，不变。
 3、基线之下，向左倾斜。倾斜几个像素，left要做相应调整。

 为了方便，这里特别挑选几个有代表性的字。

 中，width=11, rows=16, left= 2, top=14
 竖向： 基线是0。基线之上有14行，基线之下有 1行。
        三块大小：14,  1,  1
        数据分布：14,  1,  1
 横向： 基线是0。基线之左有 1列，基线之右有 9列。
        三块大小： 9,  1,  0
        数据分布： 9,  1,  0

 一，width=15, rows= 1, left= 0, top= 7
 竖向： 基线是0。基线之上有 7行，基线之下有 0行。
        三块大小： 7,  0,  0
        数据分布： 1,  0,  0
 横向： 基线是0。基线之左有 0列，基线之右有14列。
        三块大小：14,  1,  0
        数据分布：14,  1,  0
 补充说明：此处的7，从基线0开始的第7行。也就是说，在第7行上绘制。

 丨，width= 1, rows=16, left= 7, top=14
 竖向： 基线是0。基线之上有14行，基线之下有 1行。
        三块大小：14,  1,  1
        数据分布：14,  1,  1
 横向： 基线是0。基线之左有 0列，基线之右有 7列。
        三块大小： 7,  0,  0
        数据分布： 1,  1,  0
 补充说明：此处的7，从基线0开始的第7行。 也就是说，在第7列上绘制。

 */
int FT_TS_Bitmap_Italic_Hor(FT_TS_Bitmap* pBitmapDst, const int degree, const int to_bottom, float oblique,
    FT_TS_Bitmap* pBitmapSrc, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)
{
    int isCCW;
    int y;
    int x;
    int move;

    int slotOffset;
    int totalItalic;

    /** 感觉上，定义变量比->访问性能好一点 */
    int     widthSrc;
    int      rowsSrc;
    UCHAR* bufferSrc = pBitmapSrc->buffer;
    int       topSrc = slot->bitmap_top;

    UCHAR* bufferDst;

    widthSrc = pPixelFuncs->get_width(pBitmapSrc);
    rowsSrc  = pPixelFuncs->get_rows( pBitmapSrc);

    if (oblique >= 0)
    {
        isCCW   = 0;
    }
    else
    {
        isCCW   = 1;
        oblique = -oblique;
    }

    totalItalic = FT_TS_Bitmap_Italic_Hor_Init(pBitmapDst, oblique, pBitmapSrc, slot, pPixelFuncs);
    slotOffset  = FT_TS_Bitmap_Italic_Hor_SlotOffset(degree, to_bottom, oblique, pBitmapSrc, slot, pPixelFuncs);
    bufferDst   = pBitmapDst->buffer;
    for ( y = 0; y < rowsSrc; y++ )
    {
        move  = FT_TS_Bitmap_FloatToInt((topSrc - y)*oblique) + slotOffset;
        if (isCCW)
        {
            move = totalItalic-move;
        }
        for (x=0; x<widthSrc; x++)
        {
            pPixelFuncs->copy_xy(
                pBitmapSrc, bufferSrc, x,      y,
                pBitmapDst, bufferDst, x+move, y);
        }
    }

    //调整插槽位置。
    if (isCCW)
    {
        if (slotOffset > 0)
        {
            slotOffset = totalItalic - slotOffset;
        }
        else
        {
            slotOffset = totalItalic + slotOffset;
        }
    }
    if (slotOffset != 0)
    {
        slot->bitmap_left -= slotOffset;
    }

    return 0;
}

static int FT_TS_Bitmap_Italic_Ver_Init(FT_TS_Bitmap* pDst,
                                        const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot,
                                        FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
{
    int size;
    /** pSrc->width > size = false ? why */
    int width;
    int italic;

    size  = slot->bitmap_left >= 0 ? slot->bitmap_left : (-slot->bitmap_left);
    width = pPixelFuncs->get_width(pSrc);
    if (width > size)
    {
        size = width;
    }
    italic      = FT_TS_Bitmap_FloatToInt(size*oblique);
    
    pPixelFuncs->set_width(pDst, pPixelFuncs->get_width(pSrc));
    pPixelFuncs->set_rows( pDst, pPixelFuncs->get_rows(pSrc)+ italic);

    FT_TS_Bitmap_Init_Buffer(pDst);

    return italic;
}

static int FT_TS_Bitmap_Italic_Ver_SlotOffset(const int to_bottom, const int degree,
                                              const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot,
                                              FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
{
    int size;
    /** pSrc->width > size = false ? why */
    int width;
    int slotOffset;

    if (degree == 180 || degree == 270)
    {
        size = -slot->bitmap_left;
    }
    else
    {
        size =  slot->bitmap_left;
    }
    width = pPixelFuncs->get_width(pSrc);

    if (width > size)
    {
        size = width;
    }

    if (   degree ==  90
        || degree == 270
        || degree ==   0 && to_bottom
        || degree == 180 && to_bottom)
    {
        slotOffset = FT_TS_Bitmap_FloatToInt(slot->bitmap_left* oblique);
    }
    else if (width > slot->bitmap_left)
    {
        slotOffset = FT_TS_Bitmap_FloatToInt(size* oblique);
    }
    else
    {
        slotOffset = 0;
    }

    return slotOffset;
}

/**
90:
 中，width= 16, rows= 11, left=- 2, top=- 2
 一，width=  1, rows= 15, left=  6, top=  0
 丨，width= 16, rows=  1, left=- 2, top=- 7

 270:
 中，width= 16, rows= 11, left=-14, top= 13
 一，width=  1, rows= 15, left=- 7, top= 15
 丨，width= 16, rows=  1, left=-14, top=  8

 */
int FT_TS_Bitmap_Italic_Ver(FT_TS_Bitmap* pDstBitmap, const int degree, const int to_bottom, float oblique,
    FT_TS_Bitmap* pSrcBitmap, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)
{
    int isCCW;
    int y;
    int x;
    int move;
    int slotOffset;
    int totalItalic;

    int     widthSrc;
    int      rowsSrc;
    UCHAR* bufferSrc = pSrcBitmap->buffer;
    int     slotLeft = slot->bitmap_left;

    UCHAR* bufferDst;

    widthSrc = pPixelFuncs->get_width(pSrcBitmap);
    rowsSrc  = pPixelFuncs->get_rows (pSrcBitmap);

    if (oblique >= 0)
    {
        isCCW   = 0;
    }
    else
    {
        isCCW   = 1;
        oblique = -oblique;
    }

    totalItalic = FT_TS_Bitmap_Italic_Ver_Init(pDstBitmap, oblique, pSrcBitmap, slot, pPixelFuncs);
    slotOffset  = FT_TS_Bitmap_Italic_Ver_SlotOffset(to_bottom, degree, oblique, pSrcBitmap, slot, pPixelFuncs);
    bufferDst   = pDstBitmap->buffer;

    for ( x = 0; x < widthSrc; x++ )
    {
        move  = FT_TS_Bitmap_FloatToInt((x+slotLeft)*oblique) - slotOffset;
        if (isCCW)
        {
            move = totalItalic-move;
        }

        for (y=0; y<rowsSrc; y++)
        {
            pPixelFuncs->copy_xy(
                pSrcBitmap, bufferSrc, x, y,
                pDstBitmap, bufferDst, x, y+move);
        }
    }

    //调整插槽位置。
    if (isCCW)
    {
        if (   degree ==  90 && !to_bottom
            || degree ==   0 &&  to_bottom
            || degree == 270)
        {
            slotOffset = -(totalItalic + slotOffset);
        }
        else if (slotOffset > 0)
        {
            slotOffset =   totalItalic - slotOffset;
        }
        else
        {
            slotOffset =   totalItalic + slotOffset;
        }
    }

    if (slotOffset != 0)
    {
        slot->bitmap_top -= slotOffset;
    }

    return 0;
}

  FT_TS_EXPORT_DEF(    FT_TS_Error )
  FT_TS_Bitmap_Italic( FT_TS_GlyphSlot  slot)
  {
      return FT_TS_Bitmap_Italic_Direction(slot,
                 FT_TS_FONT_ITALIC_VALUE,
                 FT_TS_POSTURE_TO_RIGHT,
                 0);
  }

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Italic_Direction(FT_TS_GlyphSlot  slot,
                    float       oblique,
                    int         to_bottom,
                    int         degree)
  {
    FT_TS_Error        error;
    FT_TS_Library      library =   slot->library;
    FT_TS_Bitmap*      bitmap  = &(slot->bitmap);
    FT_TS_Bitmap       oItalic = {0};
    FT_TS_Bitmap_Rotate_90N*  pRotateFuncs;
    FT_TS_Bitmap_Pixel_MN*    pPixelFuncs;

    if (!slot || !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !bitmap || !bitmap->buffer )
      return FT_TS_THROW( Invalid_Argument );

    if (oblique == 0)
    {
        return FT_TS_Err_Ok;
    }

    switch ( bitmap->pixel_mode )
    {
    case FT_TS_PIXEL_MODE_GRAY2:
    case FT_TS_PIXEL_MODE_GRAY4:
      {
        FT_TS_Bitmap  tmp;

        /* convert to 8bpp */
        FT_TS_Bitmap_Init( &tmp );
        error = FT_TS_Bitmap_Convert( library, bitmap, &tmp, 1 );
        if ( error )
          return error;

        FT_TS_Bitmap_Done( library, bitmap );
        *bitmap = tmp;
      }
      break;

    case FT_TS_PIXEL_MODE_MONO:
    case FT_TS_PIXEL_MODE_LCD:
    case FT_TS_PIXEL_MODE_LCD_V:
      break;

    case FT_TS_PIXEL_MODE_BGRA:
      /* We don't embolden color glyphs. */
      return FT_TS_Err_Ok;
    }

#ifdef  FONT_BITMAP_TO_FILE
    FT_TS_Bitmap_Dump(bitmap);
#endif

    pPixelFuncs  = FT_TS_Bitmap_Get_PixelFuncsByBitmap(bitmap);
    pRotateFuncs = FT_TS_Bitmap_Get_RotateFuncs(degree);

    memcpy(&oItalic, bitmap, sizeof(FT_TS_Bitmap));
    pRotateFuncs->italic_action(&oItalic, to_bottom, degree, oblique, bitmap, slot, pPixelFuncs);

    if (oItalic.buffer != NULL)
    {
        pPixelFuncs->set_width(bitmap, pPixelFuncs->get_width((&oItalic)));
        pPixelFuncs->set_rows( bitmap, pPixelFuncs->get_rows( (&oItalic)));
        pPixelFuncs->set_pitch(bitmap, pPixelFuncs->get_pitch((&oItalic)));

        FT_TS_Bitmap_Change_Buffer(bitmap, &oItalic);
    }

#ifdef FONT_BITMAP_TO_FILE
    FT_TS_Bitmap_Dump(bitmap);
#endif

    return FT_TS_Err_Ok;
  }

  /**
   Italic }}}}}}}}}}
   */


  /**
   Bold and Weight {{{{{{{{{{
   */

  static void FT_TS_Bitmap_Create_Buffer_Bit2Byte(FT_TS_Bitmap* pBitmap,
      FT_TS_Int32     load_flags)
  {
      UINT   x, y;
      UCHAR* pSrcData;
      UCHAR* pDstData;
      UCHAR  pixelModeSrc;
      int    widthSrc;
      int    rowsSrc;
      FT_TS_Bitmap oTemp = {0};
      FT_TS_Bitmap_Pixel_MN* pPixelFuncsSrc;
      FT_TS_Bitmap_Pixel_MN* pPixelFuncsDst;

      pixelModeSrc = pBitmap->pixel_mode;
      pPixelFuncsSrc = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);

      pPixelFuncsDst = FT_TS_Bitmap_Get_PixelFuncsByLoadFlags(load_flags, 1);
      pPixelFuncsDst->init_with_bitmap((&oTemp), pBitmap);
      
      widthSrc = pPixelFuncsSrc->get_width(pBitmap);
      rowsSrc  = pPixelFuncsSrc->get_rows( pBitmap);

      pSrcData = pBitmap->buffer;
      pDstData =    oTemp.buffer;
      for ( y = 0; y < rowsSrc; y++ )
      {
          for (x = 0; x < widthSrc; x++)
          {
              pPixelFuncsDst->convert_from(pixelModeSrc, pSrcData, x, (&oTemp), pDstData);
          }
          pSrcData = pPixelFuncsSrc->move_row(pBitmap,  pSrcData);
          pDstData = pPixelFuncsDst->move_row((&oTemp), pDstData);
      }
      pPixelFuncsDst->set_width(pBitmap, pPixelFuncsDst->get_width((&oTemp)));
      pPixelFuncsDst->set_rows (pBitmap, pPixelFuncsDst->get_rows ((&oTemp)));
      pPixelFuncsDst->set_pitch(pBitmap, pPixelFuncsDst->get_pitch((&oTemp)));
      pBitmap->pixel_mode = oTemp.pixel_mode;
      pBitmap->num_grays  = oTemp.num_grays;
      
      FT_TS_Bitmap_Change_Buffer(pBitmap, &oTemp);
  }


  /**
   * Create a new buffer with weight.
   * If embolden, the width/rows  size is larger.
   * If slim, the width/rows  size isn't larger.
   *
   * 此时
   * 注意：
   * 在处理缓冲区的时候，pBitmap的width/rows是不变的。
   * 否则图像内容会处理错误。
   *
   * 粗体完成之后，pBitmap的width/rows需要改变。
   */
  static void FT_TS_Bitmap_Create_BufferWeight(FT_TS_Bitmap* pBitmap, int xstr, int ystr)
  {
      UCHAR* pSrcData;
      UCHAR* pDstData;
      FT_TS_Bitmap oTemp = {0};
      FT_TS_Bitmap_Pixel_MN* pPixelFuncs;

      memcpy(&oTemp, pBitmap, sizeof(FT_TS_Bitmap));

      pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
      if (xstr > 0)
      {
          pPixelFuncs->set_width((&oTemp), pPixelFuncs->get_width((&oTemp)) + xstr);
      }
      if (ystr > 0)
      {
          pPixelFuncs->set_rows ((&oTemp), pPixelFuncs->get_rows ((&oTemp)) + ystr);
      }

      if (   pPixelFuncs->get_width((&oTemp)) != pPixelFuncs->get_width(pBitmap)
          || pPixelFuncs->get_rows ((&oTemp)) != pPixelFuncs->get_rows (pBitmap) )
      {
          UCHAR* pSrcData;
          UCHAR* pDstData;
          UINT   i;
          int rows = pPixelFuncs->get_rows ((&oTemp));

          FT_TS_Bitmap_Init_Buffer(&oTemp);

          pSrcData = pBitmap->buffer;
          pDstData =    oTemp.buffer;
          for (i=0; i<rows; i++)
          {
              pPixelFuncs->copy_row(
                  pBitmap,  (&pSrcData),
                  (&oTemp), (&pDstData));
          }

          pPixelFuncs->set_width(pBitmap, pPixelFuncs->get_width(&oTemp));
          pPixelFuncs->set_rows (pBitmap, pPixelFuncs->get_rows (&oTemp));
          pPixelFuncs->set_pitch(pBitmap, pPixelFuncs->get_pitch(&oTemp));
          FT_TS_Bitmap_Change_Buffer(pBitmap, &oTemp);
      }
  }

static int
FT_TS_Bitmap_Bold_Action_Default( FT_TS_Bitmap*  pBitmap,
                      FT_TS_Pos      xstr,
                      FT_TS_Pos      ystr )
{
    UCHAR*     p;
    FT_TS_Int  i, x, pitchDst;
    FT_TS_UInt y;
    int        rows;
    int        pitchSrc;
    FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
    
    rows     = pPixelFuncs->get_rows (pBitmap);
    pitchDst = pPixelFuncs->get_pitch(pBitmap);
    p        = pBitmap->buffer;
    /* for each row */
    for ( y = 0; y < rows; y++ )
    {
      /*
       * Horizontally:
       *
       * From the last pixel on, make each pixel or'ed with the
       * `xstr' pixels before it.
       */
      for ( x = pitchDst - 1; x >= 0; x-- )
      {
        UCHAR  tmp;

        tmp = p[x];
        for ( i = 1; i <= xstr; i++ )
        {
          if ( pBitmap->pixel_mode == FT_TS_PIXEL_MODE_MONO )
          {
            p[x] |= tmp >> i;

            /* the maximum value of 8 for `xstr' comes from here */
            if ( x > 0 )
              p[x] |= p[x - 1] << ( 8 - i );

#if 0
            if ( p[x] == 0xFF )
              break;
#endif
          }
          else
          {
            if ( x - i >= 0 )
            {
              if ( p[x] + p[x - i] > pBitmap->num_grays - 1 )
              {
                p[x] = (UCHAR)( pBitmap->num_grays - 1 );
                break;
              }
              else
              {
                p[x] = (UCHAR)( p[x] + p[x - i] );
                if ( p[x] == pBitmap->num_grays - 1 )
                  break;
              }
            }
            else
              break;
          }
        }
      }

      /*
       * Vertically:
       *
       * Make the above `ystr' rows or'ed with it.
       */
      pitchSrc = pPixelFuncs->get_pitch(pBitmap);
      for ( x = 1; x <= ystr; x++ )
      {
        UCHAR*  q;

        q = p - pitchSrc * x;
        for ( i = 0; i < pitchDst; i++ )
          q[i] |= p[i];
      }

      p += pitchSrc;
    }

    return 0;
}

/**
 注意，此时：
 pCurrent[x-1] = 0xFF
 pCurrent[x  ] = 0x00
 */
static void FT_TS_Bitmap_Bold_VerLine(FT_TS_Bitmap*  pBitmap, int width,
      UCHAR* pCurrent, FT_TS_Int  x, FT_TS_Int  y, int pixel_black,
      FT_TS_Bitmap_Pixel_MN* pPixelFuncs)
{
    UCHAR* pMove;
    UCHAR fill;
    int y1;
    int rows     = pPixelFuncs->get_rows(pBitmap);
    int height   = 0;
    int height2  = 0;
    int blank    = 0;

    pMove = pCurrent;
    for (y1=y; y1<rows; y1++)
    {
        if (   pPixelFuncs->get_value_x(pMove, x-1) != pixel_black
            || pPixelFuncs->get_value_x(pMove, x  ) == pixel_black
            || pPixelFuncs->get_value_x(pMove, x  ) != 0x00)
        {
            break;
        }

        blank  ++;
        height ++;
        if (x+1 < width)
        {
            if (pPixelFuncs->get_value_x(pMove, x+1) == 0x00)
            {
                blank ++;
            }
            else if (pPixelFuncs->get_value_x(pMove, x+1) == pixel_black)
            {
                height2 ++;
            }
        }

        pMove = pPixelFuncs->move_row(pBitmap, pMove);
    }

    if (x == (width-1) || blank == height*2)
    {
        fill = FT_TS_BITMAP_PIXEL_GRAY_DARK;
    }
    else if (height2 > 1 && height == height2+1)
    {
        fill = FT_TS_BITMAP_PIXEL_GRAY_LIGHT+FT_TS_BITMAP_PIXEL_GRAY_HEAVY/height;
    }
    else if (blank != height)
    {
        fill = FT_TS_BITMAP_PIXEL_GRAY_HEAVY;
    }
    else
    {
        fill = FT_TS_BITMAP_PIXEL_GRAY_LIGHT;
    }

    pMove = pCurrent;
    for (y1=0; y1<height; y1++)
    {
        pPixelFuncs->set_value_x(pBitmap, pMove, x, fill);
        pMove = pPixelFuncs->move_row(pBitmap, pMove);
    }
}

/**
 In fact, for bitmap, always:
 xstr=1
 ystr=0
 */
static int
FT_TS_Bitmap_Bold_Action_Gray( FT_TS_Bitmap*  pBitmap,
                      FT_TS_Pos      xstr,
                      FT_TS_Pos      ystr )
{
    UCHAR*     pCurrent;
    FT_TS_Int  x;
    FT_TS_UInt y;
    int        width;
    int        rows;
    int        pixel_black;
    FT_TS_Bitmap_Pixel_MN* pPixelFuncs;

    pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(pBitmap);
    width       = pPixelFuncs->get_width(pBitmap);
    rows        = pPixelFuncs->get_rows (pBitmap);

    pixel_black = FT_TS_BITMAP_PIXEL_BLACK;

    pCurrent = pBitmap->buffer;
    for ( y = 0; y < rows; y++ )
    {
        /* 1, x=0时，显然不需要处理。 */
        for ( x = width-1; x > 0; x-- )
        {
            if (   pPixelFuncs->get_value_x(pCurrent, x  ) == 0
                && pPixelFuncs->get_value_x(pCurrent, x-1) == pixel_black)
            {
                FT_TS_Bitmap_Bold_VerLine(pBitmap, width, pCurrent, x, y, pixel_black, pPixelFuncs);
            }
        }
        pCurrent = pPixelFuncs->move_row(pBitmap, pCurrent);
    }

    pCurrent = pBitmap->buffer;
    for ( y = 0; y < rows; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            if (pCurrent[x] == FT_TS_BITMAP_PIXEL_GRAY_DARK)
            {
                pPixelFuncs->set_value_x(pBitmap, pCurrent, x, pixel_black);
            }
        }
        pCurrent = pPixelFuncs->move_row(pBitmap, pCurrent);
    }

    return 0;
}

  /* documentation is in ftbitmapext.h */

  /* Enlarge `bitmap' horizontally and vertically by `xpixels' */
  /* and `ypixels', respectively.                              */

  static FT_TS_Error
  ft_bitmap_assure_buffer( FT_TS_Memory   memory,
                           FT_TS_Bitmap*  bitmap,
                           FT_TS_UInt     xpixels,
                           FT_TS_UInt     ypixels )
  {
    FT_TS_Error error;
    UINT        pitch;
    UINT        new_pitch;
    FT_TS_UInt  bpp;
    FT_TS_UInt  width, height;
    UCHAR*      buffer = NULL;
    FT_TS_Bitmap_Pixel_MN* pPixelFuncs = FT_TS_Bitmap_Get_PixelFuncsByBitmap(bitmap);

    width  = pPixelFuncs->get_width(bitmap);
    height = pPixelFuncs->get_rows (bitmap);
    pitch  = pPixelFuncs->get_pitch(bitmap);
    pitch  = (UINT)FT_TS_ABS( pitch );

    switch ( bitmap->pixel_mode )
    {
    case FT_TS_PIXEL_MODE_MONO:
      bpp       = 1;
      new_pitch = ( width + xpixels + 7 ) >> 3;
      break;
    case FT_TS_PIXEL_MODE_GRAY2:
      bpp       = 2;
      new_pitch = ( width + xpixels + 3 ) >> 2;
      break;
    case FT_TS_PIXEL_MODE_GRAY4:
      bpp       = 4;
      new_pitch = ( width + xpixels + 1 ) >> 1;
      break;
    case FT_TS_PIXEL_MODE_GRAY:
    case FT_TS_PIXEL_MODE_LCD:
    case FT_TS_PIXEL_MODE_LCD_V:
      bpp       = 8;
      new_pitch = width + xpixels;
      break;
    default:
      return FT_TS_THROW( Invalid_Glyph_Format );
    }

    /* if no need to allocate memory */
    if ( ypixels == 0 && new_pitch <= pitch )
    {
      /* zero the padding */
      FT_TS_UInt  bit_width = pitch * 8;
      FT_TS_UInt  bit_last  = ( width + xpixels ) * bpp;

      if ( bit_last < bit_width )
      {
        FT_TS_Byte*  line  = bitmap->buffer + ( bit_last >> 3 );
        FT_TS_Byte*  end   = bitmap->buffer + pitch;
        FT_TS_UInt   shift = bit_last & 7;
        FT_TS_UInt   mask  = 0xFF00U >> shift;
        FT_TS_UInt   count = height;

        for ( ; count > 0; count--, line += pitch, end += pitch )
        {
          FT_TS_Byte*  write = line;

          if ( shift > 0 )
          {
            write[0] = (FT_TS_Byte)( write[0] & mask );
            write++;
          }
          if ( write < end )
            FT_TS_MEM_ZERO( write, end - write );
        }
      }

      return FT_TS_Err_Ok;
    }

    /* otherwise allocate new buffer */
    if ( FT_TS_QALLOC_MULT( buffer, height + ypixels, new_pitch ) )
      return error;

    /* new rows get added at the top of the bitmap, */
    /* thus take care of the flow direction         */
    if ( pPixelFuncs->get_pitch(bitmap) > 0 )
    {
      FT_TS_UInt len = ( width * bpp + 7 ) >> 3;

      UCHAR*     in  = bitmap->buffer;
      UCHAR*     out = buffer;

      UCHAR*   limit = bitmap->buffer + pitch * height;
      UINT     delta = new_pitch - len;

      FT_TS_MEM_ZERO( out, new_pitch * ypixels );
      out += new_pitch * ypixels;

      while ( in < limit )
      {
        FT_TS_MEM_COPY( out, in, len );
        in  += pitch;
        out += len;

        /* we use FT_TS_QALLOC_MULT, which doesn't zero out the buffer;      */
        /* consequently, we have to manually zero out the remaining bytes */
        FT_TS_MEM_ZERO( out, delta );
        out += delta;
      }
    }
    else
    {
      FT_TS_UInt len = ( width * bpp + 7 ) >> 3;

      UCHAR*     in  = bitmap->buffer;
      UCHAR*     out = buffer;

      UCHAR*   limit = bitmap->buffer + pitch * height;
      UINT     delta = new_pitch - len;

      while ( in < limit )
      {
        FT_TS_MEM_COPY( out, in, len );
        in  += pitch;
        out += len;

        FT_TS_MEM_ZERO( out, delta );
        out += delta;
      }

      FT_TS_MEM_ZERO( out, new_pitch * ypixels );
    }

    FT_TS_FREE( bitmap->buffer );
    bitmap->buffer = buffer;

    /* set pitch only, width and height are left untouched */
    if ( pPixelFuncs->get_pitch(bitmap) < 0 )
      pPixelFuncs->set_pitch(bitmap, -(int)new_pitch);
    else
      pPixelFuncs->set_pitch(bitmap,  (int)new_pitch);

    return FT_TS_Err_Ok;
  }

  static FT_TS_Error
  FT_TS_Bitmap_Check_XYStrength(
                         FT_TS_Pos       xStrength,
                         FT_TS_Pos       yStrength,
                         FT_TS_Int*      pXstr,
                         FT_TS_Int*      pYstr)
  {
    FT_TS_Error        error;

    /**
      this action is devide by 64
      In fact, there should be: 1, 0
     */
    (*pXstr) = (FT_TS_Int)FT_TS_PIX_ROUND( xStrength ) >> 6;
    (*pYstr) = (FT_TS_Int)FT_TS_PIX_ROUND( yStrength ) >> 6;

    if ( ( (*pXstr) > FT_TS_INT_MAX ) ||
         ( (*pYstr) > FT_TS_INT_MAX ) )
      return FT_TS_THROW( Invalid_Argument );

    if ( (*pXstr) < 0 || (*pYstr) < 0 )
      return FT_TS_THROW( Invalid_Argument );

    return FT_TS_Err_Ok;
  }

  /**
   has any use ?
   */
  static FT_TS_Error
  FT_TS_Bitmap_Check_Pixel(FT_TS_Library   library,
                         FT_TS_Bitmap*   bitmap,
                         FT_TS_Int*      pXstr,
                         FT_TS_Int*      pYstr)
  {
    FT_TS_Error        error;

    switch ( bitmap->pixel_mode )
    {
    case FT_TS_PIXEL_MODE_GRAY2:
    case FT_TS_PIXEL_MODE_GRAY4:
      {
        FT_TS_Bitmap  tmp;

        /* convert to 8bpp */
        FT_TS_Bitmap_Init( &tmp );
        error = FT_TS_Bitmap_Convert( library, bitmap, &tmp, 1 );
        if ( error )
          return error;

        FT_TS_Bitmap_Done( library, bitmap );
        *bitmap = tmp;
      }
      break;

    case FT_TS_PIXEL_MODE_MONO:
      if ( (*pXstr) > 8 )
        (*pXstr) = 8;
      break;

    case FT_TS_PIXEL_MODE_LCD:
      (*pXstr) *= 3;
      break;

    case FT_TS_PIXEL_MODE_LCD_V:
      (*pYstr) *= 3;
      break;

    case FT_TS_PIXEL_MODE_BGRA:
      /* We don't embolden color glyphs. */
      return FT_TS_Err_Ok;
    }

    error = ft_bitmap_assure_buffer( library->memory, bitmap,
                                     (FT_TS_UInt)(*pXstr), (FT_TS_UInt)(*pYstr) );
    if ( error )
        return error;

    return FT_TS_Err_Ok;
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Embolden( FT_TS_Library   library,
                         FT_TS_Bitmap*   bitmap,
                         FT_TS_GlyphSlot slot,
                         FT_TS_Pos       strength )
  {
    return FT_TS_Bitmap_EmboldenXY(library, bitmap, slot, strength, strength, FT_TS_LOAD_DEFAULT, 0);
  }
  
  /**
   * 考虑到工作简化，采取了先转回0度，再转回原角度的方案。
   * 这样可以确保正确，而且各角度完全一样。
   */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_EmboldenXY( FT_TS_Library   library,
                           FT_TS_Bitmap*   bitmap,
                           FT_TS_GlyphSlot slot,
                           FT_TS_Pos       xStrength,
                           FT_TS_Pos       yStrength,
                           FT_TS_Int32     load_flags,
                           int             office_flags)
  {
    FT_TS_Error        error;
    FT_TS_Int          xstr, ystr;
    int                degree;
    int                grayFlag   = 0;
    FT_TS_Bitmap_Pixel_MN* pPixelFuncs;

    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !bitmap || !bitmap->buffer )
      return FT_TS_THROW( Invalid_Argument );

    error = FT_TS_Bitmap_Check_XYStrength(
        xStrength, yStrength,
        &xstr, &ystr);
    if (error)
    {
        return error;
    }

#if 0
    error = FT_TS_Bitmap_Check_Pixel(library, bitmap,
        &xstr, &ystr);
    if (error)
    {
        return error;
    }
#endif

    degree = FT_TS_GlyphSlot_Get_Degree_From_Slot(slot);
    if (   FT_TS_CHECK_BITMAP_BOLD_GRAY(office_flags)
        || (degree ==   0 && FT_TS_CHECK_BITMAP_BOLD_GRAY_NORTH(office_flags))
        || (degree ==  90 && FT_TS_CHECK_BITMAP_BOLD_GRAY_EAST( office_flags))
        || (degree == 180 && FT_TS_CHECK_BITMAP_BOLD_GRAY_SORTH(office_flags))
        || (degree == 270 && FT_TS_CHECK_BITMAP_BOLD_GRAY_WEST( office_flags))
        )
    {
        FT_TS_Bitmap_Create_Buffer_Bit2Byte(bitmap, load_flags);
        grayFlag = 1;
    }

    if (degree != 0)
    {
        FT_TS_Bitmap_Rotate(bitmap, 360-degree, NULL, office_flags);
    }

    FT_TS_Bitmap_Create_BufferWeight(bitmap, xstr, ystr);

    if (grayFlag)
    {
        FT_TS_Bitmap_Bold_Action_Gray(bitmap, xstr, ystr);
    }
    else
    {
        FT_TS_Bitmap_Bold_Action_Default(bitmap, xstr, ystr);
    }

    if (degree != 0)
    {
        FT_TS_Bitmap_Rotate(bitmap, degree, NULL, office_flags);
    }

    return FT_TS_Err_Ok;
  }

  /**
   Bold and Weight }}}}}}}}}}
   */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Load_Glyph( 
                      FT_TS_Library   library,
                      FT_TS_Bitmap*   bitmap,
                      FT_TS_GlyphSlot slot,
                      FT_TS_Int32     load_flags,
                      int             office_flags)
  {
    int       degree;

    degree = FT_TS_GlyphSlot_Get_Degree_From_Slot(slot);
    if (!FT_TS_GlyphSlot_Is_Valid_BitmapDegree(degree))
    {
        return FT_TS_Err_Ok;
    }

    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !bitmap || !bitmap->buffer )
      return FT_TS_THROW( Invalid_Argument );

    /** process flip first */
    FT_TS_Bitmap_Flip(bitmap, slot, office_flags);

    FT_TS_Bitmap_Rotate(bitmap, degree, slot, office_flags);

    return FT_TS_Err_Ok;
  }


/* END */
