/****************************************************************************
 *
 * ftbitmappixel.h
 *
 *   FreeType utility functions for bitmaps (specification).
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


#ifndef FTBITMAP_PIXEL_H_
#define FTBITMAP_PIXEL_H_


#include <freetype/freetype.h>
#include <freetype/ftcolor.h>
#include <freetype/ftbitmapext.h>

FT_TS_BEGIN_HEADER

/**
 TSIT PIXEL {{{{{{{{{{
 */

#define FT_TS_BITMAP_PIXEL_BLACK           0xFF

#define FT_TS_BITMAP_PIXEL_GRAY_DARK       0xFD
#define FT_TS_BITMAP_PIXEL_GRAY_HEAVY      0xB0
#define FT_TS_BITMAP_PIXEL_GRAY_LIGHT      0x50

typedef int (*Type_Pixel_Init_With_Bitmap      )(FT_TS_Bitmap* pDst,    const FT_TS_Bitmap* pSrc);

typedef int (*Type_Pixel_Get_Width             )(const FT_TS_Bitmap* pBitmap);
typedef int (*Type_Pixel_Set_Width             )(      FT_TS_Bitmap* pBitmap, const int width);

typedef int (*Type_Pixel_Get_Rows              )(const FT_TS_Bitmap* pBitmap);
typedef int (*Type_Pixel_Set_Rows              )(      FT_TS_Bitmap* pBitmap, const int rows);

typedef int (*Type_Pixel_Get_Pitch             )(const FT_TS_Bitmap* pBitmap);
typedef int (*Type_Pixel_Set_Pitch             )(      FT_TS_Bitmap* pBitmap, const int pitch);

typedef int (*Type_Pixel_Copy_Row              )(const FT_TS_Bitmap* pBitmapSrc, UCHAR** pBufferSrc,
                                                 const FT_TS_Bitmap* pBitmapDst, UCHAR** pBufferDst);

typedef int (*Type_Pixel_Copy_Row2             )(const FT_TS_Bitmap* pBitmapSrc, UCHAR** pBufferSrc, const int aheadSrc,
                                                 const FT_TS_Bitmap* pBitmapDst, UCHAR** pBufferDst, const int aheadDst);

typedef int (*Type_Pixel_Get_Pitch_Full        )(const FT_TS_Bitmap* pBitmap);
typedef UCHAR* (*Type_Pixel_Move_Row           )(const FT_TS_Bitmap* pBitmap, UCHAR* pBuffer);

typedef int (*Type_Pixel_Get_Value_X           )(const UCHAR* pBuffer, const int xSrc);
typedef int (*Type_Pixel_Set_Value_X           )(const FT_TS_Bitmap* pBitmapSrc,
                                                       UCHAR* pBuffer, const int xSrc, const UCHAR value);

typedef int (*Type_Pixel_Dump_Data             )(const FT_TS_Bitmap* pBitmap, UCHAR* pData, const int x, UCHAR* pBuffer, const int pos);

typedef int (*Type_Pixel_Copy_XY               )(const FT_TS_Bitmap* pBitmapSrc, UCHAR* pSrc, int xSrc, int ySrc,
                                                 const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst, int xDst, int yDst);
typedef int (*Type_Pixel_Swap_XY               )(const FT_TS_Bitmap* pBitmapSrc, UCHAR* pSrc, int xSrc, int ySrc,
                                                 const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst, int xDst, int yDst);

typedef int (*Type_Pixel_Convert_From          )(const UCHAR pixelSrc, const UCHAR* pSrc, const int x,
                                                 const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst);

typedef int (*Type_Pixel_Init_Pitch            )(FT_TS_Bitmap* pBitmap);
typedef int (*Type_Pixel_Init_Buffer           )(FT_TS_Bitmap* pBitmap);

typedef struct FT_TS_Bitmap_Pixel_MN
{    
    Type_Pixel_Init_With_Bitmap init_with_bitmap;

    Type_Pixel_Get_Width        get_width;
    Type_Pixel_Set_Width        set_width;

    Type_Pixel_Get_Width        get_rows;
    Type_Pixel_Set_Width        set_rows;

    Type_Pixel_Get_Pitch        get_pitch;
    Type_Pixel_Set_Pitch        set_pitch;

    Type_Pixel_Copy_Row         copy_row;
    Type_Pixel_Copy_Row2        copy_row2;

    Type_Pixel_Get_Pitch_Full   get_pitch_full;
    Type_Pixel_Move_Row         move_row;

    Type_Pixel_Get_Value_X      get_value_x;
    Type_Pixel_Set_Value_X      set_value_x;

    Type_Pixel_Copy_XY          copy_xy;

    Type_Pixel_Swap_XY          swap_xy;

    Type_Pixel_Convert_From     convert_from;

    Type_Pixel_Dump_Data        dump_data;

    Type_Pixel_Init_Pitch       init_pitch;
    Type_Pixel_Init_Buffer      init_buffer;

} FT_TS_Bitmap_Pixel_MN;

FT_TS_Bitmap_Pixel_MN* FT_TS_Bitmap_Get_PixelFuncsByBitmap(const FT_TS_Bitmap*  pBitmap);
FT_TS_Bitmap_Pixel_MN* FT_TS_Bitmap_Get_PixelFuncsByMode(const int nMode);


#define    NAME_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(bits, mode)        FT_TS_Bitmap_Pixel_Init_With_Bitmap_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(bits, mode)        int NAME_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(bits, mode)( \
                                                                            FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc)

#define    NAME_FT_TS_BITMAP_PIXEL_GET_WIDTH(bits, mode)               FT_TS_Bitmap_Pixel_Get_Width_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_GET_WIDTH(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap)

#define    NAME_FT_TS_BITMAP_PIXEL_SET_WIDTH(bits, mode)               FT_TS_Bitmap_Pixel_Set_Width_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_SET_WIDTH(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap, const int width)

#define    NAME_FT_TS_BITMAP_PIXEL_GET_ROWS(bits, mode)                FT_TS_Bitmap_Pixel_Get_Rows_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(bits, mode)                int NAME_FT_TS_BITMAP_PIXEL_GET_ROWS(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap)

#define    NAME_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)                FT_TS_Bitmap_Pixel_Set_Rows_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)                int NAME_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap, const int rows)

#define    NAME_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)                FT_TS_Bitmap_Pixel_Set_Rows_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)                int NAME_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap, const int rows)

#define    NAME_FT_TS_BITMAP_PIXEL_GET_PITCH(bits, mode)               FT_TS_Bitmap_Pixel_Get_Pitch_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_GET_PITCH(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap)

#define    NAME_FT_TS_BITMAP_PIXEL_SET_PITCH(bits, mode)               FT_TS_Bitmap_Pixel_Set_Pitch_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_SET_PITCH(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap, const int pitch)

#define    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW(bits, mode)                FT_TS_Bitmap_Pixel_Copy_Row_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(bits, mode)                int NAME_FT_TS_BITMAP_PIXEL_COPY_ROW(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmapSrc, UCHAR** pBufferSrc, \
                                                                            const FT_TS_Bitmap* pBitmapDst, UCHAR** pBufferDst)

#define    NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(bits, mode)               FT_TS_Bitmap_Pixel_Copy_Row2_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmapSrc, UCHAR** pBufferSrc, const int aheadSrc, \
                                                                            const FT_TS_Bitmap* pBitmapDst, UCHAR** pBufferDst, const int aheadDst)

#define    NAME_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(bits, mode)          FT_TS_Bitmap_Pixel_Get_Pitch_Full_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(bits, mode)          int NAME_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap)

#define    NAME_FT_TS_BITMAP_PIXEL_MOVE_ROW(bits, mode)                FT_TS_Bitmap_Pixel_Move_Row_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(bits, mode)                UCHAR* NAME_FT_TS_BITMAP_PIXEL_MOVE_ROW(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap, UCHAR* pBuffer)

#define    NAME_FT_TS_BITMAP_PIXEL_GET_VALUE_X(bits, mode)             FT_TS_Bitmap_Pixel_Get_Value_X_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(bits, mode)             int NAME_FT_TS_BITMAP_PIXEL_GET_VALUE_X(bits, mode)( \
                                                                            const UCHAR* pSrc, const int xSrc)

#define    NAME_FT_TS_BITMAP_PIXEL_SET_VALUE_X(bits, mode)             FT_TS_Bitmap_Pixel_Set_Value_X_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(bits, mode)             int NAME_FT_TS_BITMAP_PIXEL_SET_VALUE_X(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmapSrc, UCHAR* pSrc, const int xSrc, const UCHAR value)

#define    NAME_FT_TS_BITMAP_PIXEL_COPY_XY(bits, mode)                 FT_TS_Bitmap_Pixel_Copy_XY_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(bits, mode)                 int NAME_FT_TS_BITMAP_PIXEL_COPY_XY(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmapSrc, UCHAR* pSrc, int xSrc, int ySrc, \
                                                                            const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst, int xDst, int yDst)

#define    NAME_FT_TS_BITMAP_PIXEL_SWAP_XY(bits, mode)                 FT_TS_Bitmap_Pixel_Swap_XY_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(bits, mode)                 int NAME_FT_TS_BITMAP_PIXEL_SWAP_XY(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmapSrc, UCHAR* pSrc, int xSrc, int ySrc, \
                                                                            const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst, int xDst, int yDst)

#define    NAME_FT_TS_BITMAP_PIXEL_CONVERT_FROM(bits, mode)            FT_TS_Bitmap_Pixel_Convert_From_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(bits, mode)            int NAME_FT_TS_BITMAP_PIXEL_CONVERT_FROM(bits, mode)( \
                                                                            const UCHAR pixelSrc, const UCHAR* pSrc, const int x, \
                                                                            const FT_TS_Bitmap* pBitmapDst, UCHAR* pDst)

#define    NAME_FT_TS_BITMAP_PIXEL_DUMP_DATA(bits, mode)               FT_TS_Bitmap_Pixel_Dump_Data_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(bits, mode)               int NAME_FT_TS_BITMAP_PIXEL_DUMP_DATA(bits, mode)( \
                                                                            const FT_TS_Bitmap* pBitmap, UCHAR* pData, const int x, UCHAR* pBuffer, const int pos)

#define    NAME_FT_TS_BITMAP_PIXEL_INIT_PITCH(bits, mode)              FT_TS_Bitmap_Pixel_Init_Pitch_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(bits, mode)              int NAME_FT_TS_BITMAP_PIXEL_INIT_PITCH(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap)

#define    NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(bits, mode)             FT_TS_Bitmap_Pixel_Init_Buffer_##bits_##mode
#define DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(bits, mode)             int NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(bits, mode)( \
                                                                            FT_TS_Bitmap* pBitmap)



#define    NAME_FT_TS_BITMAP_PIXEL_INIT(bits, mode)          FT_TS_Bitmap_Pixel_Init_##bits_##mode()

#define DECLARE_FT_TS_BITMAP_PIXEL_INIT(bits, mode)          DECLARE_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_GET_WIDTH(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_SET_WIDTH(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_GET_ROWS(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_SET_PITCH(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_COPY_ROW2(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_MOVE_ROW(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_GET_VALUE_X(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_SET_VALUE_X(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_COPY_XY(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_SWAP_XY(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_CONVERT_FROM(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_DUMP_DATA(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_INIT_PITCH(bits, mode);  \
                                                             DECLARE_FT_TS_BITMAP_PIXEL_INIT_BUFFER(bits, mode);  \
                                                             static FT_TS_Bitmap_Pixel_MN FT_TS_Bitmap_PixelFuncs_##bits_##mode = { \
                                                                 NAME_FT_TS_BITMAP_PIXEL_INIT_WITH_BITMAP(bits, mode),  \
                                                                 NAME_FT_TS_BITMAP_PIXEL_GET_WIDTH(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_SET_WIDTH(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_GET_ROWS(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_SET_ROWS(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_GET_PITCH(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_SET_PITCH(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_COPY_ROW(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_COPY_ROW2(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_GET_PITCH_FULL(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_MOVE_ROW(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_GET_VALUE_X(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_SET_VALUE_X(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_COPY_XY(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_SWAP_XY(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_CONVERT_FROM(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_DUMP_DATA(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_INIT_PITCH(bits, mode), \
                                                                 NAME_FT_TS_BITMAP_PIXEL_INIT_BUFFER(bits, mode), \
                                                             }; \
                                                             FT_TS_Bitmap_Pixel_MN* NAME_FT_TS_BITMAP_PIXEL_INIT(bits, mode) \
                                                             { \
                                                                 return &(FT_TS_Bitmap_PixelFuncs_##bits_##mode); \
                                                             }

/**
 JDKµÄ·´¾â³Ý£º
 TEXT_AA_LCD_HRGB£¬FT_TS_PIXEL_MODE_LCD¡£
 TEXT_AA_LCD_HBGR£¬FT_TS_PIXEL_MODE_LCD_V¡£
 */
   FT_TS_Bitmap_Pixel_MN* NAME_FT_TS_BITMAP_PIXEL_INIT( 1, FT_TS_PIXEL_MODE_MONO);
   FT_TS_Bitmap_Pixel_MN* NAME_FT_TS_BITMAP_PIXEL_INIT( 8, FT_TS_PIXEL_MODE_GRAY);
   FT_TS_Bitmap_Pixel_MN* NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD);
   FT_TS_Bitmap_Pixel_MN* NAME_FT_TS_BITMAP_PIXEL_INIT(24, FT_TS_PIXEL_MODE_LCD_V);

/**
  PIXEL }}}}}}}}}}
  */

FT_TS_END_HEADER

#endif /* FTBITMAP_PIXEL_H_ */


/* END */
