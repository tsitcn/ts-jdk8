/****************************************************************************
 *
 * ftsynth.c
 *
 *   FreeType synthesizing code for emboldening and slanting (body).
 *
 * Copyright (C) 2000-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/ftsynth.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbitmap.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  synth


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL OBLIQUING SUPPORT                                ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /* documentation is in ftsynth.h */

/**
 TSIT {{{{{{{{{{
 */

  FT_TS_EXPORT_DEF( int )
  FT_TS_GlyphSlot_Get_Degree_From_Matrix(const FT_TS_Matrix* pMatrix)
  {
    if (!pMatrix)
    {
        return 0;
    }

    if (  pMatrix->xx ==  0x10000
       && pMatrix->xy ==  0
       && pMatrix->yx ==  0
       && pMatrix->yy ==  0x10000)
    {
        return 0;
    }

    if (  pMatrix->xx ==  0
       && pMatrix->xy ==  0x10000
       && pMatrix->yx == -0x10000
       && pMatrix->yy ==  0)
    {
       return  90;
    }

    if (  pMatrix->xx == -0x10000
       && pMatrix->xy ==  0
       && pMatrix->yx ==  0
       && pMatrix->yy == -0x10000)
    {
        return 180;
    }

    if (pMatrix->xx ==  0
     && pMatrix->xy == -0x10000
     && pMatrix->yx ==  0x10000
     && pMatrix->yy ==  0 )
    {
       return 270;
    }

    /** -1 means unknown degree. */
    return -1;
  }

  FT_TS_EXPORT_DEF( int )
  FT_TS_GlyphSlot_Get_Degree_From_Slot(const FT_TS_GlyphSlot  slot)
  {
    if (!slot)
    {
        return 0;
    }
	return FT_TS_GlyphSlot_Get_Degree_From_Matrix(&(slot->face->internal->transform_matrix));
  }

  FT_TS_EXPORT(int)
  FT_TS_GlyphSlot_Is_Valid_BitmapDegree(const int degree)
  {
    switch (degree % 360)
    {
      case    0:
      case   90:
      case  180:
      case  270:
        return 1;

      default:
        return 0;
    }
  }

  FT_TS_EXPORT(int)
  FT_TS_GlyphSlot_Is_Valid_BitmapMatrix(const FT_TS_Matrix* pMatrix)
  {
      int degree = FT_TS_GlyphSlot_Get_Degree_From_Matrix(pMatrix);
      return FT_TS_GlyphSlot_Is_Valid_BitmapDegree(degree);
  }

  FT_TS_EXPORT_DEF( void )
  FT_TS_GlyphSlot_Oblique( FT_TS_GlyphSlot  slot )
  {
    FT_TS_GlyphSlot_Oblique_Direction(slot, FT_TS_FONT_ITALIC_VALUE, FT_TS_POSTURE_TO_RIGHT);
  }

  FT_TS_EXPORT_DEF( void )
  FT_TS_GlyphSlot_Oblique_Direction( FT_TS_GlyphSlot  slot, float oblique, int flags )
  {
    FT_TS_Matrix    transform = {0};
    FT_TS_Outline*  outline;
    int          to_bottom;
    int          degree;
    int          transvalue;

    if ( !slot )
      return;

    degree    = FT_TS_GlyphSlot_Get_Degree_From_Slot( slot );
    to_bottom = FT_TS_CHECK_POSTURE_TO_BOTTOM(flags);
    if ( slot->format != FT_TS_GLYPH_FORMAT_OUTLINE )
    {
        FT_TS_Bitmap_Italic_Direction(slot, oblique, to_bottom, degree);
        return;
    }

    /* we don't touch the advance width */

    /* For italic, simply apply a shear transform, with an angle */
    /* of about 12 degrees.                                      */
    /* if text both have directory and italic,
       text directory is solid attibute.
       italic is temporary attribute.

       so, I must change transform for directory.
       0x0366AL/0x10000L=0.21
     */
    /* if is oblique=0.21256, transvalue=0x0366AL*/
    transvalue = (int)(0x10000L*oblique);
    if (   degree ==   0 &&  to_bottom
        || degree == 180 &&  to_bottom
        || degree ==  90 && !to_bottom
        || degree == 270 && !to_bottom )
    {
        transform.xx =  0x10000L;
        transform.yx = -transvalue;
        transform.xy =  0x00000L;
        transform.yy =  0x10000L;
    }
    else
    {
        transform.xx =  0x10000L;
        transform.yx =  0x00000L;
        transform.xy =  transvalue;
        transform.yy =  0x10000L;
    }

    outline = &slot->outline;
    FT_TS_Outline_Transform( outline, &transform );
  }

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   EXPERIMENTAL EMBOLDENING SUPPORT                              ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftsynth.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_GlyphSlot_Embolden( FT_TS_GlyphSlot  slot )
  {
    FT_TS_GlyphSlot_Weight(slot, FT_TS_WEIGHT_BOLD, FT_TS_WEIGHT_BOLD, FT_TS_LOAD_DEFAULT, 0);
  }

  FT_TS_EXPORT_DEF( void )
  FT_TS_GlyphSlot_Weight( FT_TS_GlyphSlot  slot, float weight_x, float weight_y,
        int load_flags, int office_flags )
  {
    FT_TS_Library  library;
    FT_TS_Face     face;
    FT_TS_Error    error;
    FT_TS_Pos      xstr, ystr;
    int            changex, changey;

    if ( !slot )
      return;

    library = slot->library;
    face    = slot->face;

    if ( slot->format != FT_TS_GLYPH_FORMAT_OUTLINE &&
         slot->format != FT_TS_GLYPH_FORMAT_BITMAP  )
      return;

    /* some reasonable strength. result is 45 */
    xstr = FT_TS_MulFix( face->units_per_EM,
                      face->size->metrics.y_scale ) / 24;
    ystr = xstr;

    /**
     different weight cause different thick.
     weight plain(1) means no process, so we minus it.
     */
    weight_x = weight_x - FT_TS_WEIGHT_PLAIN;
    xstr     = (int)(xstr * weight_x);
    weight_y = weight_y - FT_TS_WEIGHT_PLAIN;
    ystr     = (int)(ystr * weight_y);

    if ( slot->format == FT_TS_GLYPH_FORMAT_OUTLINE )
    {
      FT_TS_Outline_WeightXY( &slot->outline, xstr, ystr );
    }
    else if (weight_x > 0 && weight_y >= 0)
	/* slot->format == FT_TS_GLYPH_FORMAT_BITMAP */
    {
      /* round to full pixels */
      xstr &= ~63;
      if ( xstr == 0 )
        xstr = 1 << 6;

      ystr &= ~63;
      /** now is 64/0 */

      /*
       * XXX: overflow check for 16-bit system, for compatibility
       *      with FT_TS_GlyphSlot_Embolden() since FreeType 2.1.10.
       *      unfortunately, this function return no informations
       *      about the cause of error.
       */
      if ( ( ystr >> 6 ) > FT_TS_INT_MAX || ( ystr >> 6 ) < FT_TS_INT_MIN )
      {
        FT_TS_TRACE1(( "FT_TS_GlyphSlot_Embolden:" ));
        FT_TS_TRACE1(( "too strong emboldening parameter ystr=%ld\n", ystr ));
        return;
      }
      error = FT_TS_GlyphSlot_Own_Bitmap( slot );
      if ( error )
        return;

      error = FT_TS_Bitmap_EmboldenXY( library, &slot->bitmap, slot, xstr, ystr, load_flags, office_flags );
      if ( error )
        return;
    }

    changex = FT_TS_CHECK_CHANGE_SIZE_ENABLE(office_flags);
    changey = changex;

    changex = (changex && FT_TS_CHECK_CHANGE_SIZE_X_ENABLE(office_flags));
    changey = (changey && FT_TS_CHECK_CHANGE_SIZE_Y_ENABLE(office_flags));

    if (changex)
    {
      /* this change size.*/
      if ( slot->advance.x )
        slot->advance.x += xstr;
      slot->metrics.width        += xstr;
      slot->metrics.horiAdvance  += xstr;
    }

    if (changey)
    {
      if ( slot->advance.y )
        slot->advance.y += ystr;

      slot->metrics.height       += ystr;
      slot->metrics.vertAdvance  += ystr;
      slot->metrics.horiBearingY += ystr;

      /* XXX: 16-bit overflow case must be excluded before here */
      if ( slot->format == FT_TS_GLYPH_FORMAT_BITMAP )
        slot->bitmap_top += (FT_TS_Int)( ystr >> 6 );
    }
  }

/**
 TSIT }}}}}}}}}}
 */

/* END */
