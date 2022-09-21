/****************************************************************************
 *
 * ftbitmap.c
 *
 *   FreeType utility functions for bitmaps (body).
 *
 * Copyright (C) 2004-2022 by
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
#include <freetype/ftimage.h>
#include <freetype/internal/ftobjs.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  bitmap


  static
  const FT_TS_Bitmap  null_bitmap = { 0, 0, 0, NULL, 0, 0, 0, NULL };


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Bitmap_Init( FT_TS_Bitmap  *abitmap )
  {
    if ( abitmap )
      *abitmap = null_bitmap;
  }


  /* deprecated function name; retained for ABI compatibility */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Bitmap_New( FT_TS_Bitmap  *abitmap )
  {
    if ( abitmap )
      *abitmap = null_bitmap;
  }


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Copy( FT_TS_Library        library,
                  const FT_TS_Bitmap  *source,
                  FT_TS_Bitmap        *target)
  {
    FT_TS_Memory  memory;
    FT_TS_Error   error  = FT_TS_Err_Ok;

    FT_TS_Int    pitch;
    FT_TS_ULong  size;

    FT_TS_Int  source_pitch_sign, target_pitch_sign;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !source || !target )
      return FT_TS_THROW( Invalid_Argument );

    if ( source == target )
      return FT_TS_Err_Ok;

    source_pitch_sign = source->pitch < 0 ? -1 : 1;
    target_pitch_sign = target->pitch < 0 ? -1 : 1;

    if ( !source->buffer )
    {
      *target = *source;
      if ( source_pitch_sign != target_pitch_sign )
        target->pitch = -target->pitch;

      return FT_TS_Err_Ok;
    }

    memory = library->memory;
    pitch  = source->pitch;

    if ( pitch < 0 )
      pitch = -pitch;
    size = (FT_TS_ULong)pitch * source->rows;

    if ( target->buffer )
    {
      FT_TS_Int    target_pitch = target->pitch;
      FT_TS_ULong  target_size;


      if ( target_pitch < 0 )
        target_pitch = -target_pitch;
      target_size = (FT_TS_ULong)target_pitch * target->rows;

      if ( target_size != size )
        FT_TS_MEM_QREALLOC( target->buffer, target_size, size );
    }
    else
      FT_TS_MEM_QALLOC( target->buffer, size );

    if ( !error )
    {
      unsigned char *p;


      p = target->buffer;
      *target = *source;
      target->buffer = p;

      if ( source_pitch_sign == target_pitch_sign )
        FT_TS_MEM_COPY( target->buffer, source->buffer, size );
      else
      {
        /* take care of bitmap flow */
        FT_TS_UInt   i;
        FT_TS_Byte*  s = source->buffer;
        FT_TS_Byte*  t = target->buffer;


        t += (FT_TS_ULong)pitch * ( target->rows - 1 );

        for ( i = target->rows; i > 0; i-- )
        {
          FT_TS_ARRAY_COPY( t, s, pitch );

          s += pitch;
          t -= pitch;
        }
      }
    }

    return error;
  }

  /* documentation is in ftbitmap.h */

  static FT_TS_Byte
  ft_gray_for_premultiplied_srgb_bgra( const FT_TS_Byte*  bgra )
  {
    FT_TS_UInt  a = bgra[3];
    FT_TS_UInt  l;


    /* Short-circuit transparent color to avoid division by zero. */
    if ( !a )
      return 0;

    /*
     * Luminosity for sRGB is defined using ~0.2126,0.7152,0.0722
     * coefficients for RGB channels *on the linear colors*.
     * A gamma of 2.2 is fair to assume.  And then, we need to
     * undo the premultiplication too.
     *
     *   http://www.brucelindbloom.com/index.html?WorkingSpaceInfo.html#SideNotes
     *
     * We do the computation with integers only, applying a gamma of 2.0.
     * We guarantee 32-bit arithmetic to avoid overflow but the resulting
     * luminosity fits into 16 bits.
     *
     */

    l = (  4731UL /* 0.072186 * 65536 */ * bgra[0] * bgra[0] +
          46868UL /* 0.715158 * 65536 */ * bgra[1] * bgra[1] +
          13937UL /* 0.212656 * 65536 */ * bgra[2] * bgra[2] ) >> 16;

    /*
     * Final transparency can be determined as follows.
     *
     * - If alpha is zero, we want 0.
     * - If alpha is zero and luminosity is zero, we want 255.
     * - If alpha is zero and luminosity is one, we want 0.
     *
     * So the formula is a * (1 - l) = a - l * a.
     *
     * We still need to undo premultiplication by dividing l by a*a.
     *
     */

    return (FT_TS_Byte)( a - l / a );
  }


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Convert( FT_TS_Library        library,
                     const FT_TS_Bitmap  *source,
                     FT_TS_Bitmap        *target,
                     FT_TS_Int            alignment )
  {
    FT_TS_Error   error = FT_TS_Err_Ok;
    FT_TS_Memory  memory;

    FT_TS_Byte*  s;
    FT_TS_Byte*  t;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !source || !target )
      return FT_TS_THROW( Invalid_Argument );

    memory = library->memory;

    switch ( source->pixel_mode )
    {
    case FT_TS_PIXEL_MODE_MONO:
    case FT_TS_PIXEL_MODE_GRAY:
    case FT_TS_PIXEL_MODE_GRAY2:
    case FT_TS_PIXEL_MODE_GRAY4:
    case FT_TS_PIXEL_MODE_LCD:
    case FT_TS_PIXEL_MODE_LCD_V:
    case FT_TS_PIXEL_MODE_BGRA:
      {
        FT_TS_Int    pad, old_target_pitch, target_pitch;
        FT_TS_ULong  old_size;


        old_target_pitch = target->pitch;
        if ( old_target_pitch < 0 )
          old_target_pitch = -old_target_pitch;

        old_size = target->rows * (FT_TS_UInt)old_target_pitch;

        target->pixel_mode = FT_TS_PIXEL_MODE_GRAY;
        target->rows       = source->rows;
        target->width      = source->width;

        pad = 0;
        if ( alignment > 0 )
        {
          pad = (FT_TS_Int)source->width % alignment;
          if ( pad != 0 )
            pad = alignment - pad;
        }

        target_pitch = (FT_TS_Int)source->width + pad;

        if ( target_pitch > 0                                               &&
             (FT_TS_ULong)target->rows > FT_TS_ULONG_MAX / (FT_TS_ULong)target_pitch )
          return FT_TS_THROW( Invalid_Argument );

        if ( FT_TS_QREALLOC( target->buffer,
                          old_size, target->rows * (FT_TS_UInt)target_pitch ) )
          return error;

        target->pitch = target->pitch < 0 ? -target_pitch : target_pitch;
      }
      break;

    default:
      error = FT_TS_THROW( Invalid_Argument );
    }

    s = source->buffer;
    t = target->buffer;

    /* take care of bitmap flow */
    if ( source->pitch < 0 )
      s -= source->pitch * (FT_TS_Int)( source->rows - 1 );
    if ( target->pitch < 0 )
      t -= target->pitch * (FT_TS_Int)( target->rows - 1 );

    switch ( source->pixel_mode )
    {
    case FT_TS_PIXEL_MODE_MONO:
      {
        FT_TS_UInt  i;


        target->num_grays = 2;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_TS_Byte*  ss = s;
          FT_TS_Byte*  tt = t;
          FT_TS_UInt   j;


          /* get the full bytes */
          for ( j = source->width >> 3; j > 0; j-- )
          {
            FT_TS_Int  val = ss[0]; /* avoid a byte->int cast on each line */


            tt[0] = (FT_TS_Byte)( ( val & 0x80 ) >> 7 );
            tt[1] = (FT_TS_Byte)( ( val & 0x40 ) >> 6 );
            tt[2] = (FT_TS_Byte)( ( val & 0x20 ) >> 5 );
            tt[3] = (FT_TS_Byte)( ( val & 0x10 ) >> 4 );
            tt[4] = (FT_TS_Byte)( ( val & 0x08 ) >> 3 );
            tt[5] = (FT_TS_Byte)( ( val & 0x04 ) >> 2 );
            tt[6] = (FT_TS_Byte)( ( val & 0x02 ) >> 1 );
            tt[7] = (FT_TS_Byte)(   val & 0x01 );

            tt += 8;
            ss += 1;
          }

          /* get remaining pixels (if any) */
          j = source->width & 7;
          if ( j > 0 )
          {
            FT_TS_Int  val = *ss;


            for ( ; j > 0; j-- )
            {
              tt[0] = (FT_TS_Byte)( ( val & 0x80 ) >> 7);
              val <<= 1;
              tt   += 1;
            }
          }

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_TS_PIXEL_MODE_GRAY:
    case FT_TS_PIXEL_MODE_LCD:
    case FT_TS_PIXEL_MODE_LCD_V:
      {
        FT_TS_UInt  width = source->width;
        FT_TS_UInt  i;


        target->num_grays = 256;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_TS_ARRAY_COPY( t, s, width );

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_TS_PIXEL_MODE_GRAY2:
      {
        FT_TS_UInt  i;


        target->num_grays = 4;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_TS_Byte*  ss = s;
          FT_TS_Byte*  tt = t;
          FT_TS_UInt   j;


          /* get the full bytes */
          for ( j = source->width >> 2; j > 0; j-- )
          {
            FT_TS_Int  val = ss[0];


            tt[0] = (FT_TS_Byte)( ( val & 0xC0 ) >> 6 );
            tt[1] = (FT_TS_Byte)( ( val & 0x30 ) >> 4 );
            tt[2] = (FT_TS_Byte)( ( val & 0x0C ) >> 2 );
            tt[3] = (FT_TS_Byte)( ( val & 0x03 ) );

            ss += 1;
            tt += 4;
          }

          j = source->width & 3;
          if ( j > 0 )
          {
            FT_TS_Int  val = ss[0];


            for ( ; j > 0; j-- )
            {
              tt[0]  = (FT_TS_Byte)( ( val & 0xC0 ) >> 6 );
              val  <<= 2;
              tt    += 1;
            }
          }

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_TS_PIXEL_MODE_GRAY4:
      {
        FT_TS_UInt  i;


        target->num_grays = 16;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_TS_Byte*  ss = s;
          FT_TS_Byte*  tt = t;
          FT_TS_UInt   j;


          /* get the full bytes */
          for ( j = source->width >> 1; j > 0; j-- )
          {
            FT_TS_Int  val = ss[0];


            tt[0] = (FT_TS_Byte)( ( val & 0xF0 ) >> 4 );
            tt[1] = (FT_TS_Byte)( ( val & 0x0F ) );

            ss += 1;
            tt += 2;
          }

          if ( source->width & 1 )
            tt[0] = (FT_TS_Byte)( ( ss[0] & 0xF0 ) >> 4 );

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;


    case FT_TS_PIXEL_MODE_BGRA:
      {
        FT_TS_UInt  i;


        target->num_grays = 256;

        for ( i = source->rows; i > 0; i-- )
        {
          FT_TS_Byte*  ss = s;
          FT_TS_Byte*  tt = t;
          FT_TS_UInt   j;


          for ( j = source->width; j > 0; j-- )
          {
            tt[0] = ft_gray_for_premultiplied_srgb_bgra( ss );

            ss += 4;
            tt += 1;
          }

          s += source->pitch;
          t += target->pitch;
        }
      }
      break;

    default:
      ;
    }

    return error;
  }


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Blend( FT_TS_Library        library,
                   const FT_TS_Bitmap*  source_,
                   const FT_TS_Vector   source_offset_,
                   FT_TS_Bitmap*        target,
                   FT_TS_Vector        *atarget_offset,
                   FT_TS_Color          color )
  {
    FT_TS_Error   error = FT_TS_Err_Ok;
    FT_TS_Memory  memory;

    FT_TS_Bitmap         source_bitmap;
    const FT_TS_Bitmap*  source;

    FT_TS_Vector  source_offset;
    FT_TS_Vector  target_offset;

    FT_TS_Bool  free_source_bitmap          = 0;
    FT_TS_Bool  free_target_bitmap_on_error = 0;

    FT_TS_Pos  source_llx, source_lly, source_urx, source_ury;
    FT_TS_Pos  target_llx, target_lly, target_urx, target_ury;
    FT_TS_Pos  final_llx, final_lly, final_urx, final_ury;

    unsigned int  final_rows, final_width;
    long          x, y;


    if ( !library || !target || !source_ || !atarget_offset )
      return FT_TS_THROW( Invalid_Argument );

    memory = library->memory;

    if ( !( target->pixel_mode == FT_TS_PIXEL_MODE_NONE     ||
            ( target->pixel_mode == FT_TS_PIXEL_MODE_BGRA &&
              target->buffer                           ) ) )
      return FT_TS_THROW( Invalid_Argument );

    if ( source_->pixel_mode == FT_TS_PIXEL_MODE_NONE )
      return FT_TS_Err_Ok;               /* nothing to do */

    /* pitches must have the same sign */
    if ( target->pixel_mode == FT_TS_PIXEL_MODE_BGRA &&
         ( source_->pitch ^ target->pitch ) < 0   )
      return FT_TS_THROW( Invalid_Argument );

    if ( !( source_->width && source_->rows ) )
      return FT_TS_Err_Ok;               /* nothing to do */

    /* assure integer pixel offsets */
    source_offset.x = FT_TS_PIX_FLOOR( source_offset_.x );
    source_offset.y = FT_TS_PIX_FLOOR( source_offset_.y );
    target_offset.x = FT_TS_PIX_FLOOR( atarget_offset->x );
    target_offset.y = FT_TS_PIX_FLOOR( atarget_offset->y );

    /* get source bitmap dimensions */
    source_llx = source_offset.x;
    if ( FT_TS_LONG_MIN + (FT_TS_Pos)( source_->rows << 6 ) + 64 > source_offset.y )
    {
      FT_TS_TRACE5((
        "FT_TS_Bitmap_Blend: y coordinate overflow in source bitmap\n" ));
      return FT_TS_THROW( Invalid_Argument );
    }
    source_lly = source_offset.y - ( source_->rows << 6 );

    if ( FT_TS_LONG_MAX - (FT_TS_Pos)( source_->width << 6 ) - 64 < source_llx )
    {
      FT_TS_TRACE5((
        "FT_TS_Bitmap_Blend: x coordinate overflow in source bitmap\n" ));
      return FT_TS_THROW( Invalid_Argument );
    }
    source_urx = source_llx + ( source_->width << 6 );
    source_ury = source_offset.y;

    /* get target bitmap dimensions */
    if ( target->width && target->rows )
    {
      target_llx = target_offset.x;
      if ( FT_TS_LONG_MIN + (FT_TS_Pos)( target->rows << 6 ) > target_offset.y )
      {
        FT_TS_TRACE5((
          "FT_TS_Bitmap_Blend: y coordinate overflow in target bitmap\n" ));
        return FT_TS_THROW( Invalid_Argument );
      }
      target_lly = target_offset.y - ( target->rows << 6 );

      if ( FT_TS_LONG_MAX - (FT_TS_Pos)( target->width << 6 ) < target_llx )
      {
        FT_TS_TRACE5((
          "FT_TS_Bitmap_Blend: x coordinate overflow in target bitmap\n" ));
        return FT_TS_THROW( Invalid_Argument );
      }
      target_urx = target_llx + ( target->width << 6 );
      target_ury = target_offset.y;
    }
    else
    {
      target_llx = FT_TS_LONG_MAX;
      target_lly = FT_TS_LONG_MAX;
      target_urx = FT_TS_LONG_MIN;
      target_ury = FT_TS_LONG_MIN;
    }

    /* compute final bitmap dimensions */
    final_llx = FT_TS_MIN( source_llx, target_llx );
    final_lly = FT_TS_MIN( source_lly, target_lly );
    final_urx = FT_TS_MAX( source_urx, target_urx );
    final_ury = FT_TS_MAX( source_ury, target_ury );

    final_width = ( final_urx - final_llx ) >> 6;
    final_rows  = ( final_ury - final_lly ) >> 6;

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    FT_TS_TRACE5(( "FT_TS_Bitmap_Blend:\n" ));
    FT_TS_TRACE5(( "  source bitmap: (%ld, %ld) -- (%ld, %ld); %d x %d\n",
      source_llx / 64, source_lly / 64,
      source_urx / 64, source_ury / 64,
      source_->width, source_->rows ));

    if ( target->width && target->rows )
      FT_TS_TRACE5(( "  target bitmap: (%ld, %ld) -- (%ld, %ld); %d x %d\n",
        target_llx / 64, target_lly / 64,
        target_urx / 64, target_ury / 64,
        target->width, target->rows ));
    else
      FT_TS_TRACE5(( "  target bitmap: empty\n" ));

    if ( final_width && final_rows )
      FT_TS_TRACE5(( "  final bitmap: (%ld, %ld) -- (%ld, %ld); %d x %d\n",
        final_llx / 64, final_lly / 64,
        final_urx / 64, final_ury / 64,
        final_width, final_rows ));
    else
      FT_TS_TRACE5(( "  final bitmap: empty\n" ));
#endif /* FT_TS_DEBUG_LEVEL_TRACE */

    if ( !( final_width && final_rows ) )
      return FT_TS_Err_Ok;               /* nothing to do */

    /* for blending, set offset vector of final bitmap */
    /* temporarily to (0,0)                            */
    source_llx -= final_llx;
    source_lly -= final_lly;

    if ( target->width && target->rows )
    {
      target_llx -= final_llx;
      target_lly -= final_lly;
    }

    /* set up target bitmap */
    if ( target->pixel_mode == FT_TS_PIXEL_MODE_NONE )
    {
      /* create new empty bitmap */
      target->width      = final_width;
      target->rows       = final_rows;
      target->pixel_mode = FT_TS_PIXEL_MODE_BGRA;
      target->pitch      = (int)final_width * 4;
      target->num_grays  = 256;

      if ( FT_TS_LONG_MAX / target->pitch < (int)target->rows )
      {
        FT_TS_TRACE5(( "FT_TS_Blend_Bitmap: target bitmap too large (%d x %d)\n",
                     final_width, final_rows ));
        return FT_TS_THROW( Invalid_Argument );
      }

      if ( FT_TS_ALLOC( target->buffer, target->pitch * (int)target->rows ) )
        return error;

      free_target_bitmap_on_error = 1;
    }
    else if ( target->width != final_width ||
              target->rows  != final_rows  )
    {
      /* adjust old bitmap to enlarged size */
      int  pitch, new_pitch;

      unsigned char*  buffer = NULL;


      pitch = target->pitch;

      if ( pitch < 0 )
        pitch = -pitch;

      new_pitch = (int)final_width * 4;

      if ( FT_TS_LONG_MAX / new_pitch < (int)final_rows )
      {
        FT_TS_TRACE5(( "FT_TS_Blend_Bitmap: target bitmap too large (%d x %d)\n",
                     final_width, final_rows ));
        return FT_TS_THROW( Invalid_Argument );
      }

      /* TODO: provide an in-buffer solution for large bitmaps */
      /*       to avoid allocation of a new buffer             */
      if ( FT_TS_ALLOC( buffer, new_pitch * (int)final_rows ) )
        goto Error;

      /* copy data to new buffer */
      x = target_llx >> 6;
      y = target_lly >> 6;

      /* the bitmap flow is from top to bottom, */
      /* but y is measured from bottom to top   */
      if ( target->pitch < 0 )
      {
        /* XXX */
      }
      else
      {
        unsigned char*  p =
          target->buffer;
        unsigned char*  q =
          buffer +
          ( final_rows - y - target->rows ) * new_pitch +
          x * 4;
        unsigned char*  limit_p =
          p + pitch * (int)target->rows;


        while ( p < limit_p )
        {
          FT_TS_MEM_COPY( q, p, pitch );

          p += pitch;
          q += new_pitch;
        }
      }

      FT_TS_FREE( target->buffer );

      target->width = final_width;
      target->rows  = final_rows;

      if ( target->pitch < 0 )
        target->pitch = -new_pitch;
      else
        target->pitch = new_pitch;

      target->buffer = buffer;
    }

    /* adjust source bitmap if necessary */
    if ( source_->pixel_mode != FT_TS_PIXEL_MODE_GRAY )
    {
      FT_TS_Bitmap_Init( &source_bitmap );
      error = FT_TS_Bitmap_Convert( library, source_, &source_bitmap, 1 );
      if ( error )
        goto Error;

      source             = &source_bitmap;
      free_source_bitmap = 1;
    }
    else
      source = source_;

    /* do blending; the code below returns pre-multiplied channels, */
    /* similar to what FreeType gets from `CBDT' tables             */
    x = source_llx >> 6;
    y = source_lly >> 6;

    /* the bitmap flow is from top to bottom, */
    /* but y is measured from bottom to top   */
    if ( target->pitch < 0 )
    {
      /* XXX */
    }
    else
    {
      unsigned char*  p =
        source->buffer;
      unsigned char*  q =
        target->buffer +
        ( target->rows - y - source->rows ) * target->pitch +
        x * 4;
      unsigned char*  limit_p =
        p + source->pitch * (int)source->rows;


      while ( p < limit_p )
      {
        unsigned char*  r       = p;
        unsigned char*  s       = q;
        unsigned char*  limit_r = r + source->width;


        while ( r < limit_r )
        {
          int  aa = *r++;
          int  fa = color.alpha * aa / 255;

          int  fb = color.blue * fa / 255;
          int  fg = color.green * fa / 255;
          int  fr = color.red * fa / 255;

          int  ba2 = 255 - fa;

          int  bb = s[0];
          int  bg = s[1];
          int  br = s[2];
          int  ba = s[3];


          *s++ = (unsigned char)( bb * ba2 / 255 + fb );
          *s++ = (unsigned char)( bg * ba2 / 255 + fg );
          *s++ = (unsigned char)( br * ba2 / 255 + fr );
          *s++ = (unsigned char)( ba * ba2 / 255 + fa );
        }

        p += source->pitch;
        q += target->pitch;
      }
    }

    atarget_offset->x = final_llx;
    atarget_offset->y = final_lly + ( final_rows << 6 );

  Error:
    if ( error && free_target_bitmap_on_error )
      FT_TS_Bitmap_Done( library, target );

    if ( free_source_bitmap )
      FT_TS_Bitmap_Done( library, &source_bitmap );

    return error;
  }


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_GlyphSlot_Own_Bitmap( FT_TS_GlyphSlot  slot )
  {
    if ( slot && slot->format == FT_TS_GLYPH_FORMAT_BITMAP   &&
         !( slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP ) )
    {
      FT_TS_Bitmap  bitmap;
      FT_TS_Error   error;


      FT_TS_Bitmap_Init( &bitmap );
      error = FT_TS_Bitmap_Copy( slot->library, &slot->bitmap, &bitmap );
      if ( error )
        return error;

      slot->bitmap = bitmap;
      slot->internal->flags |= FT_TS_GLYPH_OWN_BITMAP;
    }

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftbitmap.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Bitmap_Done( FT_TS_Library  library,
                  FT_TS_Bitmap  *bitmap )
  {
    FT_TS_Memory  memory;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !bitmap )
      return FT_TS_THROW( Invalid_Argument );

    memory = library->memory;

    FT_TS_FREE( bitmap->buffer );
    *bitmap = null_bitmap;

    return FT_TS_Err_Ok;
  }


/* END */
