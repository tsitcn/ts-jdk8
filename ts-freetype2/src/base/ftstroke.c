/****************************************************************************
 *
 * ftstroke.c
 *
 *   FreeType path stroker (body).
 *
 * Copyright (C) 2002-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/ftstroke.h>
#include <freetype/fttrigon.h>
#include <freetype/ftoutln.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>


  /* declare an extern to access `ft_outline_glyph_class' globally */
  /* allocated  in `ftglyph.c'                                     */
  FT_TS_CALLBACK_TABLE const FT_TS_Glyph_Class  ft_outline_glyph_class;


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_StrokerBorder )
  FT_TS_Outline_GetInsideBorder( FT_TS_Outline*  outline )
  {
    FT_TS_Orientation  o = FT_TS_Outline_Get_Orientation( outline );


    return o == FT_TS_ORIENTATION_TRUETYPE ? FT_TS_STROKER_BORDER_RIGHT
                                        : FT_TS_STROKER_BORDER_LEFT;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_StrokerBorder )
  FT_TS_Outline_GetOutsideBorder( FT_TS_Outline*  outline )
  {
    FT_TS_Orientation  o = FT_TS_Outline_Get_Orientation( outline );


    return o == FT_TS_ORIENTATION_TRUETYPE ? FT_TS_STROKER_BORDER_LEFT
                                        : FT_TS_STROKER_BORDER_RIGHT;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      BEZIER COMPUTATIONS                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define FT_TS_SMALL_CONIC_THRESHOLD  ( FT_TS_ANGLE_PI / 6 )
#define FT_TS_SMALL_CUBIC_THRESHOLD  ( FT_TS_ANGLE_PI / 8 )

#define FT_TS_EPSILON  2

#define FT_TS_IS_SMALL( x )  ( (x) > -FT_TS_EPSILON && (x) < FT_TS_EPSILON )


  static FT_TS_Pos
  ft_pos_abs( FT_TS_Pos  x )
  {
    return x >= 0 ? x : -x;
  }


  static void
  ft_conic_split( FT_TS_Vector*  base )
  {
    FT_TS_Pos  a, b;


    base[4].x = base[2].x;
    a = base[0].x + base[1].x;
    b = base[1].x + base[2].x;
    base[3].x = b >> 1;
    base[2].x = ( a + b ) >> 2;
    base[1].x = a >> 1;

    base[4].y = base[2].y;
    a = base[0].y + base[1].y;
    b = base[1].y + base[2].y;
    base[3].y = b >> 1;
    base[2].y = ( a + b ) >> 2;
    base[1].y = a >> 1;
  }


  static FT_TS_Bool
  ft_conic_is_small_enough( FT_TS_Vector*  base,
                            FT_TS_Angle   *angle_in,
                            FT_TS_Angle   *angle_out )
  {
    FT_TS_Vector  d1, d2;
    FT_TS_Angle   theta;
    FT_TS_Int     close1, close2;


    d1.x = base[1].x - base[2].x;
    d1.y = base[1].y - base[2].y;
    d2.x = base[0].x - base[1].x;
    d2.y = base[0].y - base[1].y;

    close1 = FT_TS_IS_SMALL( d1.x ) && FT_TS_IS_SMALL( d1.y );
    close2 = FT_TS_IS_SMALL( d2.x ) && FT_TS_IS_SMALL( d2.y );

    if ( close1 )
    {
      if ( close2 )
      {
        /* basically a point;                      */
        /* do nothing to retain original direction */
      }
      else
      {
        *angle_in  =
        *angle_out = FT_TS_Atan2( d2.x, d2.y );
      }
    }
    else /* !close1 */
    {
      if ( close2 )
      {
        *angle_in  =
        *angle_out = FT_TS_Atan2( d1.x, d1.y );
      }
      else
      {
        *angle_in  = FT_TS_Atan2( d1.x, d1.y );
        *angle_out = FT_TS_Atan2( d2.x, d2.y );
      }
    }

    theta = ft_pos_abs( FT_TS_Angle_Diff( *angle_in, *angle_out ) );

    return FT_TS_BOOL( theta < FT_TS_SMALL_CONIC_THRESHOLD );
  }


  static void
  ft_cubic_split( FT_TS_Vector*  base )
  {
    FT_TS_Pos  a, b, c;


    base[6].x = base[3].x;
    a = base[0].x + base[1].x;
    b = base[1].x + base[2].x;
    c = base[2].x + base[3].x;
    base[5].x = c >> 1;
    c += b;
    base[4].x = c >> 2;
    base[1].x = a >> 1;
    a += b;
    base[2].x = a >> 2;
    base[3].x = ( a + c ) >> 3;

    base[6].y = base[3].y;
    a = base[0].y + base[1].y;
    b = base[1].y + base[2].y;
    c = base[2].y + base[3].y;
    base[5].y = c >> 1;
    c += b;
    base[4].y = c >> 2;
    base[1].y = a >> 1;
    a += b;
    base[2].y = a >> 2;
    base[3].y = ( a + c ) >> 3;
  }


  /* Return the average of `angle1' and `angle2'.            */
  /* This gives correct result even if `angle1' and `angle2' */
  /* have opposite signs.                                    */
  static FT_TS_Angle
  ft_angle_mean( FT_TS_Angle  angle1,
                 FT_TS_Angle  angle2 )
  {
    return angle1 + FT_TS_Angle_Diff( angle1, angle2 ) / 2;
  }


  static FT_TS_Bool
  ft_cubic_is_small_enough( FT_TS_Vector*  base,
                            FT_TS_Angle   *angle_in,
                            FT_TS_Angle   *angle_mid,
                            FT_TS_Angle   *angle_out )
  {
    FT_TS_Vector  d1, d2, d3;
    FT_TS_Angle   theta1, theta2;
    FT_TS_Int     close1, close2, close3;


    d1.x = base[2].x - base[3].x;
    d1.y = base[2].y - base[3].y;
    d2.x = base[1].x - base[2].x;
    d2.y = base[1].y - base[2].y;
    d3.x = base[0].x - base[1].x;
    d3.y = base[0].y - base[1].y;

    close1 = FT_TS_IS_SMALL( d1.x ) && FT_TS_IS_SMALL( d1.y );
    close2 = FT_TS_IS_SMALL( d2.x ) && FT_TS_IS_SMALL( d2.y );
    close3 = FT_TS_IS_SMALL( d3.x ) && FT_TS_IS_SMALL( d3.y );

    if ( close1 )
    {
      if ( close2 )
      {
        if ( close3 )
        {
          /* basically a point;                      */
          /* do nothing to retain original direction */
        }
        else /* !close3 */
        {
          *angle_in  =
          *angle_mid =
          *angle_out = FT_TS_Atan2( d3.x, d3.y );
        }
      }
      else /* !close2 */
      {
        if ( close3 )
        {
          *angle_in  =
          *angle_mid =
          *angle_out = FT_TS_Atan2( d2.x, d2.y );
        }
        else /* !close3 */
        {
          *angle_in  =
          *angle_mid = FT_TS_Atan2( d2.x, d2.y );
          *angle_out = FT_TS_Atan2( d3.x, d3.y );
        }
      }
    }
    else /* !close1 */
    {
      if ( close2 )
      {
        if ( close3 )
        {
          *angle_in  =
          *angle_mid =
          *angle_out = FT_TS_Atan2( d1.x, d1.y );
        }
        else /* !close3 */
        {
          *angle_in  = FT_TS_Atan2( d1.x, d1.y );
          *angle_out = FT_TS_Atan2( d3.x, d3.y );
          *angle_mid = ft_angle_mean( *angle_in, *angle_out );
        }
      }
      else /* !close2 */
      {
        if ( close3 )
        {
          *angle_in  = FT_TS_Atan2( d1.x, d1.y );
          *angle_mid =
          *angle_out = FT_TS_Atan2( d2.x, d2.y );
        }
        else /* !close3 */
        {
          *angle_in  = FT_TS_Atan2( d1.x, d1.y );
          *angle_mid = FT_TS_Atan2( d2.x, d2.y );
          *angle_out = FT_TS_Atan2( d3.x, d3.y );
        }
      }
    }

    theta1 = ft_pos_abs( FT_TS_Angle_Diff( *angle_in,  *angle_mid ) );
    theta2 = ft_pos_abs( FT_TS_Angle_Diff( *angle_mid, *angle_out ) );

    return FT_TS_BOOL( theta1 < FT_TS_SMALL_CUBIC_THRESHOLD &&
                    theta2 < FT_TS_SMALL_CUBIC_THRESHOLD );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       STROKE BORDERS                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef enum  FT_TS_StrokeTags_
  {
    FT_TS_STROKE_TAG_ON    = 1,   /* on-curve point  */
    FT_TS_STROKE_TAG_CUBIC = 2,   /* cubic off-point */
    FT_TS_STROKE_TAG_BEGIN = 4,   /* sub-path start  */
    FT_TS_STROKE_TAG_END   = 8    /* sub-path end    */

  } FT_TS_StrokeTags;

#define  FT_TS_STROKE_TAG_BEGIN_END  ( FT_TS_STROKE_TAG_BEGIN | FT_TS_STROKE_TAG_END )

  typedef struct  FT_TS_StrokeBorderRec_
  {
    FT_TS_UInt     num_points;
    FT_TS_UInt     max_points;
    FT_TS_Vector*  points;
    FT_TS_Byte*    tags;
    FT_TS_Bool     movable;  /* TRUE for ends of lineto borders */
    FT_TS_Int      start;    /* index of current sub-path start point */
    FT_TS_Memory   memory;
    FT_TS_Bool     valid;

  } FT_TS_StrokeBorderRec, *FT_TS_StrokeBorder;


  static FT_TS_Error
  ft_stroke_border_grow( FT_TS_StrokeBorder  border,
                         FT_TS_UInt          new_points )
  {
    FT_TS_UInt   old_max = border->max_points;
    FT_TS_UInt   new_max = border->num_points + new_points;
    FT_TS_Error  error   = FT_TS_Err_Ok;


    if ( new_max > old_max )
    {
      FT_TS_UInt    cur_max = old_max;
      FT_TS_Memory  memory  = border->memory;


      while ( cur_max < new_max )
        cur_max += ( cur_max >> 1 ) + 16;

      if ( FT_TS_RENEW_ARRAY( border->points, old_max, cur_max ) ||
           FT_TS_RENEW_ARRAY( border->tags,   old_max, cur_max ) )
        goto Exit;

      border->max_points = cur_max;
    }

  Exit:
    return error;
  }


  static void
  ft_stroke_border_close( FT_TS_StrokeBorder  border,
                          FT_TS_Bool          reverse )
  {
    FT_TS_UInt  start = (FT_TS_UInt)border->start;
    FT_TS_UInt  count = border->num_points;


    FT_TS_ASSERT( border->start >= 0 );

    /* don't record empty paths! */
    if ( count <= start + 1U )
      border->num_points = start;
    else
    {
      /* copy the last point to the start of this sub-path, since */
      /* it contains the `adjusted' starting coordinates          */
      border->num_points    = --count;
      border->points[start] = border->points[count];
      border->tags[start]   = border->tags[count];

      if ( reverse )
      {
        /* reverse the points */
        {
          FT_TS_Vector*  vec1 = border->points + start + 1;
          FT_TS_Vector*  vec2 = border->points + count - 1;


          for ( ; vec1 < vec2; vec1++, vec2-- )
          {
            FT_TS_Vector  tmp;


            tmp   = *vec1;
            *vec1 = *vec2;
            *vec2 = tmp;
          }
        }

        /* then the tags */
        {
          FT_TS_Byte*  tag1 = border->tags + start + 1;
          FT_TS_Byte*  tag2 = border->tags + count - 1;


          for ( ; tag1 < tag2; tag1++, tag2-- )
          {
            FT_TS_Byte  tmp;


            tmp   = *tag1;
            *tag1 = *tag2;
            *tag2 = tmp;
          }
        }
      }

      border->tags[start    ] |= FT_TS_STROKE_TAG_BEGIN;
      border->tags[count - 1] |= FT_TS_STROKE_TAG_END;
    }

    border->start   = -1;
    border->movable = FALSE;
  }


  static FT_TS_Error
  ft_stroke_border_lineto( FT_TS_StrokeBorder  border,
                           FT_TS_Vector*       to,
                           FT_TS_Bool          movable )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    FT_TS_ASSERT( border->start >= 0 );

    if ( border->movable )
    {
      /* move last point */
      border->points[border->num_points - 1] = *to;
    }
    else
    {
      /* don't add zero-length lineto, but always add moveto */
      if ( border->num_points > (FT_TS_UInt)border->start                     &&
           FT_TS_IS_SMALL( border->points[border->num_points - 1].x - to->x ) &&
           FT_TS_IS_SMALL( border->points[border->num_points - 1].y - to->y ) )
        return error;

      /* add one point */
      error = ft_stroke_border_grow( border, 1 );
      if ( !error )
      {
        FT_TS_Vector*  vec = border->points + border->num_points;
        FT_TS_Byte*    tag = border->tags   + border->num_points;


        vec[0] = *to;
        tag[0] = FT_TS_STROKE_TAG_ON;

        border->num_points += 1;
      }
    }
    border->movable = movable;
    return error;
  }


  static FT_TS_Error
  ft_stroke_border_conicto( FT_TS_StrokeBorder  border,
                            FT_TS_Vector*       control,
                            FT_TS_Vector*       to )
  {
    FT_TS_Error  error;


    FT_TS_ASSERT( border->start >= 0 );

    error = ft_stroke_border_grow( border, 2 );
    if ( !error )
    {
      FT_TS_Vector*  vec = border->points + border->num_points;
      FT_TS_Byte*    tag = border->tags   + border->num_points;


      vec[0] = *control;
      vec[1] = *to;

      tag[0] = 0;
      tag[1] = FT_TS_STROKE_TAG_ON;

      border->num_points += 2;
    }

    border->movable = FALSE;

    return error;
  }


  static FT_TS_Error
  ft_stroke_border_cubicto( FT_TS_StrokeBorder  border,
                            FT_TS_Vector*       control1,
                            FT_TS_Vector*       control2,
                            FT_TS_Vector*       to )
  {
    FT_TS_Error  error;


    FT_TS_ASSERT( border->start >= 0 );

    error = ft_stroke_border_grow( border, 3 );
    if ( !error )
    {
      FT_TS_Vector*  vec = border->points + border->num_points;
      FT_TS_Byte*    tag = border->tags   + border->num_points;


      vec[0] = *control1;
      vec[1] = *control2;
      vec[2] = *to;

      tag[0] = FT_TS_STROKE_TAG_CUBIC;
      tag[1] = FT_TS_STROKE_TAG_CUBIC;
      tag[2] = FT_TS_STROKE_TAG_ON;

      border->num_points += 3;
    }

    border->movable = FALSE;

    return error;
  }


#define FT_TS_ARC_CUBIC_ANGLE  ( FT_TS_ANGLE_PI / 2 )


  static FT_TS_Error
  ft_stroke_border_arcto( FT_TS_StrokeBorder  border,
                          FT_TS_Vector*       center,
                          FT_TS_Fixed         radius,
                          FT_TS_Angle         angle_start,
                          FT_TS_Angle         angle_diff )
  {
    FT_TS_Fixed   coef;
    FT_TS_Vector  a0, a1, a2, a3;
    FT_TS_Int     i, arcs = 1;
    FT_TS_Error   error = FT_TS_Err_Ok;


    /* number of cubic arcs to draw */
    while (  angle_diff > FT_TS_ARC_CUBIC_ANGLE * arcs ||
            -angle_diff > FT_TS_ARC_CUBIC_ANGLE * arcs )
      arcs++;

    /* control tangents */
    coef  = FT_TS_Tan( angle_diff / ( 4 * arcs ) );
    coef += coef / 3;

    /* compute start and first control point */
    FT_TS_Vector_From_Polar( &a0, radius, angle_start );
    a1.x = FT_TS_MulFix( -a0.y, coef );
    a1.y = FT_TS_MulFix(  a0.x, coef );

    a0.x += center->x;
    a0.y += center->y;
    a1.x += a0.x;
    a1.y += a0.y;

    for ( i = 1; i <= arcs; i++ )
    {
      /* compute end and second control point */
      FT_TS_Vector_From_Polar( &a3, radius,
                            angle_start + i * angle_diff / arcs );
      a2.x = FT_TS_MulFix(  a3.y, coef );
      a2.y = FT_TS_MulFix( -a3.x, coef );

      a3.x += center->x;
      a3.y += center->y;
      a2.x += a3.x;
      a2.y += a3.y;

      /* add cubic arc */
      error = ft_stroke_border_cubicto( border, &a1, &a2, &a3 );
      if ( error )
        break;

      /* a0 = a3; */
      a1.x = a3.x - a2.x + a3.x;
      a1.y = a3.y - a2.y + a3.y;
    }

    return error;
  }


  static FT_TS_Error
  ft_stroke_border_moveto( FT_TS_StrokeBorder  border,
                           FT_TS_Vector*       to )
  {
    /* close current open path if any ? */
    if ( border->start >= 0 )
      ft_stroke_border_close( border, FALSE );

    border->start = (FT_TS_Int)border->num_points;
    border->movable = FALSE;

    return ft_stroke_border_lineto( border, to, FALSE );
  }


  static void
  ft_stroke_border_init( FT_TS_StrokeBorder  border,
                         FT_TS_Memory        memory )
  {
    border->memory = memory;
    border->points = NULL;
    border->tags   = NULL;

    border->num_points = 0;
    border->max_points = 0;
    border->start      = -1;
    border->valid      = FALSE;
  }


  static void
  ft_stroke_border_reset( FT_TS_StrokeBorder  border )
  {
    border->num_points = 0;
    border->start      = -1;
    border->valid      = FALSE;
  }


  static void
  ft_stroke_border_done( FT_TS_StrokeBorder  border )
  {
    FT_TS_Memory  memory = border->memory;


    FT_TS_FREE( border->points );
    FT_TS_FREE( border->tags );

    border->num_points = 0;
    border->max_points = 0;
    border->start      = -1;
    border->valid      = FALSE;
  }


  static FT_TS_Error
  ft_stroke_border_get_counts( FT_TS_StrokeBorder  border,
                               FT_TS_UInt         *anum_points,
                               FT_TS_UInt         *anum_contours )
  {
    FT_TS_Error  error        = FT_TS_Err_Ok;
    FT_TS_UInt   num_points   = 0;
    FT_TS_UInt   num_contours = 0;

    FT_TS_UInt     count      = border->num_points;
    FT_TS_Vector*  point      = border->points;
    FT_TS_Byte*    tags       = border->tags;
    FT_TS_Int      in_contour = 0;


    for ( ; count > 0; count--, num_points++, point++, tags++ )
    {
      if ( tags[0] & FT_TS_STROKE_TAG_BEGIN )
      {
        if ( in_contour != 0 )
          goto Fail;

        in_contour = 1;
      }
      else if ( in_contour == 0 )
        goto Fail;

      if ( tags[0] & FT_TS_STROKE_TAG_END )
      {
        in_contour = 0;
        num_contours++;
      }
    }

    if ( in_contour != 0 )
      goto Fail;

    border->valid = TRUE;

  Exit:
    *anum_points   = num_points;
    *anum_contours = num_contours;
    return error;

  Fail:
    num_points   = 0;
    num_contours = 0;
    goto Exit;
  }


  static void
  ft_stroke_border_export( FT_TS_StrokeBorder  border,
                           FT_TS_Outline*      outline )
  {
    /* copy point locations */
    if ( border->num_points )
      FT_TS_ARRAY_COPY( outline->points + outline->n_points,
                     border->points,
                     border->num_points );

    /* copy tags */
    {
      FT_TS_UInt   count = border->num_points;
      FT_TS_Byte*  read  = border->tags;
      FT_TS_Byte*  write = (FT_TS_Byte*)outline->tags + outline->n_points;


      for ( ; count > 0; count--, read++, write++ )
      {
        if ( *read & FT_TS_STROKE_TAG_ON )
          *write = FT_TS_CURVE_TAG_ON;
        else if ( *read & FT_TS_STROKE_TAG_CUBIC )
          *write = FT_TS_CURVE_TAG_CUBIC;
        else
          *write = FT_TS_CURVE_TAG_CONIC;
      }
    }

    /* copy contours */
    {
      FT_TS_UInt    count = border->num_points;
      FT_TS_Byte*   tags  = border->tags;
      FT_TS_Short*  write = outline->contours + outline->n_contours;
      FT_TS_Short   idx   = (FT_TS_Short)outline->n_points;


      for ( ; count > 0; count--, tags++, idx++ )
      {
        if ( *tags & FT_TS_STROKE_TAG_END )
        {
          *write++ = idx;
          outline->n_contours++;
        }
      }
    }

    outline->n_points += (short)border->num_points;

    FT_TS_ASSERT( FT_TS_Outline_Check( outline ) == 0 );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           STROKER                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define FT_TS_SIDE_TO_ROTATE( s )   ( FT_TS_ANGLE_PI2 - (s) * FT_TS_ANGLE_PI )

  typedef struct  FT_TS_StrokerRec_
  {
    FT_TS_Angle             angle_in;             /* direction into curr join */
    FT_TS_Angle             angle_out;            /* direction out of join  */
    FT_TS_Vector            center;               /* current position */
    FT_TS_Fixed             line_length;          /* length of last lineto */
    FT_TS_Bool              first_point;          /* is this the start? */
    FT_TS_Bool              subpath_open;         /* is the subpath open? */
    FT_TS_Angle             subpath_angle;        /* subpath start direction */
    FT_TS_Vector            subpath_start;        /* subpath start position */
    FT_TS_Fixed             subpath_line_length;  /* subpath start lineto len */
    FT_TS_Bool              handle_wide_strokes;  /* use wide strokes logic? */

    FT_TS_Stroker_LineCap   line_cap;
    FT_TS_Stroker_LineJoin  line_join;
    FT_TS_Stroker_LineJoin  line_join_saved;
    FT_TS_Fixed             miter_limit;
    FT_TS_Fixed             radius;

    FT_TS_StrokeBorderRec   borders[2];
    FT_TS_Library           library;

  } FT_TS_StrokerRec;


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_New( FT_TS_Library   library,
                  FT_TS_Stroker  *astroker )
  {
    FT_TS_Error    error;           /* assigned in FT_TS_NEW */
    FT_TS_Memory   memory;
    FT_TS_Stroker  stroker = NULL;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !astroker )
      return FT_TS_THROW( Invalid_Argument );

    memory = library->memory;

    if ( !FT_TS_NEW( stroker ) )
    {
      stroker->library = library;

      ft_stroke_border_init( &stroker->borders[0], memory );
      ft_stroke_border_init( &stroker->borders[1], memory );
    }

    *astroker = stroker;

    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Stroker_Set( FT_TS_Stroker           stroker,
                  FT_TS_Fixed             radius,
                  FT_TS_Stroker_LineCap   line_cap,
                  FT_TS_Stroker_LineJoin  line_join,
                  FT_TS_Fixed             miter_limit )
  {
    if ( !stroker )
      return;

    stroker->radius      = radius;
    stroker->line_cap    = line_cap;
    stroker->line_join   = line_join;
    stroker->miter_limit = miter_limit;

    /* ensure miter limit has sensible value */
    if ( stroker->miter_limit < 0x10000L )
      stroker->miter_limit = 0x10000L;

    /* save line join style:                                           */
    /* line join style can be temporarily changed when stroking curves */
    stroker->line_join_saved = line_join;

    FT_TS_Stroker_Rewind( stroker );
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Stroker_Rewind( FT_TS_Stroker  stroker )
  {
    if ( stroker )
    {
      ft_stroke_border_reset( &stroker->borders[0] );
      ft_stroke_border_reset( &stroker->borders[1] );
    }
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Stroker_Done( FT_TS_Stroker  stroker )
  {
    if ( stroker )
    {
      FT_TS_Memory  memory = stroker->library->memory;


      ft_stroke_border_done( &stroker->borders[0] );
      ft_stroke_border_done( &stroker->borders[1] );

      stroker->library = NULL;
      FT_TS_FREE( stroker );
    }
  }


  /* create a circular arc at a corner or cap */
  static FT_TS_Error
  ft_stroker_arcto( FT_TS_Stroker  stroker,
                    FT_TS_Int      side )
  {
    FT_TS_Angle         total, rotate;
    FT_TS_Fixed         radius = stroker->radius;
    FT_TS_Error         error  = FT_TS_Err_Ok;
    FT_TS_StrokeBorder  border = stroker->borders + side;


    rotate = FT_TS_SIDE_TO_ROTATE( side );

    total = FT_TS_Angle_Diff( stroker->angle_in, stroker->angle_out );
    if ( total == FT_TS_ANGLE_PI )
      total = -rotate * 2;

    error = ft_stroke_border_arcto( border,
                                    &stroker->center,
                                    radius,
                                    stroker->angle_in + rotate,
                                    total );
    border->movable = FALSE;
    return error;
  }


  /* add a cap at the end of an opened path */
  static FT_TS_Error
  ft_stroker_cap( FT_TS_Stroker  stroker,
                  FT_TS_Angle    angle,
                  FT_TS_Int      side )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    if ( stroker->line_cap == FT_TS_STROKER_LINECAP_ROUND )
    {
      /* add a round cap */
      stroker->angle_in  = angle;
      stroker->angle_out = angle + FT_TS_ANGLE_PI;

      error = ft_stroker_arcto( stroker, side );
    }
    else
    {
      /* add a square or butt cap */
      FT_TS_Vector        middle, delta;
      FT_TS_Fixed         radius = stroker->radius;
      FT_TS_StrokeBorder  border = stroker->borders + side;


      /* compute middle point and first angle point */
      FT_TS_Vector_From_Polar( &middle, radius, angle );
      delta.x = side ?  middle.y : -middle.y;
      delta.y = side ? -middle.x :  middle.x;

      if ( stroker->line_cap == FT_TS_STROKER_LINECAP_SQUARE )
      {
        middle.x += stroker->center.x;
        middle.y += stroker->center.y;
      }
      else  /* FT_TS_STROKER_LINECAP_BUTT */
      {
        middle.x  = stroker->center.x;
        middle.y  = stroker->center.y;
      }

      delta.x  += middle.x;
      delta.y  += middle.y;

      error = ft_stroke_border_lineto( border, &delta, FALSE );
      if ( error )
        goto Exit;

      /* compute second angle point */
      delta.x = middle.x - delta.x + middle.x;
      delta.y = middle.y - delta.y + middle.y;

      error = ft_stroke_border_lineto( border, &delta, FALSE );
    }

  Exit:
    return error;
  }


  /* process an inside corner, i.e. compute intersection */
  static FT_TS_Error
  ft_stroker_inside( FT_TS_Stroker  stroker,
                     FT_TS_Int      side,
                     FT_TS_Fixed    line_length )
  {
    FT_TS_StrokeBorder  border = stroker->borders + side;
    FT_TS_Angle         phi, theta, rotate;
    FT_TS_Fixed         length;
    FT_TS_Vector        sigma = { 0, 0 };
    FT_TS_Vector        delta;
    FT_TS_Error         error = FT_TS_Err_Ok;
    FT_TS_Bool          intersect;          /* use intersection of lines? */


    rotate = FT_TS_SIDE_TO_ROTATE( side );

    theta = FT_TS_Angle_Diff( stroker->angle_in, stroker->angle_out ) / 2;

    /* Only intersect borders if between two lineto's and both */
    /* lines are long enough (line_length is zero for curves). */
    /* Also avoid U-turns of nearly 180 degree.                */
    if ( !border->movable || line_length == 0  ||
         theta > 0x59C000 || theta < -0x59C000 )
      intersect = FALSE;
    else
    {
      /* compute minimum required length of lines */
      FT_TS_Fixed  min_length;


      FT_TS_Vector_Unit( &sigma, theta );
      min_length =
        ft_pos_abs( FT_TS_MulDiv( stroker->radius, sigma.y, sigma.x ) );

      intersect = FT_TS_BOOL( min_length                         &&
                           stroker->line_length >= min_length &&
                           line_length          >= min_length );
    }

    if ( !intersect )
    {
      FT_TS_Vector_From_Polar( &delta, stroker->radius,
                            stroker->angle_out + rotate );
      delta.x += stroker->center.x;
      delta.y += stroker->center.y;

      border->movable = FALSE;
    }
    else
    {
      /* compute median angle */
      phi = stroker->angle_in + theta + rotate;

      length = FT_TS_DivFix( stroker->radius, sigma.x );

      FT_TS_Vector_From_Polar( &delta, length, phi );
      delta.x += stroker->center.x;
      delta.y += stroker->center.y;
    }

    error = ft_stroke_border_lineto( border, &delta, FALSE );

    return error;
  }


  /* process an outside corner, i.e. compute bevel/miter/round */
  static FT_TS_Error
  ft_stroker_outside( FT_TS_Stroker  stroker,
                      FT_TS_Int      side,
                      FT_TS_Fixed    line_length )
  {
    FT_TS_StrokeBorder  border = stroker->borders + side;
    FT_TS_Error         error;
    FT_TS_Angle         rotate;


    if ( stroker->line_join == FT_TS_STROKER_LINEJOIN_ROUND )
      error = ft_stroker_arcto( stroker, side );
    else
    {
      /* this is a mitered (pointed) or beveled (truncated) corner */
      FT_TS_Fixed   radius = stroker->radius;
      FT_TS_Vector  sigma = { 0, 0 };
      FT_TS_Angle   theta = 0, phi = 0;
      FT_TS_Bool    bevel, fixed_bevel;


      rotate = FT_TS_SIDE_TO_ROTATE( side );

      bevel =
        FT_TS_BOOL( stroker->line_join == FT_TS_STROKER_LINEJOIN_BEVEL );

      fixed_bevel =
        FT_TS_BOOL( stroker->line_join != FT_TS_STROKER_LINEJOIN_MITER_VARIABLE );

      /* check miter limit first */
      if ( !bevel )
      {
        theta = FT_TS_Angle_Diff( stroker->angle_in, stroker->angle_out ) / 2;

        if ( theta == FT_TS_ANGLE_PI2 )
          theta = -rotate;

        phi    = stroker->angle_in + theta + rotate;

        FT_TS_Vector_From_Polar( &sigma, stroker->miter_limit, theta );

        /* is miter limit exceeded? */
        if ( sigma.x < 0x10000L )
        {
          /* don't create variable bevels for very small deviations; */
          /* FT_TS_Sin(x) = 0 for x <= 57                               */
          if ( fixed_bevel || ft_pos_abs( theta ) > 57 )
            bevel = TRUE;
        }
      }

      if ( bevel )  /* this is a bevel (broken angle) */
      {
        if ( fixed_bevel )
        {
          /* the outer corners are simply joined together */
          FT_TS_Vector  delta;


          /* add bevel */
          FT_TS_Vector_From_Polar( &delta,
                                radius,
                                stroker->angle_out + rotate );
          delta.x += stroker->center.x;
          delta.y += stroker->center.y;

          border->movable = FALSE;
          error = ft_stroke_border_lineto( border, &delta, FALSE );
        }
        else /* variable bevel or clipped miter */
        {
          /* the miter is truncated */
          FT_TS_Vector  middle, delta;
          FT_TS_Fixed   coef;


          /* compute middle point and first angle point */
          FT_TS_Vector_From_Polar( &middle,
                                FT_TS_MulFix( radius, stroker->miter_limit ),
                                phi );

          coef    = FT_TS_DivFix(  0x10000L - sigma.x, sigma.y );
          delta.x = FT_TS_MulFix(  middle.y, coef );
          delta.y = FT_TS_MulFix( -middle.x, coef );

          middle.x += stroker->center.x;
          middle.y += stroker->center.y;
          delta.x  += middle.x;
          delta.y  += middle.y;

          error = ft_stroke_border_lineto( border, &delta, FALSE );
          if ( error )
            goto Exit;

          /* compute second angle point */
          delta.x = middle.x - delta.x + middle.x;
          delta.y = middle.y - delta.y + middle.y;

          error = ft_stroke_border_lineto( border, &delta, FALSE );
          if ( error )
            goto Exit;

          /* finally, add an end point; only needed if not lineto */
          /* (line_length is zero for curves)                     */
          if ( line_length == 0 )
          {
            FT_TS_Vector_From_Polar( &delta,
                                  radius,
                                  stroker->angle_out + rotate );

            delta.x += stroker->center.x;
            delta.y += stroker->center.y;

            error = ft_stroke_border_lineto( border, &delta, FALSE );
          }
        }
      }
      else /* this is a miter (intersection) */
      {
        FT_TS_Fixed   length;
        FT_TS_Vector  delta;


        length = FT_TS_MulDiv( stroker->radius, stroker->miter_limit, sigma.x );

        FT_TS_Vector_From_Polar( &delta, length, phi );
        delta.x += stroker->center.x;
        delta.y += stroker->center.y;

        error = ft_stroke_border_lineto( border, &delta, FALSE );
        if ( error )
          goto Exit;

        /* now add an end point; only needed if not lineto */
        /* (line_length is zero for curves)                */
        if ( line_length == 0 )
        {
          FT_TS_Vector_From_Polar( &delta,
                                stroker->radius,
                                stroker->angle_out + rotate );
          delta.x += stroker->center.x;
          delta.y += stroker->center.y;

          error = ft_stroke_border_lineto( border, &delta, FALSE );
        }
      }
    }

  Exit:
    return error;
  }


  static FT_TS_Error
  ft_stroker_process_corner( FT_TS_Stroker  stroker,
                             FT_TS_Fixed    line_length )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;
    FT_TS_Angle  turn;
    FT_TS_Int    inside_side;


    turn = FT_TS_Angle_Diff( stroker->angle_in, stroker->angle_out );

    /* no specific corner processing is required if the turn is 0 */
    if ( turn == 0 )
      goto Exit;

    /* when we turn to the right, the inside side is 0 */
    /* otherwise, the inside side is 1 */
    inside_side = ( turn < 0 );

    /* process the inside side */
    error = ft_stroker_inside( stroker, inside_side, line_length );
    if ( error )
      goto Exit;

    /* process the outside side */
    error = ft_stroker_outside( stroker, !inside_side, line_length );

  Exit:
    return error;
  }


  /* add two points to the left and right borders corresponding to the */
  /* start of the subpath                                              */
  static FT_TS_Error
  ft_stroker_subpath_start( FT_TS_Stroker  stroker,
                            FT_TS_Angle    start_angle,
                            FT_TS_Fixed    line_length )
  {
    FT_TS_Vector        delta;
    FT_TS_Vector        point;
    FT_TS_Error         error;
    FT_TS_StrokeBorder  border;


    FT_TS_Vector_From_Polar( &delta, stroker->radius,
                          start_angle + FT_TS_ANGLE_PI2 );

    point.x = stroker->center.x + delta.x;
    point.y = stroker->center.y + delta.y;

    border = stroker->borders;
    error = ft_stroke_border_moveto( border, &point );
    if ( error )
      goto Exit;

    point.x = stroker->center.x - delta.x;
    point.y = stroker->center.y - delta.y;

    border++;
    error = ft_stroke_border_moveto( border, &point );

    /* save angle, position, and line length for last join */
    /* (line_length is zero for curves)                    */
    stroker->subpath_angle       = start_angle;
    stroker->first_point         = FALSE;
    stroker->subpath_line_length = line_length;

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_LineTo( FT_TS_Stroker  stroker,
                     FT_TS_Vector*  to )
  {
    FT_TS_Error         error = FT_TS_Err_Ok;
    FT_TS_StrokeBorder  border;
    FT_TS_Vector        delta;
    FT_TS_Angle         angle;
    FT_TS_Int           side;
    FT_TS_Fixed         line_length;


    if ( !stroker || !to )
      return FT_TS_THROW( Invalid_Argument );

    delta.x = to->x - stroker->center.x;
    delta.y = to->y - stroker->center.y;

    /* a zero-length lineto is a no-op; avoid creating a spurious corner */
    if ( delta.x == 0 && delta.y == 0 )
       goto Exit;

    /* compute length of line */
    line_length = FT_TS_Vector_Length( &delta );

    angle = FT_TS_Atan2( delta.x, delta.y );
    FT_TS_Vector_From_Polar( &delta, stroker->radius, angle + FT_TS_ANGLE_PI2 );

    /* process corner if necessary */
    if ( stroker->first_point )
    {
      /* This is the first segment of a subpath.  We need to     */
      /* add a point to each border at their respective starting */
      /* point locations.                                        */
      error = ft_stroker_subpath_start( stroker, angle, line_length );
      if ( error )
        goto Exit;
    }
    else
    {
      /* process the current corner */
      stroker->angle_out = angle;
      error = ft_stroker_process_corner( stroker, line_length );
      if ( error )
        goto Exit;
    }

    /* now add a line segment to both the `inside' and `outside' paths */
    for ( border = stroker->borders, side = 1; side >= 0; side--, border++ )
    {
      FT_TS_Vector  point;


      point.x = to->x + delta.x;
      point.y = to->y + delta.y;

      /* the ends of lineto borders are movable */
      error = ft_stroke_border_lineto( border, &point, TRUE );
      if ( error )
        goto Exit;

      delta.x = -delta.x;
      delta.y = -delta.y;
    }

    stroker->angle_in    = angle;
    stroker->center      = *to;
    stroker->line_length = line_length;

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_ConicTo( FT_TS_Stroker  stroker,
                      FT_TS_Vector*  control,
                      FT_TS_Vector*  to )
  {
    FT_TS_Error    error = FT_TS_Err_Ok;
    FT_TS_Vector   bez_stack[34];
    FT_TS_Vector*  arc;
    FT_TS_Vector*  limit = bez_stack + 30;
    FT_TS_Bool     first_arc = TRUE;


    if ( !stroker || !control || !to )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    /* if all control points are coincident, this is a no-op; */
    /* avoid creating a spurious corner                       */
    if ( FT_TS_IS_SMALL( stroker->center.x - control->x ) &&
         FT_TS_IS_SMALL( stroker->center.y - control->y ) &&
         FT_TS_IS_SMALL( control->x        - to->x      ) &&
         FT_TS_IS_SMALL( control->y        - to->y      ) )
    {
       stroker->center = *to;
       goto Exit;
    }

    arc    = bez_stack;
    arc[0] = *to;
    arc[1] = *control;
    arc[2] = stroker->center;

    while ( arc >= bez_stack )
    {
      FT_TS_Angle  angle_in, angle_out;


      /* initialize with current direction */
      angle_in = angle_out = stroker->angle_in;

      if ( arc < limit                                             &&
           !ft_conic_is_small_enough( arc, &angle_in, &angle_out ) )
      {
        if ( stroker->first_point )
          stroker->angle_in = angle_in;

        ft_conic_split( arc );
        arc += 2;
        continue;
      }

      if ( first_arc )
      {
        first_arc = FALSE;

        /* process corner if necessary */
        if ( stroker->first_point )
          error = ft_stroker_subpath_start( stroker, angle_in, 0 );
        else
        {
          stroker->angle_out = angle_in;
          error = ft_stroker_process_corner( stroker, 0 );
        }
      }
      else if ( ft_pos_abs( FT_TS_Angle_Diff( stroker->angle_in, angle_in ) ) >
                  FT_TS_SMALL_CONIC_THRESHOLD / 4                             )
      {
        /* if the deviation from one arc to the next is too great, */
        /* add a round corner                                      */
        stroker->center    = arc[2];
        stroker->angle_out = angle_in;
        stroker->line_join = FT_TS_STROKER_LINEJOIN_ROUND;

        error = ft_stroker_process_corner( stroker, 0 );

        /* reinstate line join style */
        stroker->line_join = stroker->line_join_saved;
      }

      if ( error )
        goto Exit;

      /* the arc's angle is small enough; we can add it directly to each */
      /* border                                                          */
      {
        FT_TS_Vector        ctrl, end;
        FT_TS_Angle         theta, phi, rotate, alpha0 = 0;
        FT_TS_Fixed         length;
        FT_TS_StrokeBorder  border;
        FT_TS_Int           side;


        theta  = FT_TS_Angle_Diff( angle_in, angle_out ) / 2;
        phi    = angle_in + theta;
        length = FT_TS_DivFix( stroker->radius, FT_TS_Cos( theta ) );

        /* compute direction of original arc */
        if ( stroker->handle_wide_strokes )
          alpha0 = FT_TS_Atan2( arc[0].x - arc[2].x, arc[0].y - arc[2].y );

        for ( border = stroker->borders, side = 0;
              side <= 1;
              side++, border++ )
        {
          rotate = FT_TS_SIDE_TO_ROTATE( side );

          /* compute control point */
          FT_TS_Vector_From_Polar( &ctrl, length, phi + rotate );
          ctrl.x += arc[1].x;
          ctrl.y += arc[1].y;

          /* compute end point */
          FT_TS_Vector_From_Polar( &end, stroker->radius, angle_out + rotate );
          end.x += arc[0].x;
          end.y += arc[0].y;

          if ( stroker->handle_wide_strokes )
          {
            FT_TS_Vector  start;
            FT_TS_Angle   alpha1;


            /* determine whether the border radius is greater than the */
            /* radius of curvature of the original arc                 */
            start = border->points[border->num_points - 1];

            alpha1 = FT_TS_Atan2( end.x - start.x, end.y - start.y );

            /* is the direction of the border arc opposite to */
            /* that of the original arc? */
            if ( ft_pos_abs( FT_TS_Angle_Diff( alpha0, alpha1 ) ) >
                   FT_TS_ANGLE_PI / 2                             )
            {
              FT_TS_Angle   beta, gamma;
              FT_TS_Vector  bvec, delta;
              FT_TS_Fixed   blen, sinA, sinB, alen;


              /* use the sine rule to find the intersection point */
              beta  = FT_TS_Atan2( arc[2].x - start.x, arc[2].y - start.y );
              gamma = FT_TS_Atan2( arc[0].x - end.x,   arc[0].y - end.y );

              bvec.x = end.x - start.x;
              bvec.y = end.y - start.y;

              blen = FT_TS_Vector_Length( &bvec );

              sinA = ft_pos_abs( FT_TS_Sin( alpha1 - gamma ) );
              sinB = ft_pos_abs( FT_TS_Sin( beta - gamma ) );

              alen = FT_TS_MulDiv( blen, sinA, sinB );

              FT_TS_Vector_From_Polar( &delta, alen, beta );
              delta.x += start.x;
              delta.y += start.y;

              /* circumnavigate the negative sector backwards */
              border->movable = FALSE;
              error = ft_stroke_border_lineto( border, &delta, FALSE );
              if ( error )
                goto Exit;
              error = ft_stroke_border_lineto( border, &end, FALSE );
              if ( error )
                goto Exit;
              error = ft_stroke_border_conicto( border, &ctrl, &start );
              if ( error )
                goto Exit;
              /* and then move to the endpoint */
              error = ft_stroke_border_lineto( border, &end, FALSE );
              if ( error )
                goto Exit;

              continue;
            }

            /* else fall through */
          }

          /* simply add an arc */
          error = ft_stroke_border_conicto( border, &ctrl, &end );
          if ( error )
            goto Exit;
        }
      }

      arc -= 2;

      stroker->angle_in = angle_out;
    }

    stroker->center      = *to;
    stroker->line_length = 0;

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_CubicTo( FT_TS_Stroker  stroker,
                      FT_TS_Vector*  control1,
                      FT_TS_Vector*  control2,
                      FT_TS_Vector*  to )
  {
    FT_TS_Error    error = FT_TS_Err_Ok;
    FT_TS_Vector   bez_stack[37];
    FT_TS_Vector*  arc;
    FT_TS_Vector*  limit = bez_stack + 32;
    FT_TS_Bool     first_arc = TRUE;


    if ( !stroker || !control1 || !control2 || !to )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    /* if all control points are coincident, this is a no-op; */
    /* avoid creating a spurious corner */
    if ( FT_TS_IS_SMALL( stroker->center.x - control1->x ) &&
         FT_TS_IS_SMALL( stroker->center.y - control1->y ) &&
         FT_TS_IS_SMALL( control1->x       - control2->x ) &&
         FT_TS_IS_SMALL( control1->y       - control2->y ) &&
         FT_TS_IS_SMALL( control2->x       - to->x       ) &&
         FT_TS_IS_SMALL( control2->y       - to->y       ) )
    {
       stroker->center = *to;
       goto Exit;
    }

    arc    = bez_stack;
    arc[0] = *to;
    arc[1] = *control2;
    arc[2] = *control1;
    arc[3] = stroker->center;

    while ( arc >= bez_stack )
    {
      FT_TS_Angle  angle_in, angle_mid, angle_out;


      /* initialize with current direction */
      angle_in = angle_out = angle_mid = stroker->angle_in;

      if ( arc < limit                                         &&
           !ft_cubic_is_small_enough( arc, &angle_in,
                                      &angle_mid, &angle_out ) )
      {
        if ( stroker->first_point )
          stroker->angle_in = angle_in;

        ft_cubic_split( arc );
        arc += 3;
        continue;
      }

      if ( first_arc )
      {
        first_arc = FALSE;

        /* process corner if necessary */
        if ( stroker->first_point )
          error = ft_stroker_subpath_start( stroker, angle_in, 0 );
        else
        {
          stroker->angle_out = angle_in;
          error = ft_stroker_process_corner( stroker, 0 );
        }
      }
      else if ( ft_pos_abs( FT_TS_Angle_Diff( stroker->angle_in, angle_in ) ) >
                  FT_TS_SMALL_CUBIC_THRESHOLD / 4                             )
      {
        /* if the deviation from one arc to the next is too great, */
        /* add a round corner                                      */
        stroker->center    = arc[3];
        stroker->angle_out = angle_in;
        stroker->line_join = FT_TS_STROKER_LINEJOIN_ROUND;

        error = ft_stroker_process_corner( stroker, 0 );

        /* reinstate line join style */
        stroker->line_join = stroker->line_join_saved;
      }

      if ( error )
        goto Exit;

      /* the arc's angle is small enough; we can add it directly to each */
      /* border                                                          */
      {
        FT_TS_Vector        ctrl1, ctrl2, end;
        FT_TS_Angle         theta1, phi1, theta2, phi2, rotate, alpha0 = 0;
        FT_TS_Fixed         length1, length2;
        FT_TS_StrokeBorder  border;
        FT_TS_Int           side;


        theta1  = FT_TS_Angle_Diff( angle_in,  angle_mid ) / 2;
        theta2  = FT_TS_Angle_Diff( angle_mid, angle_out ) / 2;
        phi1    = ft_angle_mean( angle_in,  angle_mid );
        phi2    = ft_angle_mean( angle_mid, angle_out );
        length1 = FT_TS_DivFix( stroker->radius, FT_TS_Cos( theta1 ) );
        length2 = FT_TS_DivFix( stroker->radius, FT_TS_Cos( theta2 ) );

        /* compute direction of original arc */
        if ( stroker->handle_wide_strokes )
          alpha0 = FT_TS_Atan2( arc[0].x - arc[3].x, arc[0].y - arc[3].y );

        for ( border = stroker->borders, side = 0;
              side <= 1;
              side++, border++ )
        {
          rotate = FT_TS_SIDE_TO_ROTATE( side );

          /* compute control points */
          FT_TS_Vector_From_Polar( &ctrl1, length1, phi1 + rotate );
          ctrl1.x += arc[2].x;
          ctrl1.y += arc[2].y;

          FT_TS_Vector_From_Polar( &ctrl2, length2, phi2 + rotate );
          ctrl2.x += arc[1].x;
          ctrl2.y += arc[1].y;

          /* compute end point */
          FT_TS_Vector_From_Polar( &end, stroker->radius, angle_out + rotate );
          end.x += arc[0].x;
          end.y += arc[0].y;

          if ( stroker->handle_wide_strokes )
          {
            FT_TS_Vector  start;
            FT_TS_Angle   alpha1;


            /* determine whether the border radius is greater than the */
            /* radius of curvature of the original arc                 */
            start = border->points[border->num_points - 1];

            alpha1 = FT_TS_Atan2( end.x - start.x, end.y - start.y );

            /* is the direction of the border arc opposite to */
            /* that of the original arc? */
            if ( ft_pos_abs( FT_TS_Angle_Diff( alpha0, alpha1 ) ) >
                   FT_TS_ANGLE_PI / 2                             )
            {
              FT_TS_Angle   beta, gamma;
              FT_TS_Vector  bvec, delta;
              FT_TS_Fixed   blen, sinA, sinB, alen;


              /* use the sine rule to find the intersection point */
              beta  = FT_TS_Atan2( arc[3].x - start.x, arc[3].y - start.y );
              gamma = FT_TS_Atan2( arc[0].x - end.x,   arc[0].y - end.y );

              bvec.x = end.x - start.x;
              bvec.y = end.y - start.y;

              blen = FT_TS_Vector_Length( &bvec );

              sinA = ft_pos_abs( FT_TS_Sin( alpha1 - gamma ) );
              sinB = ft_pos_abs( FT_TS_Sin( beta - gamma ) );

              alen = FT_TS_MulDiv( blen, sinA, sinB );

              FT_TS_Vector_From_Polar( &delta, alen, beta );
              delta.x += start.x;
              delta.y += start.y;

              /* circumnavigate the negative sector backwards */
              border->movable = FALSE;
              error = ft_stroke_border_lineto( border, &delta, FALSE );
              if ( error )
                goto Exit;
              error = ft_stroke_border_lineto( border, &end, FALSE );
              if ( error )
                goto Exit;
              error = ft_stroke_border_cubicto( border,
                                                &ctrl2,
                                                &ctrl1,
                                                &start );
              if ( error )
                goto Exit;
              /* and then move to the endpoint */
              error = ft_stroke_border_lineto( border, &end, FALSE );
              if ( error )
                goto Exit;

              continue;
            }

            /* else fall through */
          }

          /* simply add an arc */
          error = ft_stroke_border_cubicto( border, &ctrl1, &ctrl2, &end );
          if ( error )
            goto Exit;
        }
      }

      arc -= 3;

      stroker->angle_in = angle_out;
    }

    stroker->center      = *to;
    stroker->line_length = 0;

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_BeginSubPath( FT_TS_Stroker  stroker,
                           FT_TS_Vector*  to,
                           FT_TS_Bool     open )
  {
    if ( !stroker || !to )
      return FT_TS_THROW( Invalid_Argument );

    /* We cannot process the first point, because there is not enough      */
    /* information regarding its corner/cap.  The latter will be processed */
    /* in the `FT_TS_Stroker_EndSubPath' routine.                             */
    /*                                                                     */
    stroker->first_point  = TRUE;
    stroker->center       = *to;
    stroker->subpath_open = open;

    /* Determine if we need to check whether the border radius is greater */
    /* than the radius of curvature of a curve, to handle this case       */
    /* specially.  This is only required if bevel joins or butt caps may  */
    /* be created, because round & miter joins and round & square caps    */
    /* cover the negative sector created with wide strokes.               */
    stroker->handle_wide_strokes =
      FT_TS_BOOL( stroker->line_join != FT_TS_STROKER_LINEJOIN_ROUND  ||
               ( stroker->subpath_open                        &&
                 stroker->line_cap == FT_TS_STROKER_LINECAP_BUTT ) );

    /* record the subpath start point for each border */
    stroker->subpath_start = *to;

    stroker->angle_in = 0;

    return FT_TS_Err_Ok;
  }


  static FT_TS_Error
  ft_stroker_add_reverse_left( FT_TS_Stroker  stroker,
                               FT_TS_Bool     open )
  {
    FT_TS_StrokeBorder  right = stroker->borders + 0;
    FT_TS_StrokeBorder  left  = stroker->borders + 1;
    FT_TS_Int           new_points;
    FT_TS_Error         error = FT_TS_Err_Ok;


    FT_TS_ASSERT( left->start >= 0 );

    new_points = (FT_TS_Int)left->num_points - left->start;
    if ( new_points > 0 )
    {
      error = ft_stroke_border_grow( right, (FT_TS_UInt)new_points );
      if ( error )
        goto Exit;

      {
        FT_TS_Vector*  dst_point = right->points + right->num_points;
        FT_TS_Byte*    dst_tag   = right->tags   + right->num_points;
        FT_TS_Vector*  src_point = left->points  + left->num_points - 1;
        FT_TS_Byte*    src_tag   = left->tags    + left->num_points - 1;


        while ( src_point >= left->points + left->start )
        {
          *dst_point = *src_point;
          *dst_tag   = *src_tag;

          if ( open )
            dst_tag[0] &= ~FT_TS_STROKE_TAG_BEGIN_END;
          else
          {
            FT_TS_Byte  ttag =
                       (FT_TS_Byte)( dst_tag[0] & FT_TS_STROKE_TAG_BEGIN_END );


            /* switch begin/end tags if necessary */
            if ( ttag == FT_TS_STROKE_TAG_BEGIN ||
                 ttag == FT_TS_STROKE_TAG_END   )
              dst_tag[0] ^= FT_TS_STROKE_TAG_BEGIN_END;
          }

          src_point--;
          src_tag--;
          dst_point++;
          dst_tag++;
        }
      }

      left->num_points   = (FT_TS_UInt)left->start;
      right->num_points += (FT_TS_UInt)new_points;

      right->movable = FALSE;
      left->movable  = FALSE;
    }

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  /* there's a lot of magic in this function! */
  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_EndSubPath( FT_TS_Stroker  stroker )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    if ( !stroker )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( stroker->subpath_open )
    {
      FT_TS_StrokeBorder  right = stroker->borders;


      /* All right, this is an opened path, we need to add a cap between */
      /* right & left, add the reverse of left, then add a final cap     */
      /* between left & right.                                           */
      error = ft_stroker_cap( stroker, stroker->angle_in, 0 );
      if ( error )
        goto Exit;

      /* add reversed points from `left' to `right' */
      error = ft_stroker_add_reverse_left( stroker, TRUE );
      if ( error )
        goto Exit;

      /* now add the final cap */
      stroker->center = stroker->subpath_start;
      error = ft_stroker_cap( stroker,
                              stroker->subpath_angle + FT_TS_ANGLE_PI, 0 );
      if ( error )
        goto Exit;

      /* Now end the right subpath accordingly.  The left one is */
      /* rewind and doesn't need further processing.             */
      ft_stroke_border_close( right, FALSE );
    }
    else
    {
      /* close the path if needed */
      if ( !FT_TS_IS_SMALL( stroker->center.x - stroker->subpath_start.x ) ||
           !FT_TS_IS_SMALL( stroker->center.y - stroker->subpath_start.y ) )
      {
         error = FT_TS_Stroker_LineTo( stroker, &stroker->subpath_start );
         if ( error )
           goto Exit;
      }

      /* process the corner */
      stroker->angle_out = stroker->subpath_angle;

      error = ft_stroker_process_corner( stroker,
                                         stroker->subpath_line_length );
      if ( error )
        goto Exit;

      /* then end our two subpaths */
      ft_stroke_border_close( stroker->borders + 0, FALSE );
      ft_stroke_border_close( stroker->borders + 1, TRUE );
    }

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_GetBorderCounts( FT_TS_Stroker        stroker,
                              FT_TS_StrokerBorder  border,
                              FT_TS_UInt          *anum_points,
                              FT_TS_UInt          *anum_contours )
  {
    FT_TS_UInt   num_points = 0, num_contours = 0;
    FT_TS_Error  error;


    if ( !stroker || border > 1 )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    error = ft_stroke_border_get_counts( stroker->borders + border,
                                         &num_points, &num_contours );
  Exit:
    if ( anum_points )
      *anum_points = num_points;

    if ( anum_contours )
      *anum_contours = num_contours;

    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_GetCounts( FT_TS_Stroker  stroker,
                        FT_TS_UInt    *anum_points,
                        FT_TS_UInt    *anum_contours )
  {
    FT_TS_UInt   count1, count2, num_points   = 0;
    FT_TS_UInt   count3, count4, num_contours = 0;
    FT_TS_Error  error;


    if ( !stroker )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    error = ft_stroke_border_get_counts( stroker->borders + 0,
                                         &count1, &count2 );
    if ( error )
      goto Exit;

    error = ft_stroke_border_get_counts( stroker->borders + 1,
                                         &count3, &count4 );
    if ( error )
      goto Exit;

    num_points   = count1 + count3;
    num_contours = count2 + count4;

  Exit:
    if ( anum_points )
      *anum_points   = num_points;

    if ( anum_contours )
      *anum_contours = num_contours;

    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Stroker_ExportBorder( FT_TS_Stroker        stroker,
                           FT_TS_StrokerBorder  border,
                           FT_TS_Outline*       outline )
  {
    if ( !stroker || !outline )
      return;

    if ( border == FT_TS_STROKER_BORDER_LEFT  ||
         border == FT_TS_STROKER_BORDER_RIGHT )
    {
      FT_TS_StrokeBorder  sborder = & stroker->borders[border];


      if ( sborder->valid )
        ft_stroke_border_export( sborder, outline );
    }
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Stroker_Export( FT_TS_Stroker   stroker,
                     FT_TS_Outline*  outline )
  {
    FT_TS_Stroker_ExportBorder( stroker, FT_TS_STROKER_BORDER_LEFT, outline );
    FT_TS_Stroker_ExportBorder( stroker, FT_TS_STROKER_BORDER_RIGHT, outline );
  }


  /* documentation is in ftstroke.h */

  /*
   * The following is very similar to FT_TS_Outline_Decompose, except
   * that we do support opened paths, and do not scale the outline.
   */
  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Stroker_ParseOutline( FT_TS_Stroker   stroker,
                           FT_TS_Outline*  outline,
                           FT_TS_Bool      opened )
  {
    FT_TS_Vector   v_last;
    FT_TS_Vector   v_control;
    FT_TS_Vector   v_start;

    FT_TS_Vector*  point;
    FT_TS_Vector*  limit;
    char*       tags;

    FT_TS_Error    error;

    FT_TS_Int      n;         /* index of contour in outline     */
    FT_TS_UInt     first;     /* index of first point in contour */
    FT_TS_Int      tag;       /* current point's state           */


    if ( !outline )
      return FT_TS_THROW( Invalid_Outline );

    if ( !stroker )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_Stroker_Rewind( stroker );

    first = 0;

    for ( n = 0; n < outline->n_contours; n++ )
    {
      FT_TS_UInt  last;  /* index of last point in contour */


      last  = (FT_TS_UInt)outline->contours[n];
      limit = outline->points + last;

      /* skip empty points; we don't stroke these */
      if ( last <= first )
      {
        first = last + 1;
        continue;
      }

      v_start = outline->points[first];
      v_last  = outline->points[last];

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
        /* First point is conic control.  Yes, this happens. */
        if ( FT_TS_CURVE_TAG( outline->tags[last] ) == FT_TS_CURVE_TAG_ON )
        {
          /* start at last point if it is on the curve */
          v_start = v_last;
          limit--;
        }
        else
        {
          /* if both first and last points are conic, */
          /* start at their middle                    */
          v_start.x = ( v_start.x + v_last.x ) / 2;
          v_start.y = ( v_start.y + v_last.y ) / 2;
        }
        point--;
        tags--;
      }

      error = FT_TS_Stroker_BeginSubPath( stroker, &v_start, opened );
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


            vec.x = point->x;
            vec.y = point->y;

            error = FT_TS_Stroker_LineTo( stroker, &vec );
            if ( error )
              goto Exit;
            continue;
          }

        case FT_TS_CURVE_TAG_CONIC:  /* consume conic arcs */
          v_control.x = point->x;
          v_control.y = point->y;

        Do_Conic:
          if ( point < limit )
          {
            FT_TS_Vector  vec;
            FT_TS_Vector  v_middle;


            point++;
            tags++;
            tag = FT_TS_CURVE_TAG( tags[0] );

            vec = point[0];

            if ( tag == FT_TS_CURVE_TAG_ON )
            {
              error = FT_TS_Stroker_ConicTo( stroker, &v_control, &vec );
              if ( error )
                goto Exit;
              continue;
            }

            if ( tag != FT_TS_CURVE_TAG_CONIC )
              goto Invalid_Outline;

            v_middle.x = ( v_control.x + vec.x ) / 2;
            v_middle.y = ( v_control.y + vec.y ) / 2;

            error = FT_TS_Stroker_ConicTo( stroker, &v_control, &v_middle );
            if ( error )
              goto Exit;

            v_control = vec;
            goto Do_Conic;
          }

          error = FT_TS_Stroker_ConicTo( stroker, &v_control, &v_start );
          goto Close;

        default:  /* FT_TS_CURVE_TAG_CUBIC */
          {
            FT_TS_Vector  vec1, vec2;


            if ( point + 1 > limit                             ||
                 FT_TS_CURVE_TAG( tags[1] ) != FT_TS_CURVE_TAG_CUBIC )
              goto Invalid_Outline;

            point += 2;
            tags  += 2;

            vec1 = point[-2];
            vec2 = point[-1];

            if ( point <= limit )
            {
              FT_TS_Vector  vec;


              vec = point[0];

              error = FT_TS_Stroker_CubicTo( stroker, &vec1, &vec2, &vec );
              if ( error )
                goto Exit;
              continue;
            }

            error = FT_TS_Stroker_CubicTo( stroker, &vec1, &vec2, &v_start );
            goto Close;
          }
        }
      }

    Close:
      if ( error )
        goto Exit;

      /* don't try to end the path if no segments have been generated */
      if ( !stroker->first_point )
      {
        error = FT_TS_Stroker_EndSubPath( stroker );
        if ( error )
          goto Exit;
      }

      first = last + 1;
    }

    return FT_TS_Err_Ok;

  Exit:
    return error;

  Invalid_Outline:
    return FT_TS_THROW( Invalid_Outline );
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Glyph_Stroke( FT_TS_Glyph    *pglyph,
                   FT_TS_Stroker   stroker,
                   FT_TS_Bool      destroy )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );
    FT_TS_Glyph  glyph = NULL;


    if ( !pglyph )
      goto Exit;

    glyph = *pglyph;
    if ( !glyph || glyph->clazz != &ft_outline_glyph_class )
      goto Exit;

    {
      FT_TS_Glyph  copy;


      error = FT_TS_Glyph_Copy( glyph, &copy );
      if ( error )
        goto Exit;

      glyph = copy;
    }

    {
      FT_TS_OutlineGlyph  oglyph  = (FT_TS_OutlineGlyph)glyph;
      FT_TS_Outline*      outline = &oglyph->outline;
      FT_TS_UInt          num_points, num_contours;


      error = FT_TS_Stroker_ParseOutline( stroker, outline, FALSE );
      if ( error )
        goto Fail;

      FT_TS_Stroker_GetCounts( stroker, &num_points, &num_contours );

      FT_TS_Outline_Done( glyph->library, outline );

      error = FT_TS_Outline_New( glyph->library,
                              num_points,
                              (FT_TS_Int)num_contours,
                              outline );
      if ( error )
        goto Fail;

      outline->n_points   = 0;
      outline->n_contours = 0;

      FT_TS_Stroker_Export( stroker, outline );
    }

    if ( destroy )
      FT_TS_Done_Glyph( *pglyph );

    *pglyph = glyph;
    goto Exit;

  Fail:
    FT_TS_Done_Glyph( glyph );
    glyph = NULL;

    if ( !destroy )
      *pglyph = NULL;

  Exit:
    return error;
  }


  /* documentation is in ftstroke.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Glyph_StrokeBorder( FT_TS_Glyph    *pglyph,
                         FT_TS_Stroker   stroker,
                         FT_TS_Bool      inside,
                         FT_TS_Bool      destroy )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );
    FT_TS_Glyph  glyph = NULL;


    if ( !pglyph )
      goto Exit;

    glyph = *pglyph;
    if ( !glyph || glyph->clazz != &ft_outline_glyph_class )
      goto Exit;

    {
      FT_TS_Glyph  copy;


      error = FT_TS_Glyph_Copy( glyph, &copy );
      if ( error )
        goto Exit;

      glyph = copy;
    }

    {
      FT_TS_OutlineGlyph   oglyph  = (FT_TS_OutlineGlyph)glyph;
      FT_TS_StrokerBorder  border;
      FT_TS_Outline*       outline = &oglyph->outline;
      FT_TS_UInt           num_points, num_contours;


      border = FT_TS_Outline_GetOutsideBorder( outline );
      if ( inside )
      {
        if ( border == FT_TS_STROKER_BORDER_LEFT )
          border = FT_TS_STROKER_BORDER_RIGHT;
        else
          border = FT_TS_STROKER_BORDER_LEFT;
      }

      error = FT_TS_Stroker_ParseOutline( stroker, outline, FALSE );
      if ( error )
        goto Fail;

      FT_TS_Stroker_GetBorderCounts( stroker, border,
                                  &num_points, &num_contours );

      FT_TS_Outline_Done( glyph->library, outline );

      error = FT_TS_Outline_New( glyph->library,
                              num_points,
                              (FT_TS_Int)num_contours,
                              outline );
      if ( error )
        goto Fail;

      outline->n_points   = 0;
      outline->n_contours = 0;

      FT_TS_Stroker_ExportBorder( stroker, border, outline );
    }

    if ( destroy )
      FT_TS_Done_Glyph( *pglyph );

    *pglyph = glyph;
    goto Exit;

  Fail:
    FT_TS_Done_Glyph( glyph );
    glyph = NULL;

    if ( !destroy )
      *pglyph = NULL;

  Exit:
    return error;
  }


/* END */
