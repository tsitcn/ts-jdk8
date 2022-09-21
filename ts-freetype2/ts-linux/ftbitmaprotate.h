/****************************************************************************
 *
 * ftbitmaprotate.h
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


#ifndef FTBITMAP_ROTATE_H_
#define FTBITMAP_ROTATE_H_


#include <freetype/freetype.h>
#include <freetype/ftcolor.h>
#include <freetype/ftbitmapext.h>
#include <freetype/ftbitmappixel.h>

FT_TS_BEGIN_HEADER

/**
 TSIT ROTATE {{{{{{{{{{
 */

/**
 宋体的位图区间是：[12, 17]
 */
#define FONT_SIZE_12      896
#define FONT_SIZE_13      960
#define FONT_SIZE_14     1024
#define FONT_SIZE_15     1088
#define FONT_SIZE_16     1152
#define FONT_SIZE_17     1216


int FT_TS_Bitmap_Italic_Hor (FT_TS_Bitmap* pDst, const int degree, const int to_bottom, float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);

int FT_TS_Bitmap_Italic_Ver (FT_TS_Bitmap* pDst, const int degree, const int to_bottom, float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);

typedef int (*Type_Rotate_Init_4_Rotate     )(FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);
typedef int (*Type_Rotate_Init_4_Italic     )(FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);
typedef int (*Type_Rotate_Init_4_Bold       )(FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);

typedef int (*Type_Rotate_Slot_Position     )(FT_TS_GlyphSlot slot, const FT_TS_Bitmap* pSrc, const int flags,  const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);
typedef int (*Type_Rotate_Dst_Coords        )(int* pDstCoords, int xSrc, int ySrc, int src_width, int src_rows, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);

typedef int (*Type_Rotate_Italic_Action     )(FT_TS_Bitmap* pDst, const int degree, const int to_bottom, const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs);


typedef struct FT_TS_Bitmap_Rotate_90N
{

    Type_Rotate_Init_4_Rotate   init_4_rotate;
    Type_Rotate_Init_4_Italic   init_4_italic;
    Type_Rotate_Init_4_Bold     init_4_bold;

    Type_Rotate_Dst_Coords      rotate_dst_coords;
    Type_Rotate_Slot_Position   rotate_slot_position;

    Type_Rotate_Italic_Action   italic_action;

} FT_TS_Bitmap_Rotate_90N;


#define    NAME_FT_TS_BITMAP_ROTATE_INIT_4_90(count)         FT_TS_Bitmap_Rotate_Init_4_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_90(count)         int NAME_FT_TS_BITMAP_ROTATE_INIT_4_90(count)( \
                                                             FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)
#define    NAME_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(count)  FT_TS_Bitmap_Rotate_Init_4_Italic_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(count)  int NAME_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(count)( \
                                                             FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)
#define    NAME_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(count)    FT_TS_Bitmap_Rotate_Init_4_Bold_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(count)    int NAME_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(count)( \
                                                             FT_TS_Bitmap* pDst, const FT_TS_Bitmap* pSrc, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)

#define    NAME_FT_TS_BITMAP_ROTATE_DST_COORDS_90(count)     FT_TS_Bitmap_Rotate_Dst_Coords_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_DST_COORDS_90(count)     int NAME_FT_TS_BITMAP_ROTATE_DST_COORDS_90(count)( \
                                                             int* pDstCoords, int xSrc, int ySrc, int src_width, int src_rows, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)
#define    NAME_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(count)  FT_TS_Bitmap_Rotate_Slot_Position_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(count)  int NAME_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(count)( \
                                                             FT_TS_GlyphSlot slot, const FT_TS_Bitmap* pSrc, const int flags,  const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)

#define    NAME_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(count)  FT_TS_Bitmap_Rotate_Italic_Action_90_##count
#define DECLARE_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(count)  int NAME_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(count)( \
                                                             FT_TS_Bitmap* pDst, const int to_bottom, const int degree, const float oblique, FT_TS_Bitmap* pSrc, FT_TS_GlyphSlot slot, const FT_TS_Bitmap_Pixel_MN*   pPixelFuncs)

#define    NAME_FT_TS_BITMAP_ROTATE_INIT_90(count)           FT_TS_Bitmap_Rotate_Init_90_##count()

#define DECLARE_FT_TS_BITMAP_ROTATE_INIT_90(count)           DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_90(count);  \
                                                             DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(count); \
                                                             DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(count); \
                                                             DECLARE_FT_TS_BITMAP_ROTATE_DST_COORDS_90(count); \
                                                             DECLARE_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(count); \
                                                             DECLARE_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(count); \
                                                             static FT_TS_Bitmap_Rotate_90N FT_TS_Bitmap_RotateFuncs##count = { \
                                                                 NAME_FT_TS_BITMAP_ROTATE_INIT_4_90(count), \
                                                                 NAME_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(count), \
                                                                 NAME_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(count), \
                                                                 NAME_FT_TS_BITMAP_ROTATE_DST_COORDS_90(count), \
                                                                 NAME_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(count), \
                                                                 NAME_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(count), \
                                                             }; \
                                                             FT_TS_Bitmap_Rotate_90N* NAME_FT_TS_BITMAP_ROTATE_INIT_90(count) \
                                                             { \
                                                                 return &(FT_TS_Bitmap_RotateFuncs##count); \
                                                             }

   FT_TS_Bitmap_Rotate_90N* NAME_FT_TS_BITMAP_ROTATE_INIT_90(0);
   FT_TS_Bitmap_Rotate_90N* NAME_FT_TS_BITMAP_ROTATE_INIT_90(1);
   FT_TS_Bitmap_Rotate_90N* NAME_FT_TS_BITMAP_ROTATE_INIT_90(2);
   FT_TS_Bitmap_Rotate_90N* NAME_FT_TS_BITMAP_ROTATE_INIT_90(3);

/**
  TSIT ROTATE }}}}}}}}}}
  */

FT_TS_END_HEADER

#endif /* FTBITMAP_ROTATE_H_ */


/* END */
