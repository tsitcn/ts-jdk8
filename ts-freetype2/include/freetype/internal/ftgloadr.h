/****************************************************************************
 *
 * ftgloadr.h
 *
 *   The FreeType glyph loader (specification).
 *
 * Copyright (C) 2002-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTGLOADR_H_
#define FTGLOADR_H_


#include <freetype/freetype.h>

#include "compiler-macros.h"

FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_GlyphLoader
   *
   * @description:
   *   The glyph loader is an internal object used to load several glyphs
   *   together (for example, in the case of composites).
   */
  typedef struct  FT_TS_SubGlyphRec_
  {
    FT_TS_Int     index;
    FT_TS_UShort  flags;
    FT_TS_Int     arg1;
    FT_TS_Int     arg2;
    FT_TS_Matrix  transform;

  } FT_TS_SubGlyphRec;


  typedef struct  FT_TS_GlyphLoadRec_
  {
    FT_TS_Outline   outline;       /* outline                   */
    FT_TS_Vector*   extra_points;  /* extra points table        */
    FT_TS_Vector*   extra_points2; /* second extra points table */
    FT_TS_UInt      num_subglyphs; /* number of subglyphs       */
    FT_TS_SubGlyph  subglyphs;     /* subglyphs                 */

  } FT_TS_GlyphLoadRec, *FT_TS_GlyphLoad;


  typedef struct  FT_TS_GlyphLoaderRec_
  {
    FT_TS_Memory        memory;
    FT_TS_UInt          max_points;
    FT_TS_UInt          max_contours;
    FT_TS_UInt          max_subglyphs;
    FT_TS_Bool          use_extra;

    FT_TS_GlyphLoadRec  base;
    FT_TS_GlyphLoadRec  current;

    void*            other;            /* for possible future extension? */

  } FT_TS_GlyphLoaderRec, *FT_TS_GlyphLoader;


  /* create new empty glyph loader */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_GlyphLoader_New( FT_TS_Memory        memory,
                      FT_TS_GlyphLoader  *aloader );

  /* add an extra points table to a glyph loader */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_GlyphLoader_CreateExtra( FT_TS_GlyphLoader  loader );

  /* destroy a glyph loader */
  FT_TS_BASE( void )
  FT_TS_GlyphLoader_Done( FT_TS_GlyphLoader  loader );

  /* reset a glyph loader (frees everything int it) */
  FT_TS_BASE( void )
  FT_TS_GlyphLoader_Reset( FT_TS_GlyphLoader  loader );

  /* rewind a glyph loader */
  FT_TS_BASE( void )
  FT_TS_GlyphLoader_Rewind( FT_TS_GlyphLoader  loader );

  /* check that there is enough space to add `n_points' and `n_contours' */
  /* to the glyph loader                                                 */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_GlyphLoader_CheckPoints( FT_TS_GlyphLoader  loader,
                              FT_TS_UInt         n_points,
                              FT_TS_UInt         n_contours );


#define FT_TS_GLYPHLOADER_CHECK_P( _loader, _count )       \
  ( (_count) == 0                                    || \
    ( (FT_TS_UInt)(_loader)->base.outline.n_points    +    \
      (FT_TS_UInt)(_loader)->current.outline.n_points +    \
      (FT_TS_UInt)(_count) ) <= (_loader)->max_points   )

#define FT_TS_GLYPHLOADER_CHECK_C( _loader, _count )         \
  ( (_count) == 0                                      || \
    ( (FT_TS_UInt)(_loader)->base.outline.n_contours    +    \
      (FT_TS_UInt)(_loader)->current.outline.n_contours +    \
      (FT_TS_UInt)(_count) ) <= (_loader)->max_contours   )

#define FT_TS_GLYPHLOADER_CHECK_POINTS( _loader, _points, _contours ) \
  ( ( FT_TS_GLYPHLOADER_CHECK_P( _loader, _points )   &&              \
      FT_TS_GLYPHLOADER_CHECK_C( _loader, _contours ) )               \
    ? 0                                                            \
    : FT_TS_GlyphLoader_CheckPoints( (_loader),                       \
                                  (FT_TS_UInt)(_points),              \
                                  (FT_TS_UInt)(_contours) ) )


  /* check that there is enough space to add `n_subs' sub-glyphs to */
  /* a glyph loader                                                 */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_GlyphLoader_CheckSubGlyphs( FT_TS_GlyphLoader  loader,
                                 FT_TS_UInt         n_subs );

  /* prepare a glyph loader, i.e. empty the current glyph */
  FT_TS_BASE( void )
  FT_TS_GlyphLoader_Prepare( FT_TS_GlyphLoader  loader );

  /* add the current glyph to the base glyph */
  FT_TS_BASE( void )
  FT_TS_GlyphLoader_Add( FT_TS_GlyphLoader  loader );


FT_TS_END_HEADER

#endif /* FTGLOADR_H_ */


/* END */
