/****************************************************************************
 *
 * ftbitmapext.h
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


#ifndef FTBITMAP_EXT_H_
#define FTBITMAP_EXT_H_


#include <freetype/freetype.h>
#include <freetype/ftcolor.h>

FT_TS_BEGIN_HEADER

/**
 TSIT {{{{{{{{{{
 */


/* 0xC0 ? */
#define FT_TS_BITMAP_PIXEL_MASK     0x80

#define UCHAR unsigned char
#define UINT  unsigned int

  int  FT_TS_Bitmap_Init_Buffer(  FT_TS_Bitmap* pBitmap);
  void FT_TS_Bitmap_Change_Buffer(FT_TS_Bitmap* pDst,   FT_TS_Bitmap* pSrc);

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Bitmap_Italic
   *
   * @description:
   *   Italic a bitmap. 
   *
   * @input:
   *   slot ::
   *
   *
   * @inout:
   *   bitmap ::
   *     A handle to the target bitmap.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Bitmap_Italic(FT_TS_GlyphSlot  slot);

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Bitmap_Italic_Direction
   *
   * @description:
   *   Italic a bitmap, the direction may be LTR or TTB. 
   *
   * @input:
   *   slot ::
   *
   *   to_bottom::
   *
   *   degree ::
   *
   *   yStrength ::
   *     How strong the glyph is emboldened vertically.  Expressed in 26.6
   *     pixel format.
   *
   * @inout:
   *   bitmap ::
   *     A handle to the target bitmap.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Bitmap_Italic_Direction(FT_TS_GlyphSlot  slot,
                      float          oblique,
                      int            to_bottom,
                      int            degree);

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Bitmap_Load_Glyph
   *
   * @description:
   *   Flip and rotate a bitmap. The degree may be 0, 90, 270
   *
   * @input:
   *   library ::
   *     A handle to a library object.
   *
   *   slot ::
   *
   * @inout:
   *   bitmap ::
   *     A handle to the target bitmap.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Bitmap_Load_Glyph(
                      FT_TS_Library   library,
                      FT_TS_Bitmap*   bitmap,
                      FT_TS_GlyphSlot slot,
                      FT_TS_Int32     load_flags,
                      int             office_flags);

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Bitmap_Embolden
   *
   * @description:
   *   Embolden a bitmap.  Use same value for wider and higher. 
   *   The left and bottom borders are kept unchanged.
   *
   * @input:
   *   library ::
   *     A handle to a library object.
   *
   *   strength ::
   *     How strong the glyph is emboldened horizontally and vertically.  Expressed in 26.6
   *     pixel format.
   *
   * @inout:
   *   bitmap ::
   *     A handle to the target bitmap.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The current implementation restricts `xStrength` to be less than or
   *   equal to~8 if bitmap is of pixel_mode @FT_TS_PIXEL_MODE_MONO.
   *
   *   If you want to embolden the bitmap owned by a @FT_TS_GlyphSlotRec, you
   *   should call @FT_TS_GlyphSlot_Own_Bitmap on the slot first.
   *
   *   Bitmaps in @FT_TS_PIXEL_MODE_GRAY2 and @FT_TS_PIXEL_MODE_GRAY@ format are
   *   converted to @FT_TS_PIXEL_MODE_GRAY format (i.e., 8bpp).
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Bitmap_Embolden( FT_TS_Library   library,
                         FT_TS_Bitmap*   bitmap,
                         FT_TS_GlyphSlot slot,
                         FT_TS_Pos       strength );

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Bitmap_EmboldenXY
   *
   * @description:
   *   Embolden a bitmap.  The new bitmap will be about `xStrength` pixels
   *   wider and `yStrength` pixels higher.  The left and bottom borders are
   *   kept unchanged.
   *
   * @input:
   *   library ::
   *     A handle to a library object.
   *
   *   xStrength ::
   *     How strong the glyph is emboldened horizontally.  Expressed in 26.6
   *     pixel format.
   *
   *   yStrength ::
   *     How strong the glyph is emboldened vertically.  Expressed in 26.6
   *     pixel format.
   *
   * @inout:
   *   bitmap ::
   *     A handle to the target bitmap.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The current implementation restricts `xStrength` to be less than or
   *   equal to~8 if bitmap is of pixel_mode @FT_TS_PIXEL_MODE_MONO.
   *
   *   If you want to embolden the bitmap owned by a @FT_TS_GlyphSlotRec, you
   *   should call @FT_TS_GlyphSlot_Own_Bitmap on the slot first.
   *
   *   Bitmaps in @FT_TS_PIXEL_MODE_GRAY2 and @FT_TS_PIXEL_MODE_GRAY@ format are
   *   converted to @FT_TS_PIXEL_MODE_GRAY format (i.e., 8bpp).
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Bitmap_EmboldenXY(
                      FT_TS_Library   library,
                      FT_TS_Bitmap*   bitmap,
                      FT_TS_GlyphSlot slot,
                      FT_TS_Pos       xStrength,
                      FT_TS_Pos       yStrength,
                      FT_TS_Int32     load_flags,
                      int             office_flags);

/**
  TSIT }}}}}}}}}}
  */

FT_TS_END_HEADER

#endif /* FTBITMAP_EXT_H_ */


/* END */
