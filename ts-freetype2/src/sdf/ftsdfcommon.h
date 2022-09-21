/****************************************************************************
 *
 * ftsdfcommon.h
 *
 *   Auxiliary data for Signed Distance Field support (specification).
 *
 * Copyright (C) 2020-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * Written by Anuj Verma.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /****************************************************
   *
   * This file contains common functions and properties
   * for both the 'sdf' and 'bsdf' renderers.
   *
   */

#ifndef FTSDFCOMMON_H_
#define FTSDFCOMMON_H_

#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/internal/ftobjs.h>


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * default values (cannot be set individually for each renderer)
   *
   */

  /* default spread value */
#define DEFAULT_SPREAD  8
  /* minimum spread supported by the renderer */
#define MIN_SPREAD      2
  /* maximum spread supported by the renderer */
#define MAX_SPREAD      32
  /* pixel size in 26.6 */
#define ONE_PIXEL       ( 1 << 6 )


  /**************************************************************************
   *
   * common definitions (cannot be set individually for each renderer)
   *
   */

  /* If this macro is set to 1 the rasterizer uses squared distances for */
  /* computation.  It can greatly improve the performance but there is a */
  /* chance of overflow and artifacts.  You can safely use it up to a    */
  /* pixel size of 128.                                                  */
#ifndef USE_SQUARED_DISTANCES
#define USE_SQUARED_DISTANCES  0
#endif


  /**************************************************************************
   *
   * common macros
   *
   */

  /* convert int to 26.6 fixed-point   */
#define FT_TS_INT_26D6( x )   ( x * 64 )
  /* convert int to 16.16 fixed-point  */
#define FT_TS_INT_16D16( x )  ( x * 65536 )
  /* convert 26.6 to 16.16 fixed-point */
#define FT_TS_26D6_16D16( x ) ( x * 1024 )


  /* Convenience macro to call a function; it  */
  /* jumps to label `Exit` if an error occurs. */
#define FT_TS_CALL( x ) do                          \
                     {                           \
                       error = ( x );            \
                       if ( error != FT_TS_Err_Ok ) \
                         goto Exit;              \
                     } while ( 0 )


  /*
   * The macro `VECTOR_LENGTH_16D16` computes either squared distances or
   * actual distances, depending on the value of `USE_SQUARED_DISTANCES`.
   *
   * By using squared distances the performance can be greatly improved but
   * there is a risk of overflow.
   */
#if USE_SQUARED_DISTANCES
#define VECTOR_LENGTH_16D16( v )  ( FT_TS_MulFix( v.x, v.x ) + \
                                    FT_TS_MulFix( v.y, v.y ) )
#else
#define VECTOR_LENGTH_16D16( v )  FT_TS_Vector_Length( &v )
#endif


  /**************************************************************************
   *
   * common typedefs
   *
   */

  typedef FT_TS_Vector FT_TS_26D6_Vec;   /* with 26.6 fixed-point components  */
  typedef FT_TS_Vector FT_TS_16D16_Vec;  /* with 16.16 fixed-point components */

  typedef FT_TS_Fixed  FT_TS_16D16;      /* 16.16 fixed-point representation  */
  typedef FT_TS_Fixed  FT_TS_26D6;       /* 26.6 fixed-point representation   */
  typedef FT_TS_Byte   FT_TS_SDFFormat;  /* format to represent SDF data      */

  typedef FT_TS_BBox   FT_TS_CBox;       /* control box of a curve            */


  FT_TS_LOCAL( FT_TS_16D16 )
  square_root( FT_TS_16D16  val );

  FT_TS_LOCAL( FT_TS_SDFFormat )
  map_fixed_to_sdf( FT_TS_16D16  dist,
                    FT_TS_16D16  max_value );

  FT_TS_LOCAL( FT_TS_SDFFormat )
  invert_sign( FT_TS_SDFFormat  dist );


FT_TS_END_HEADER

#endif /* FTSDFCOMMON_H_ */


/* END */
