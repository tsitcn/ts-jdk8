/****************************************************************************
 *
 * ftgloadr.c
 *
 *   The FreeType glyph loader (body).
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


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftgloadr.h>
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftobjs.h>

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gloader


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****                    G L Y P H   L O A D E R                    *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /**************************************************************************
   *
   * The glyph loader is a simple object which is used to load a set of
   * glyphs easily.  It is critical for the correct loading of composites.
   *
   * Ideally, one can see it as a stack of abstract `glyph' objects.
   *
   *   loader.base     Is really the bottom of the stack.  It describes a
   *                   single glyph image made of the juxtaposition of
   *                   several glyphs (those `in the stack').
   *
   *   loader.current  Describes the top of the stack, on which a new
   *                   glyph can be loaded.
   *
   *   Rewind          Clears the stack.
   *   Prepare         Set up `loader.current' for addition of a new glyph
   *                   image.
   *   Add             Add the `current' glyph image to the `base' one,
   *                   and prepare for another one.
   *
   * The glyph loader is now a base object.  Each driver used to
   * re-implement it in one way or the other, which wasted code and
   * energy.
   *
   */


  /* create a new glyph loader */
  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_GlyphLoader_New( FT_TS_Memory        memory,
                      FT_TS_GlyphLoader  *aloader )
  {
    FT_TS_GlyphLoader  loader = NULL;
    FT_TS_Error        error;


    if ( !FT_TS_NEW( loader ) )
    {
      loader->memory = memory;
      *aloader       = loader;
    }
    return error;
  }


  /* rewind the glyph loader - reset counters to 0 */
  FT_TS_BASE_DEF( void )
  FT_TS_GlyphLoader_Rewind( FT_TS_GlyphLoader  loader )
  {
    FT_TS_GlyphLoad  base    = &loader->base;
    FT_TS_GlyphLoad  current = &loader->current;


    base->outline.n_points   = 0;
    base->outline.n_contours = 0;
    base->outline.flags      = 0;
    base->num_subglyphs      = 0;

    *current = *base;
  }


  /* reset glyph loader, free all allocated tables, */
  /* and start from zero                            */
  FT_TS_BASE_DEF( void )
  FT_TS_GlyphLoader_Reset( FT_TS_GlyphLoader  loader )
  {
    FT_TS_Memory  memory = loader->memory;


    FT_TS_FREE( loader->base.outline.points );
    FT_TS_FREE( loader->base.outline.tags );
    FT_TS_FREE( loader->base.outline.contours );
    FT_TS_FREE( loader->base.extra_points );
    FT_TS_FREE( loader->base.subglyphs );

    loader->base.extra_points2 = NULL;

    loader->max_points    = 0;
    loader->max_contours  = 0;
    loader->max_subglyphs = 0;

    FT_TS_GlyphLoader_Rewind( loader );
  }


  /* delete a glyph loader */
  FT_TS_BASE_DEF( void )
  FT_TS_GlyphLoader_Done( FT_TS_GlyphLoader  loader )
  {
    if ( loader )
    {
      FT_TS_Memory  memory = loader->memory;


      FT_TS_GlyphLoader_Reset( loader );
      FT_TS_FREE( loader );
    }
  }


  /* re-adjust the `current' outline fields */
  static void
  FT_TS_GlyphLoader_Adjust_Points( FT_TS_GlyphLoader  loader )
  {
    FT_TS_Outline*  base    = &loader->base.outline;
    FT_TS_Outline*  current = &loader->current.outline;


    current->points   = FT_TS_OFFSET( base->points,   base->n_points );
    current->tags     = FT_TS_OFFSET( base->tags,     base->n_points );
    current->contours = FT_TS_OFFSET( base->contours, base->n_contours );

    /* handle extra points table - if any */
    if ( loader->use_extra )
    {
      loader->current.extra_points  = loader->base.extra_points +
                                      base->n_points;

      loader->current.extra_points2 = loader->base.extra_points2 +
                                      base->n_points;
    }
  }


  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_GlyphLoader_CreateExtra( FT_TS_GlyphLoader  loader )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory = loader->memory;


    if ( loader->max_points == 0           ||
         loader->base.extra_points != NULL )
      return FT_TS_Err_Ok;

    if ( !FT_TS_NEW_ARRAY( loader->base.extra_points, 2 * loader->max_points ) )
    {
      loader->use_extra          = 1;
      loader->base.extra_points2 = loader->base.extra_points +
                                   loader->max_points;

      FT_TS_GlyphLoader_Adjust_Points( loader );
    }
    return error;
  }


  /* re-adjust the `current' subglyphs field */
  static void
  FT_TS_GlyphLoader_Adjust_Subglyphs( FT_TS_GlyphLoader  loader )
  {
    FT_TS_GlyphLoad  base    = &loader->base;
    FT_TS_GlyphLoad  current = &loader->current;


    current->subglyphs = FT_TS_OFFSET( base->subglyphs, base->num_subglyphs );
  }


  /* Ensure that we can add `n_points' and `n_contours' to our glyph.      */
  /* This function reallocates its outline tables if necessary.  Note that */
  /* it DOESN'T change the number of points within the loader!             */
  /*                                                                       */
  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_GlyphLoader_CheckPoints( FT_TS_GlyphLoader  loader,
                              FT_TS_UInt         n_points,
                              FT_TS_UInt         n_contours )
  {
    FT_TS_Memory    memory  = loader->memory;
    FT_TS_Error     error   = FT_TS_Err_Ok;
    FT_TS_Outline*  base    = &loader->base.outline;
    FT_TS_Outline*  current = &loader->current.outline;
    FT_TS_Bool      adjust  = 0;

    FT_TS_UInt      new_max, old_max;


    error = FT_TS_GlyphLoader_CreateExtra( loader );
    if ( error )
      return error;

    /* check points & tags */
    new_max = (FT_TS_UInt)base->n_points + (FT_TS_UInt)current->n_points +
              n_points;
    old_max = loader->max_points;

    if ( new_max > old_max )
    {
      new_max = FT_TS_PAD_CEIL( new_max, 8 );

      if ( new_max > FT_TS_OUTLINE_POINTS_MAX )
        return FT_TS_THROW( Array_Too_Large );

      if ( FT_TS_RENEW_ARRAY( base->points, old_max, new_max ) ||
           FT_TS_RENEW_ARRAY( base->tags,   old_max, new_max ) )
        goto Exit;

      if ( loader->use_extra )
      {
        if ( FT_TS_RENEW_ARRAY( loader->base.extra_points,
                             old_max * 2, new_max * 2 ) )
          goto Exit;

        FT_TS_ARRAY_MOVE( loader->base.extra_points + new_max,
                       loader->base.extra_points + old_max,
                       old_max );

        loader->base.extra_points2 = loader->base.extra_points + new_max;
      }

      adjust = 1;
      loader->max_points = new_max;
    }

    error = FT_TS_GlyphLoader_CreateExtra( loader );
    if ( error )
      return error;

    /* check contours */
    old_max = loader->max_contours;
    new_max = (FT_TS_UInt)base->n_contours + (FT_TS_UInt)current->n_contours +
              n_contours;
    if ( new_max > old_max )
    {
      new_max = FT_TS_PAD_CEIL( new_max, 4 );

      if ( new_max > FT_TS_OUTLINE_CONTOURS_MAX )
        return FT_TS_THROW( Array_Too_Large );

      if ( FT_TS_RENEW_ARRAY( base->contours, old_max, new_max ) )
        goto Exit;

      adjust = 1;
      loader->max_contours = new_max;
    }

    if ( adjust )
      FT_TS_GlyphLoader_Adjust_Points( loader );

  Exit:
    if ( error )
      FT_TS_GlyphLoader_Reset( loader );

    return error;
  }


  /* Ensure that we can add `n_subglyphs' to our glyph. this function */
  /* reallocates its subglyphs table if necessary.  Note that it DOES */
  /* NOT change the number of subglyphs within the loader!            */
  /*                                                                  */
  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_GlyphLoader_CheckSubGlyphs( FT_TS_GlyphLoader  loader,
                                 FT_TS_UInt         n_subs )
  {
    FT_TS_Memory     memory = loader->memory;
    FT_TS_Error      error  = FT_TS_Err_Ok;
    FT_TS_UInt       new_max, old_max;

    FT_TS_GlyphLoad  base    = &loader->base;
    FT_TS_GlyphLoad  current = &loader->current;


    new_max = base->num_subglyphs + current->num_subglyphs + n_subs;
    old_max = loader->max_subglyphs;
    if ( new_max > old_max )
    {
      new_max = FT_TS_PAD_CEIL( new_max, 2 );
      if ( FT_TS_RENEW_ARRAY( base->subglyphs, old_max, new_max ) )
        goto Exit;

      loader->max_subglyphs = new_max;

      FT_TS_GlyphLoader_Adjust_Subglyphs( loader );
    }

  Exit:
    return error;
  }


  /* prepare loader for the addition of a new glyph on top of the base one */
  FT_TS_BASE_DEF( void )
  FT_TS_GlyphLoader_Prepare( FT_TS_GlyphLoader  loader )
  {
    FT_TS_GlyphLoad  current = &loader->current;


    current->outline.n_points   = 0;
    current->outline.n_contours = 0;
    current->num_subglyphs      = 0;

    FT_TS_GlyphLoader_Adjust_Points   ( loader );
    FT_TS_GlyphLoader_Adjust_Subglyphs( loader );
  }


  /* add current glyph to the base image -- and prepare for another */
  FT_TS_BASE_DEF( void )
  FT_TS_GlyphLoader_Add( FT_TS_GlyphLoader  loader )
  {
    FT_TS_GlyphLoad  base;
    FT_TS_GlyphLoad  current;

    FT_TS_Int        n_curr_contours;
    FT_TS_Int        n_base_points;
    FT_TS_Int        n;


    if ( !loader )
      return;

    base    = &loader->base;
    current = &loader->current;

    n_curr_contours = current->outline.n_contours;
    n_base_points   = base->outline.n_points;

    base->outline.n_points =
      (short)( base->outline.n_points + current->outline.n_points );
    base->outline.n_contours =
      (short)( base->outline.n_contours + current->outline.n_contours );

    base->num_subglyphs += current->num_subglyphs;

    /* adjust contours count in newest outline */
    for ( n = 0; n < n_curr_contours; n++ )
      current->outline.contours[n] =
        (short)( current->outline.contours[n] + n_base_points );

    /* prepare for another new glyph image */
    FT_TS_GlyphLoader_Prepare( loader );
  }


/* END */
