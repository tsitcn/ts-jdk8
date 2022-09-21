/****************************************************************************
 *
 * ftoutln.c
 *
 *   FreeType outline management (body).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/ftoutln.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftcalc.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/fttrigon.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  outline


  static
  const FT_TS_Outline  null_outline = { 0, 0, NULL, NULL, NULL, 0 };


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Decompose( FT_TS_Outline*              outline,
                        const FT_TS_Outline_Funcs*  func_interface,
                        void*                    user )
  {
#undef  SCALED
#define SCALED( x )  ( (x) * ( 1L << shift ) - delta )

    FT_TS_Vector   v_last;
    FT_TS_Vector   v_control;
    FT_TS_Vector   v_start;

    FT_TS_Vector*  point;
    FT_TS_Vector*  limit;
    char*       tags;

    FT_TS_Error    error;

    FT_TS_Int   n;         /* index of contour in outline     */
    FT_TS_UInt  first;     /* index of first point in contour */
    FT_TS_Int   tag;       /* current point's state           */

    FT_TS_Int   shift;
    FT_TS_Pos   delta;


    if ( !outline )
      return FT_TS_THROW( Invalid_Outline );

    if ( !func_interface )
      return FT_TS_THROW( Invalid_Argument );

    shift = func_interface->shift;
    delta = func_interface->delta;
    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      FT_TS_Int  last;  /* index of last point in contour */


      FT_TS_TRACE5(( "FT_TS_Outline_Decompose: Outline %d\n", n ));

      last = outline->contours[n];
      if ( last < 0 )
        goto Invalid_Outline;
      limit = outline->points + last;

      v_start   = outline->points[first];
      v_start.x = SCALED( v_start.x );
      v_start.y = SCALED( v_start.y );

      v_last   = outline->points[last];
      v_last.x = SCALED( v_last.x );
      v_last.y = SCALED( v_last.y );

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags   + first;
      tag   = FT_TS_CURVE_TAG( tags[0] );

      /* A contour cannot start with a cubic control point! */
      if ( tag == FT_TS_CURVE_TAG_CUBIC )
        goto Invalid_Outline;

      /* check first point to determine origin */
      if ( tag == FT_TS_CURVE_TAG_CONIC )
      {
        /* first point is conic control.  Yes, this happens. */
        if ( FT_TS_CURVE_TAG( outline->tags[last] ) == FT_TS_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic,         */
          /* start at their middle and record its position    */
          /* for closure                                      */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;

       /* v_last = v_start; */
        }
        point--;
        tags--;
      }

      FT_TS_TRACE5(( "  move to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->move_to( &v_start, user );
      if ( error )
        goto Exit;

      while ( point < limit )
      {
        point++;
        tags++;

        tag = FT_TS_CURVE_TAG( tags[0] );
        switch ( tag )
        {
        case FT_TS_CURVE_TAG_ON:  /* emit a single line_to */
          {
            FT_TS_Vector  vec;


            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            FT_TS_TRACE5(( "  line to (%.2f, %.2f)\n",
                        vec.x / 64.0, vec.y / 64.0 ));
            error = func_interface->line_to( &vec, user );
            if ( error )
              goto Exit;
            continue;
          }

        case FT_TS_CURVE_TAG_CONIC:  /* consume conic arcs */
          v_control.x = SCALED( point->x );
          v_control.y = SCALED( point->y );

        Do_Conic:
          if ( point < limit )
          {
            FT_TS_Vector  vec;
            FT_TS_Vector  v_middle;


            point++;
            tags++;
            tag = FT_TS_CURVE_TAG( tags[0] );

            vec.x = SCALED( point->x );
            vec.y = SCALED( point->y );

            if ( tag == FT_TS_CURVE_TAG_ON )
            {
              FT_TS_TRACE5(( "  conic to (%.2f, %.2f)"
                          " with control (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          v_control.x / 64.0, v_control.y / 64.0 ));
              error = func_interface->conic_to( &v_control, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            if ( tag != FT_TS_CURVE_TAG_CONIC )
              goto Invalid_Outline;

            v_middle.x = ( v_control.x + vec.x ) / 2;
            v_middle.y = ( v_control.y + vec.y ) / 2;

            FT_TS_TRACE5(( "  conic to (%.2f, %.2f)"
                        " with control (%.2f, %.2f)\n",
                        v_middle.x / 64.0, v_middle.y / 64.0,
                        v_control.x / 64.0, v_control.y / 64.0 ));
            error = func_interface->conic_to( &v_control, &v_middle, user );
            if ( error )
              goto Exit;

            v_control = vec;
            goto Do_Conic;
          }

          FT_TS_TRACE5(( "  conic to (%.2f, %.2f)"
                      " with control (%.2f, %.2f)\n",
                      v_start.x / 64.0, v_start.y / 64.0,
                      v_control.x / 64.0, v_control.y / 64.0 ));
          error = func_interface->conic_to( &v_control, &v_start, user );
          goto Close;

        default:  /* FT_TS_CURVE_TAG_CUBIC */
          {
            FT_TS_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 FT_TS_CURVE_TAG( tags[1] ) != FT_TS_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1.x = SCALED( point[-2].x );
            vec1.y = SCALED( point[-2].y );

            vec2.x = SCALED( point[-1].x );
            vec2.y = SCALED( point[-1].y );

            if ( point <= limit )
            {
              FT_TS_Vector  vec;


              vec.x = SCALED( point->x );
              vec.y = SCALED( point->y );

              FT_TS_TRACE5(( "  cubic to (%.2f, %.2f)"
                          " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                          vec.x / 64.0, vec.y / 64.0,
                          vec1.x / 64.0, vec1.y / 64.0,
                          vec2.x / 64.0, vec2.y / 64.0 ));
              error = func_interface->cubic_to( &vec1, &vec2, &vec, user );
              if ( error )
                goto Exit;
              continue;
            }

            FT_TS_TRACE5(( "  cubic to (%.2f, %.2f)"
                        " with controls (%.2f, %.2f) and (%.2f, %.2f)\n",
                        v_start.x / 64.0, v_start.y / 64.0,
                        vec1.x / 64.0, vec1.y / 64.0,
                        vec2.x / 64.0, vec2.y / 64.0 ));
            error = func_interface->cubic_to( &vec1, &vec2, &v_start, user );
            goto Close;
          }
        }
      }

      /* close the contour with a line segment */
      FT_TS_TRACE5(( "  line to (%.2f, %.2f)\n",
                  v_start.x / 64.0, v_start.y / 64.0 ));
      error = func_interface->line_to( &v_start, user );

    Close:
      if ( error )
        goto Exit;

      first = (FT_TS_UInt)last + 1;
    }

    FT_TS_TRACE5(( "FT_TS_Outline_Decompose: Done\n" ));
    return FT_TS_Err_Ok;

  Invalid_Outline:
    error = FT_TS_THROW( Invalid_Outline );
    /* fall through */

  Exit:
    FT_TS_TRACE5(( "FT_TS_Outline_Decompose: Error 0x%x\n", error ));
    return error;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_New( FT_TS_Library   library,
                  FT_TS_UInt      numPoints,
                  FT_TS_Int       numContours,
                  FT_TS_Outline  *anoutline )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    memory = library->memory;

    if ( !anoutline || !memory )
      return FT_TS_THROW( Invalid_Argument );

    *anoutline = null_outline;

    if ( numContours < 0                  ||
         (FT_TS_UInt)numContours > numPoints )
      return FT_TS_THROW( Invalid_Argument );

    if ( numPoints > FT_TS_OUTLINE_POINTS_MAX )
      return FT_TS_THROW( Array_Too_Large );

    if ( FT_TS_NEW_ARRAY( anoutline->points,   numPoints   ) ||
         FT_TS_NEW_ARRAY( anoutline->tags,     numPoints   ) ||
         FT_TS_NEW_ARRAY( anoutline->contours, numContours ) )
      goto Fail;

    anoutline->n_points    = (FT_TS_Short)numPoints;
    anoutline->n_contours  = (FT_TS_Short)numContours;
    anoutline->flags      |= FT_TS_OUTLINE_OWNER;

    return FT_TS_Err_Ok;

  Fail:
    anoutline->flags |= FT_TS_OUTLINE_OWNER;
    FT_TS_Outline_Done( library, anoutline );

    return error;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Check( FT_TS_Outline*  outline )
  {
    if ( outline )
    {
      FT_TS_Int  n_points   = outline->n_points;
      FT_TS_Int  n_contours = outline->n_contours;
      FT_TS_Int  end0, end;
      FT_TS_Int  n;


      /* empty glyph? */
      if ( n_points == 0 && n_contours == 0 )
        return FT_TS_Err_Ok;

      /* check point and contour counts */
      if ( n_points <= 0 || n_contours <= 0 )
        goto Bad;

      end0 = end = -1;
      for ( n = 0; n < n_contours; n++ )
      {
        end = outline->contours[n];

        /* note that we don't accept empty contours */
        if ( end <= end0 || end >= n_points )
          goto Bad;

        end0 = end;
      }

      if ( end != n_points - 1 )
        goto Bad;

      /* XXX: check the tags array */
      return FT_TS_Err_Ok;
    }

  Bad:
    return FT_TS_THROW( Invalid_Argument );
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Copy( const FT_TS_Outline*  source,
                   FT_TS_Outline        *target )
  {
    FT_TS_Int  is_owner;


    if ( !source || !target )
      return FT_TS_THROW( Invalid_Outline );

    if ( source->n_points   != target->n_points   ||
         source->n_contours != target->n_contours )
      return FT_TS_THROW( Invalid_Argument );

    if ( source == target )
      return FT_TS_Err_Ok;

    if ( source->n_points )
    {
      FT_TS_ARRAY_COPY( target->points, source->points, source->n_points );
      FT_TS_ARRAY_COPY( target->tags,   source->tags,   source->n_points );
    }

    if ( source->n_contours )
      FT_TS_ARRAY_COPY( target->contours, source->contours, source->n_contours );

    /* copy all flags, except the `FT_TS_OUTLINE_OWNER' one */
    is_owner      = target->flags & FT_TS_OUTLINE_OWNER;
    target->flags = source->flags;

    target->flags &= ~FT_TS_OUTLINE_OWNER;
    target->flags |= is_owner;

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Done( FT_TS_Library   library,
                   FT_TS_Outline*  outline )
  {
    FT_TS_Memory  memory;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !outline )
      return FT_TS_THROW( Invalid_Outline );

    memory = library->memory;

    if ( !memory )
      return FT_TS_THROW( Invalid_Argument );

    if ( outline->flags & FT_TS_OUTLINE_OWNER )
    {
      FT_TS_FREE( outline->points   );
      FT_TS_FREE( outline->tags     );
      FT_TS_FREE( outline->contours );
    }
    *outline = null_outline;

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Outline_Get_CBox( const FT_TS_Outline*  outline,
                       FT_TS_BBox           *acbox )
  {
    FT_TS_Pos  xMin, yMin, xMax, yMax;


    if ( outline && acbox )
    {
      if ( outline->n_points == 0 )
      {
        xMin = 0;
        yMin = 0;
        xMax = 0;
        yMax = 0;
      }
      else
      {
        FT_TS_Vector*  vec   = outline->points;
        FT_TS_Vector*  limit = vec + outline->n_points;


        xMin = xMax = vec->x;
        yMin = yMax = vec->y;
        vec++;

        for ( ; vec < limit; vec++ )
        {
          FT_TS_Pos  x, y;


          x = vec->x;
          if ( x < xMin ) xMin = x;
          if ( x > xMax ) xMax = x;

          y = vec->y;
          if ( y < yMin ) yMin = y;
          if ( y > yMax ) yMax = y;
        }
      }
      acbox->xMin = xMin;
      acbox->xMax = xMax;
      acbox->yMin = yMin;
      acbox->yMax = yMax;
    }
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Outline_Translate( const FT_TS_Outline*  outline,
                        FT_TS_Pos             xOffset,
                        FT_TS_Pos             yOffset )
  {
    FT_TS_UShort   n;
    FT_TS_Vector*  vec;


    if ( !outline )
      return;

    vec = outline->points;

    for ( n = 0; n < outline->n_points; n++ )
    {
      vec->x = ADD_LONG( vec->x, xOffset );
      vec->y = ADD_LONG( vec->y, yOffset );
      vec++;
    }
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Outline_Reverse( FT_TS_Outline*  outline )
  {
    FT_TS_UShort  n;
    FT_TS_Int     first, last;


    if ( !outline )
      return;

    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      last  = outline->contours[n];

      /* reverse point table */
      {
        FT_TS_Vector*  p = outline->points + first;
        FT_TS_Vector*  q = outline->points + last;
        FT_TS_Vector   swap;


        while ( p < q )
        {
          swap = *p;
          *p   = *q;
          *q   = swap;
          p++;
          q--;
        }
      }

      /* reverse tags table */
      {
        char*  p = outline->tags + first;
        char*  q = outline->tags + last;


        while ( p < q )
        {
          char  swap;


          swap = *p;
          *p   = *q;
          *q   = swap;
          p++;
          q--;
        }
      }

      first = last + 1;
    }

    outline->flags ^= FT_TS_OUTLINE_REVERSE_FILL;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Render( FT_TS_Library         library,
                     FT_TS_Outline*        outline,
                     FT_TS_Raster_Params*  params )
  {
    FT_TS_Error     error;
    FT_TS_Renderer  renderer;
    FT_TS_ListNode  node;
    FT_TS_BBox      cbox;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !outline )
      return FT_TS_THROW( Invalid_Outline );

    if ( !params )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_Outline_Get_CBox( outline, &cbox );
    if ( cbox.xMin < -0x1000000L || cbox.yMin < -0x1000000L ||
         cbox.xMax >  0x1000000L || cbox.yMax >  0x1000000L )
      return FT_TS_THROW( Invalid_Outline );

    renderer = library->cur_renderer;
    node     = library->renderers.head;

    params->source = (void*)outline;

    /* preset clip_box for direct mode */
    if ( params->flags & FT_TS_RASTER_FLAG_DIRECT    &&
         !( params->flags & FT_TS_RASTER_FLAG_CLIP ) )
    {
      params->clip_box.xMin = cbox.xMin >> 6;
      params->clip_box.yMin = cbox.yMin >> 6;
      params->clip_box.xMax = ( cbox.xMax + 63 ) >> 6;
      params->clip_box.yMax = ( cbox.yMax + 63 ) >> 6;
    }

    error = FT_TS_ERR( Cannot_Render_Glyph );
    while ( renderer )
    {
      error = renderer->raster_render( renderer->raster, params );
      if ( !error || FT_TS_ERR_NEQ( error, Cannot_Render_Glyph ) )
        break;

      /* FT_TS_Err_Cannot_Render_Glyph is returned if the render mode   */
      /* is unsupported by the current renderer for this glyph image */
      /* format                                                      */

      /* now, look for another renderer that supports the same */
      /* format                                                */
      renderer = FT_TS_Lookup_Renderer( library, FT_TS_GLYPH_FORMAT_OUTLINE,
                                     &node );
    }

    return error;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Get_Bitmap( FT_TS_Library        library,
                         FT_TS_Outline*       outline,
                         const FT_TS_Bitmap  *abitmap )
  {
    FT_TS_Raster_Params  params;


    if ( !abitmap )
      return FT_TS_THROW( Invalid_Argument );

    /* other checks are delayed to `FT_TS_Outline_Render' */

    params.target = abitmap;
    params.flags  = 0;

    if ( abitmap->pixel_mode == FT_TS_PIXEL_MODE_GRAY  ||
         abitmap->pixel_mode == FT_TS_PIXEL_MODE_LCD   ||
         abitmap->pixel_mode == FT_TS_PIXEL_MODE_LCD_V )
      params.flags |= FT_TS_RASTER_FLAG_AA;

    return FT_TS_Outline_Render( library, outline, &params );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Vector_Transform( FT_TS_Vector*        vector,
                       const FT_TS_Matrix*  matrix )
  {
    FT_TS_Pos  xz, yz;


    if ( !vector || !matrix )
      return;

    xz = FT_TS_MulFix( vector->x, matrix->xx ) +
         FT_TS_MulFix( vector->y, matrix->xy );

    yz = FT_TS_MulFix( vector->x, matrix->yx ) +
         FT_TS_MulFix( vector->y, matrix->yy );

    vector->x = xz;
    vector->y = yz;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Outline_Transform( const FT_TS_Outline*  outline,
                        const FT_TS_Matrix*   matrix )
  {
    FT_TS_Vector*  vec;
    FT_TS_Vector*  limit;


    if ( !outline || !matrix || !outline->points )
      return;

    vec   = outline->points;
    limit = vec + outline->n_points;

    for ( ; vec < limit; vec++ )
      FT_TS_Vector_Transform( vec, matrix );
  }


#if 0

#define FT_TS_OUTLINE_GET_CONTOUR( outline, c, first, last )  \
  do                                                       \
  {                                                        \
    (first) = ( c > 0 ) ? (outline)->points +              \
                            (outline)->contours[c - 1] + 1 \
                        : (outline)->points;               \
    (last) = (outline)->points + (outline)->contours[c];   \
  } while ( 0 )


  /* Is a point in some contour?                     */
  /*                                                 */
  /* We treat every point of the contour as if it    */
  /* it were ON.  That is, we allow false positives, */
  /* but disallow false negatives.  (XXX really?)    */
  static FT_TS_Bool
  ft_contour_has( FT_TS_Outline*  outline,
                  FT_TS_Short     c,
                  FT_TS_Vector*   point )
  {
    FT_TS_Vector*  first;
    FT_TS_Vector*  last;
    FT_TS_Vector*  a;
    FT_TS_Vector*  b;
    FT_TS_UInt     n = 0;


    FT_TS_OUTLINE_GET_CONTOUR( outline, c, first, last );

    for ( a = first; a <= last; a++ )
    {
      FT_TS_Pos  x;
      FT_TS_Int  intersect;


      b = ( a == last ) ? first : a + 1;

      intersect = ( a->y - point->y ) ^ ( b->y - point->y );

      /* a and b are on the same side */
      if ( intersect >= 0 )
      {
        if ( intersect == 0 && a->y == point->y )
        {
          if ( ( a->x <= point->x && b->x >= point->x ) ||
               ( a->x >= point->x && b->x <= point->x ) )
            return 1;
        }

        continue;
      }

      x = a->x + ( b->x - a->x ) * (point->y - a->y ) / ( b->y - a->y );

      if ( x < point->x )
        n++;
      else if ( x == point->x )
        return 1;
    }

    return n & 1;
  }


  static FT_TS_Bool
  ft_contour_enclosed( FT_TS_Outline*  outline,
                       FT_TS_UShort    c )
  {
    FT_TS_Vector*  first;
    FT_TS_Vector*  last;
    FT_TS_Short    i;


    FT_TS_OUTLINE_GET_CONTOUR( outline, c, first, last );

    for ( i = 0; i < outline->n_contours; i++ )
    {
      if ( i != c && ft_contour_has( outline, i, first ) )
      {
        FT_TS_Vector*  pt;


        for ( pt = first + 1; pt <= last; pt++ )
          if ( !ft_contour_has( outline, i, pt ) )
            return 0;

        return 1;
      }
    }

    return 0;
  }


  /* This version differs from the public one in that each */
  /* part (contour not enclosed in another contour) of the */
  /* outline is checked for orientation.  This is          */
  /* necessary for some buggy CJK fonts.                   */
  static FT_TS_Orientation
  ft_outline_get_orientation( FT_TS_Outline*  outline )
  {
    FT_TS_Short        i;
    FT_TS_Vector*      first;
    FT_TS_Vector*      last;
    FT_TS_Orientation  orient = FT_TS_ORIENTATION_NONE;


    first = outline->points;
    for ( i = 0; i < outline->n_contours; i++, first = last + 1 )
    {
      FT_TS_Vector*  point;
      FT_TS_Vector*  xmin_point;
      FT_TS_Pos      xmin;


      last = outline->points + outline->contours[i];

      /* skip degenerate contours */
      if ( last < first + 2 )
        continue;

      if ( ft_contour_enclosed( outline, i ) )
        continue;

      xmin       = first->x;
      xmin_point = first;

      for ( point = first + 1; point <= last; point++ )
      {
        if ( point->x < xmin )
        {
          xmin       = point->x;
          xmin_point = point;
        }
      }

      /* check the orientation of the contour */
      {
        FT_TS_Vector*      prev;
        FT_TS_Vector*      next;
        FT_TS_Orientation  o;


        prev = ( xmin_point == first ) ? last : xmin_point - 1;
        next = ( xmin_point == last ) ? first : xmin_point + 1;

        if ( FT_TS_Atan2( prev->x - xmin_point->x, prev->y - xmin_point->y ) >
             FT_TS_Atan2( next->x - xmin_point->x, next->y - xmin_point->y ) )
          o = FT_TS_ORIENTATION_POSTSCRIPT;
        else
          o = FT_TS_ORIENTATION_TRUETYPE;

        if ( orient == FT_TS_ORIENTATION_NONE )
          orient = o;
        else if ( orient != o )
          return FT_TS_ORIENTATION_NONE;
      }
    }

    return orient;
  }

#endif /* 0 */


  /* documentation is in ftoutln.h */

/**
 TSIT {{{{{{{{{{
 */
 
  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_Embolden( FT_TS_Outline*  outline,
                       FT_TS_Pos       strength )
  {
    return FT_TS_Outline_WeightXY( outline, strength, strength );
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Outline_EmboldenXY( FT_TS_Outline*  outline,
                         FT_TS_Pos       xstrength,
                         FT_TS_Pos       ystrength )
  {
    /* check value ? */
  	return FT_TS_Outline_WeightXY( outline, xstrength, ystrength );
  }
  
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Outline_WeightXY( FT_TS_Outline*  outline,
                          FT_TS_Pos       xstrength,
                          FT_TS_Pos       ystrength )
  {
/**
 TSIT }}}}}}}}}}
 */
    FT_TS_Vector*      points;
    FT_TS_Int          c, first, last;
    FT_TS_Orientation  orientation;


    if ( !outline )
      return FT_TS_THROW( Invalid_Outline );

    xstrength /= 2;
    ystrength /= 2;
    if ( xstrength == 0 && ystrength == 0 )
      return FT_TS_Err_Ok;

    orientation = FT_TS_Outline_Get_Orientation( outline );
    if ( orientation == FT_TS_ORIENTATION_NONE )
    {
      if ( outline->n_contours )
        return FT_TS_THROW( Invalid_Argument );
      else
        return FT_TS_Err_Ok;
    }

    points = outline->points;

    first = 0;
    for ( c = 0; c < outline->n_contours; c++ )
    {
      FT_TS_Vector  in, out, anchor, shift;
      FT_TS_Fixed   l_in, l_out, l_anchor = 0, l, q, d;
      FT_TS_Int     i, j, k;


      l_in = 0;
      last = outline->contours[c];

      /* pacify compiler */
      in.x = in.y = anchor.x = anchor.y = 0;

      /* Counter j cycles though the points; counter i advances only  */
      /* when points are moved; anchor k marks the first moved point. */
      for ( i = last, j = first, k = -1;
            j != i && i != k;
            j = j < last ? j + 1 : first )
      {
        if ( j != k )
        {
          out.x = points[j].x - points[i].x;
          out.y = points[j].y - points[i].y;
          l_out = (FT_TS_Fixed)FT_TS_Vector_NormLen( &out );

          if ( l_out == 0 )
            continue;
        }
        else
        {
          out   = anchor;
          l_out = l_anchor;
        }

        if ( l_in != 0 )
        {
          if ( k < 0 )
          {
            k        = i;
            anchor   = in;
            l_anchor = l_in;
          }

          d = FT_TS_MulFix( in.x, out.x ) + FT_TS_MulFix( in.y, out.y );

          /* shift only if turn is less than ~160 degrees */
          if ( d > -0xF000L )
          {
            d = d + 0x10000L;

            /* shift components along lateral bisector in proper orientation */
            shift.x = in.y + out.y;
            shift.y = in.x + out.x;

            if ( orientation == FT_TS_ORIENTATION_TRUETYPE )
              shift.x = -shift.x;
            else
              shift.y = -shift.y;

            /* restrict shift magnitude to better handle collapsing segments */
            q = FT_TS_MulFix( out.x, in.y ) - FT_TS_MulFix( out.y, in.x );
            if ( orientation == FT_TS_ORIENTATION_TRUETYPE )
              q = -q;

            l = FT_TS_MIN( l_in, l_out );

            /* non-strict inequalities avoid divide-by-zero when q == l == 0 */
            if ( FT_TS_MulFix( xstrength, q ) <= FT_TS_MulFix( l, d ) )
              shift.x = FT_TS_MulDiv( shift.x, xstrength, d );
            else
              shift.x = FT_TS_MulDiv( shift.x, l, q );


            if ( FT_TS_MulFix( ystrength, q ) <= FT_TS_MulFix( l, d ) )
              shift.y = FT_TS_MulDiv( shift.y, ystrength, d );
            else
              shift.y = FT_TS_MulDiv( shift.y, l, q );
          }
          else
            shift.x = shift.y = 0;

          for ( ;
                i != j;
                i = i < last ? i + 1 : first )
          {
            points[i].x += xstrength + shift.x;
            points[i].y += ystrength + shift.y;
          }
        }
        else
          i = j;

        in   = out;
        l_in = l_out;
      }

      first = last + 1;
    }

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftoutln.h */

  FT_TS_EXPORT_DEF( FT_TS_Orientation )
  FT_TS_Outline_Get_Orientation( FT_TS_Outline*  outline )
  {
    FT_TS_BBox     cbox = { 0, 0, 0, 0 };
    FT_TS_Int      xshift, yshift;
    FT_TS_Vector*  points;
    FT_TS_Vector   v_prev, v_cur;
    FT_TS_Int      c, n, first;
    FT_TS_Pos      area = 0;


    if ( !outline || outline->n_points <= 0 )
      return FT_TS_ORIENTATION_TRUETYPE;

    /* We use the nonzero winding rule to find the orientation.       */
    /* Since glyph outlines behave much more `regular' than arbitrary */
    /* cubic or quadratic curves, this test deals with the polygon    */
    /* only that is spanned up by the control points.                 */

    FT_TS_Outline_Get_CBox( outline, &cbox );

    /* Handle collapsed outlines to avoid undefined FT_TS_MSB. */
    if ( cbox.xMin == cbox.xMax || cbox.yMin == cbox.yMax )
      return FT_TS_ORIENTATION_NONE;

    /* Reject values large outlines. */
    if ( cbox.xMin < -0x1000000L || cbox.yMin < -0x1000000L ||
         cbox.xMax >  0x1000000L || cbox.yMax >  0x1000000L )
      return FT_TS_ORIENTATION_NONE;

    xshift = FT_TS_MSB( (FT_TS_UInt32)( FT_TS_ABS( cbox.xMax ) |
                                  FT_TS_ABS( cbox.xMin ) ) ) - 14;
    xshift = FT_TS_MAX( xshift, 0 );

    yshift = FT_TS_MSB( (FT_TS_UInt32)( cbox.yMax - cbox.yMin ) ) - 14;
    yshift = FT_TS_MAX( yshift, 0 );

    points = outline->points;

    first = 0;
    for ( c = 0; c < outline->n_contours; c++ )
    {
      FT_TS_Int  last = outline->contours[c];


      v_prev.x = points[last].x >> xshift;
      v_prev.y = points[last].y >> yshift;

      for ( n = first; n <= last; n++ )
      {
        v_cur.x = points[n].x >> xshift;
        v_cur.y = points[n].y >> yshift;

        area = ADD_LONG( area,
                         MUL_LONG( v_cur.y - v_prev.y,
                                   v_cur.x + v_prev.x ) );

        v_prev = v_cur;
      }

      first = last + 1;
    }

    if ( area > 0 )
      return FT_TS_ORIENTATION_POSTSCRIPT;
    else if ( area < 0 )
      return FT_TS_ORIENTATION_TRUETYPE;
    else
      return FT_TS_ORIENTATION_NONE;
  }


/* END */
