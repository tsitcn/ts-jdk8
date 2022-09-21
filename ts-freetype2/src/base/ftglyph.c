/****************************************************************************
 *
 * ftglyph.c
 *
 *   FreeType convenience functions to handle glyphs (body).
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

  /**************************************************************************
   *
   * This file contains the definition of several convenience functions
   * that can be used by client applications to easily retrieve glyph
   * bitmaps and outlines from a given face.
   *
   * These functions should be optional if you are writing a font server
   * or text layout engine on top of FreeType.  However, they are pretty
   * handy for many other simple uses of the library.
   *
   */


#include <freetype/internal/ftdebug.h>

#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/ftbitmap.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/otsvg.h>

#include "ftbase.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  glyph


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   FT_TS_BitmapGlyph support                                        ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_bitmap_glyph_init( FT_TS_Glyph      bitmap_glyph,
                        FT_TS_GlyphSlot  slot )
  {
    FT_TS_BitmapGlyph  glyph   = (FT_TS_BitmapGlyph)bitmap_glyph;
    FT_TS_Error        error   = FT_TS_Err_Ok;
    FT_TS_Library      library = FT_TS_GLYPH( glyph )->library;


    if ( slot->format != FT_TS_GLYPH_FORMAT_BITMAP )
    {
      error = FT_TS_THROW( Invalid_Glyph_Format );
      goto Exit;
    }

    glyph->left = slot->bitmap_left;
    glyph->top  = slot->bitmap_top;

    /* do lazy copying whenever possible */
    if ( slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP )
    {
      glyph->bitmap          = slot->bitmap;
      slot->internal->flags &= ~FT_TS_GLYPH_OWN_BITMAP;
    }
    else
    {
      FT_TS_Bitmap_Init( &glyph->bitmap );
      error = FT_TS_Bitmap_Copy( library, &slot->bitmap, &glyph->bitmap );
    }

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_bitmap_glyph_copy( FT_TS_Glyph  bitmap_source,
                        FT_TS_Glyph  bitmap_target )
  {
    FT_TS_Library      library = bitmap_source->library;
    FT_TS_BitmapGlyph  source  = (FT_TS_BitmapGlyph)bitmap_source;
    FT_TS_BitmapGlyph  target  = (FT_TS_BitmapGlyph)bitmap_target;


    target->left = source->left;
    target->top  = source->top;

    return FT_TS_Bitmap_Copy( library, &source->bitmap, &target->bitmap );
  }


  FT_TS_CALLBACK_DEF( void )
  ft_bitmap_glyph_done( FT_TS_Glyph  bitmap_glyph )
  {
    FT_TS_BitmapGlyph  glyph   = (FT_TS_BitmapGlyph)bitmap_glyph;
    FT_TS_Library      library = FT_TS_GLYPH( glyph )->library;


    FT_TS_Bitmap_Done( library, &glyph->bitmap );
  }


  FT_TS_CALLBACK_DEF( void )
  ft_bitmap_glyph_bbox( FT_TS_Glyph  bitmap_glyph,
                        FT_TS_BBox*  cbox )
  {
    FT_TS_BitmapGlyph  glyph = (FT_TS_BitmapGlyph)bitmap_glyph;


    cbox->xMin = glyph->left * 64;
    cbox->xMax = cbox->xMin + (FT_TS_Pos)( glyph->bitmap.width * 64 );
    cbox->yMax = glyph->top * 64;
    cbox->yMin = cbox->yMax - (FT_TS_Pos)( glyph->bitmap.rows * 64 );
  }


  FT_TS_DEFINE_GLYPH(
    ft_bitmap_glyph_class,

    sizeof ( FT_TS_BitmapGlyphRec ),
    FT_TS_GLYPH_FORMAT_BITMAP,

    ft_bitmap_glyph_init,    /* FT_TS_Glyph_InitFunc       glyph_init      */
    ft_bitmap_glyph_done,    /* FT_TS_Glyph_DoneFunc       glyph_done      */
    ft_bitmap_glyph_copy,    /* FT_TS_Glyph_CopyFunc       glyph_copy      */
    NULL,                    /* FT_TS_Glyph_TransformFunc  glyph_transform */
    ft_bitmap_glyph_bbox,    /* FT_TS_Glyph_GetBBoxFunc    glyph_bbox      */
    NULL                     /* FT_TS_Glyph_PrepareFunc    glyph_prepare   */
  )


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   FT_TS_OutlineGlyph support                                       ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_outline_glyph_init( FT_TS_Glyph      outline_glyph,
                         FT_TS_GlyphSlot  slot )
  {
    FT_TS_OutlineGlyph  glyph   = (FT_TS_OutlineGlyph)outline_glyph;
    FT_TS_Error         error   = FT_TS_Err_Ok;
    FT_TS_Library       library = FT_TS_GLYPH( glyph )->library;
    FT_TS_Outline*      source  = &slot->outline;
    FT_TS_Outline*      target  = &glyph->outline;


    /* check format in glyph slot */
    if ( slot->format != FT_TS_GLYPH_FORMAT_OUTLINE )
    {
      error = FT_TS_THROW( Invalid_Glyph_Format );
      goto Exit;
    }

    /* allocate new outline */
    error = FT_TS_Outline_New( library,
                            (FT_TS_UInt)source->n_points,
                            source->n_contours,
                            &glyph->outline );
    if ( error )
      goto Exit;

    FT_TS_Outline_Copy( source, target );

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( void )
  ft_outline_glyph_done( FT_TS_Glyph  outline_glyph )
  {
    FT_TS_OutlineGlyph  glyph = (FT_TS_OutlineGlyph)outline_glyph;


    FT_TS_Outline_Done( FT_TS_GLYPH( glyph )->library, &glyph->outline );
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_outline_glyph_copy( FT_TS_Glyph  outline_source,
                         FT_TS_Glyph  outline_target )
  {
    FT_TS_OutlineGlyph  source  = (FT_TS_OutlineGlyph)outline_source;
    FT_TS_OutlineGlyph  target  = (FT_TS_OutlineGlyph)outline_target;
    FT_TS_Error         error;
    FT_TS_Library       library = FT_TS_GLYPH( source )->library;


    error = FT_TS_Outline_New( library,
                            (FT_TS_UInt)source->outline.n_points,
                            source->outline.n_contours,
                            &target->outline );
    if ( !error )
      FT_TS_Outline_Copy( &source->outline, &target->outline );

    return error;
  }


  FT_TS_CALLBACK_DEF( void )
  ft_outline_glyph_transform( FT_TS_Glyph          outline_glyph,
                              const FT_TS_Matrix*  matrix,
                              const FT_TS_Vector*  delta )
  {
    FT_TS_OutlineGlyph  glyph = (FT_TS_OutlineGlyph)outline_glyph;


    if ( matrix )
      FT_TS_Outline_Transform( &glyph->outline, matrix );

    if ( delta )
      FT_TS_Outline_Translate( &glyph->outline, delta->x, delta->y );
  }


  FT_TS_CALLBACK_DEF( void )
  ft_outline_glyph_bbox( FT_TS_Glyph  outline_glyph,
                         FT_TS_BBox*  bbox )
  {
    FT_TS_OutlineGlyph  glyph = (FT_TS_OutlineGlyph)outline_glyph;


    FT_TS_Outline_Get_CBox( &glyph->outline, bbox );
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_outline_glyph_prepare( FT_TS_Glyph      outline_glyph,
                            FT_TS_GlyphSlot  slot )
  {
    FT_TS_OutlineGlyph  glyph = (FT_TS_OutlineGlyph)outline_glyph;


    slot->format         = FT_TS_GLYPH_FORMAT_OUTLINE;
    slot->outline        = glyph->outline;
    slot->outline.flags &= ~FT_TS_OUTLINE_OWNER;

    return FT_TS_Err_Ok;
  }


  FT_TS_DEFINE_GLYPH(
    ft_outline_glyph_class,

    sizeof ( FT_TS_OutlineGlyphRec ),
    FT_TS_GLYPH_FORMAT_OUTLINE,

    ft_outline_glyph_init,      /* FT_TS_Glyph_InitFunc       glyph_init      */
    ft_outline_glyph_done,      /* FT_TS_Glyph_DoneFunc       glyph_done      */
    ft_outline_glyph_copy,      /* FT_TS_Glyph_CopyFunc       glyph_copy      */
    ft_outline_glyph_transform, /* FT_TS_Glyph_TransformFunc  glyph_transform */
    ft_outline_glyph_bbox,      /* FT_TS_Glyph_GetBBoxFunc    glyph_bbox      */
    ft_outline_glyph_prepare    /* FT_TS_Glyph_PrepareFunc    glyph_prepare   */
  )


#ifdef FT_TS_CONFIG_OPTION_SVG

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   FT_TS_SvgGlyph support                                           ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_svg_glyph_init( FT_TS_Glyph      svg_glyph,
                     FT_TS_GlyphSlot  slot )
  {
    FT_TS_ULong         doc_length;
    FT_TS_SVG_Document  document;
    FT_TS_SvgGlyph      glyph = (FT_TS_SvgGlyph)svg_glyph;

    FT_TS_Error   error  = FT_TS_Err_Ok;
    FT_TS_Memory  memory = FT_TS_GLYPH( glyph )->library->memory;


    if ( slot->format != FT_TS_GLYPH_FORMAT_SVG )
    {
      error = FT_TS_THROW( Invalid_Glyph_Format );
      goto Exit;
    }

    if ( slot->other == NULL )
    {
      error = FT_TS_THROW( Invalid_Slot_Handle );
      goto Exit;
    }

    document = (FT_TS_SVG_Document)slot->other;

    if ( document->svg_document_length == 0 )
    {
      error = FT_TS_THROW( Invalid_Slot_Handle );
      goto Exit;
    }

    /* allocate a new document */
    doc_length = document->svg_document_length;
    if ( FT_TS_QALLOC( glyph->svg_document, doc_length ) )
      goto Exit;
    glyph->svg_document_length = doc_length;

    glyph->glyph_index = slot->glyph_index;

    glyph->metrics      = document->metrics;
    glyph->units_per_EM = document->units_per_EM;

    glyph->start_glyph_id = document->start_glyph_id;
    glyph->end_glyph_id   = document->end_glyph_id;

    glyph->transform = document->transform;
    glyph->delta     = document->delta;

    /* copy the document into glyph */
    FT_TS_MEM_COPY( glyph->svg_document, document->svg_document, doc_length );

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( void )
  ft_svg_glyph_done( FT_TS_Glyph  svg_glyph )
  {
    FT_TS_SvgGlyph  glyph  = (FT_TS_SvgGlyph)svg_glyph;
    FT_TS_Memory    memory = svg_glyph->library->memory;


    /* just free the memory */
    FT_TS_FREE( glyph->svg_document );
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_svg_glyph_copy( FT_TS_Glyph  svg_source,
                     FT_TS_Glyph  svg_target )
  {
    FT_TS_SvgGlyph  source = (FT_TS_SvgGlyph)svg_source;
    FT_TS_SvgGlyph  target = (FT_TS_SvgGlyph)svg_target;

    FT_TS_Error   error  = FT_TS_Err_Ok;
    FT_TS_Memory  memory = FT_TS_GLYPH( source )->library->memory;


    if ( svg_source->format != FT_TS_GLYPH_FORMAT_SVG )
    {
      error = FT_TS_THROW( Invalid_Glyph_Format );
      goto Exit;
    }

    if ( source->svg_document_length == 0 )
    {
      error = FT_TS_THROW( Invalid_Slot_Handle );
      goto Exit;
    }

    target->glyph_index = source->glyph_index;

    target->svg_document_length = source->svg_document_length;

    target->metrics      = source->metrics;
    target->units_per_EM = source->units_per_EM;

    target->start_glyph_id = source->start_glyph_id;
    target->end_glyph_id   = source->end_glyph_id;

    target->transform = source->transform;
    target->delta     = source->delta;

    /* allocate space for the SVG document */
    if ( FT_TS_QALLOC( target->svg_document, target->svg_document_length ) )
      goto Exit;

    /* copy the document */
    FT_TS_MEM_COPY( target->svg_document,
                 source->svg_document,
                 target->svg_document_length );

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( void )
  ft_svg_glyph_transform( FT_TS_Glyph          svg_glyph,
                          const FT_TS_Matrix*  _matrix,
                          const FT_TS_Vector*  _delta )
  {
    FT_TS_SvgGlyph  glyph  = (FT_TS_SvgGlyph)svg_glyph;
    FT_TS_Matrix*   matrix = (FT_TS_Matrix*)_matrix;
    FT_TS_Vector*   delta  = (FT_TS_Vector*)_delta;

    FT_TS_Matrix  tmp_matrix;
    FT_TS_Vector  tmp_delta;

    FT_TS_Matrix  a, b;
    FT_TS_Pos     x, y;


    if ( !matrix )
    {
      tmp_matrix.xx = 0x10000;
      tmp_matrix.xy = 0;
      tmp_matrix.yx = 0;
      tmp_matrix.yy = 0x10000;

      matrix = &tmp_matrix;
    }

    if ( !delta )
    {
      tmp_delta.x = 0;
      tmp_delta.y = 0;

      delta = &tmp_delta;
    }

    a = glyph->transform;
    b = *matrix;
    FT_TS_Matrix_Multiply( &b, &a );

    x = ADD_LONG( ADD_LONG( FT_TS_MulFix( matrix->xx, glyph->delta.x ),
                            FT_TS_MulFix( matrix->xy, glyph->delta.y ) ),
                  delta->x );
    y = ADD_LONG( ADD_LONG( FT_TS_MulFix( matrix->yx, glyph->delta.x ),
                            FT_TS_MulFix( matrix->yy, glyph->delta.y ) ),
                  delta->y );

    glyph->delta.x = x;
    glyph->delta.y = y;

    glyph->transform = a;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  ft_svg_glyph_prepare( FT_TS_Glyph      svg_glyph,
                        FT_TS_GlyphSlot  slot )
  {
    FT_TS_SvgGlyph  glyph = (FT_TS_SvgGlyph)svg_glyph;

    FT_TS_Error   error  = FT_TS_Err_Ok;
    FT_TS_Memory  memory = svg_glyph->library->memory;

    FT_TS_SVG_Document  document = NULL;


    if ( FT_TS_NEW( document ) )
      return error;

    document->svg_document        = glyph->svg_document;
    document->svg_document_length = glyph->svg_document_length;

    document->metrics      = glyph->metrics;
    document->units_per_EM = glyph->units_per_EM;

    document->start_glyph_id = glyph->start_glyph_id;
    document->end_glyph_id   = glyph->end_glyph_id;

    document->transform = glyph->transform;
    document->delta     = glyph->delta;

    slot->format      = FT_TS_GLYPH_FORMAT_SVG;
    slot->glyph_index = glyph->glyph_index;
    slot->other       = document;

    return error;
  }


  FT_TS_DEFINE_GLYPH(
    ft_svg_glyph_class,

    sizeof ( FT_TS_SvgGlyphRec ),
    FT_TS_GLYPH_FORMAT_SVG,

    ft_svg_glyph_init,      /* FT_TS_Glyph_InitFunc       glyph_init      */
    ft_svg_glyph_done,      /* FT_TS_Glyph_DoneFunc       glyph_done      */
    ft_svg_glyph_copy,      /* FT_TS_Glyph_CopyFunc       glyph_copy      */
    ft_svg_glyph_transform, /* FT_TS_Glyph_TransformFunc  glyph_transform */
    NULL,                   /* FT_TS_Glyph_GetBBoxFunc    glyph_bbox      */
    ft_svg_glyph_prepare    /* FT_TS_Glyph_PrepareFunc    glyph_prepare   */
  )

#endif /* FT_TS_CONFIG_OPTION_SVG */


  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****   FT_TS_Glyph class and API                                        ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

   static FT_TS_Error
   ft_new_glyph( FT_TS_Library             library,
                 const FT_TS_Glyph_Class*  clazz,
                 FT_TS_Glyph*              aglyph )
   {
     FT_TS_Memory  memory = library->memory;
     FT_TS_Error   error;
     FT_TS_Glyph   glyph  = NULL;


     *aglyph = NULL;

     if ( !FT_TS_ALLOC( glyph, clazz->glyph_size ) )
     {
       glyph->library = library;
       glyph->clazz   = clazz;
       glyph->format  = clazz->glyph_format;

       *aglyph = glyph;
     }

     return error;
   }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Glyph_Copy( FT_TS_Glyph   source,
                 FT_TS_Glyph  *target )
  {
    FT_TS_Glyph               copy;
    FT_TS_Error               error;
    const FT_TS_Glyph_Class*  clazz;


    /* check arguments */
    if ( !target || !source || !source->clazz )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    *target = NULL;

    if ( !source || !source->clazz )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    clazz = source->clazz;
    error = ft_new_glyph( source->library, clazz, &copy );
    if ( error )
      goto Exit;

    copy->advance = source->advance;
    copy->format  = source->format;

    if ( clazz->glyph_copy )
      error = clazz->glyph_copy( source, copy );

    if ( error )
      FT_TS_Done_Glyph( copy );
    else
      *target = copy;

  Exit:
    return error;
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_New_Glyph( FT_TS_Library       library,
                FT_TS_Glyph_Format  format,
                FT_TS_Glyph        *aglyph )
  {
    const FT_TS_Glyph_Class*  clazz = NULL;

    if ( !library || !aglyph )
      return FT_TS_THROW( Invalid_Argument );

    /* if it is a bitmap, that's easy :-) */
    if ( format == FT_TS_GLYPH_FORMAT_BITMAP )
      clazz = &ft_bitmap_glyph_class;

    /* if it is an outline */
    else if ( format == FT_TS_GLYPH_FORMAT_OUTLINE )
      clazz = &ft_outline_glyph_class;

#ifdef FT_TS_CONFIG_OPTION_SVG
    /* if it is an SVG glyph */
    else if ( format == FT_TS_GLYPH_FORMAT_SVG )
      clazz = &ft_svg_glyph_class;
#endif

    else
    {
      /* try to find a renderer that supports the glyph image format */
      FT_TS_Renderer  render = FT_TS_Lookup_Renderer( library, format, 0 );


      if ( render )
        clazz = &render->glyph_class;
    }

    if ( !clazz )
      return FT_TS_THROW( Invalid_Glyph_Format );

    /* create FT_TS_Glyph object */
    return ft_new_glyph( library, clazz, aglyph );
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Glyph( FT_TS_GlyphSlot  slot,
                FT_TS_Glyph     *aglyph )
  {
    FT_TS_Error  error;
    FT_TS_Glyph  glyph;


    if ( !slot )
      return FT_TS_THROW( Invalid_Slot_Handle );

    if ( !aglyph )
      return FT_TS_THROW( Invalid_Argument );

    /* create FT_TS_Glyph object */
    error = FT_TS_New_Glyph( slot->library, slot->format, &glyph );
    if ( error )
      goto Exit;

    /* copy advance while converting 26.6 to 16.16 format */
    if ( slot->advance.x >=  0x8000L * 64 ||
         slot->advance.x <= -0x8000L * 64 )
    {
      FT_TS_ERROR(( "FT_TS_Get_Glyph: advance width too large\n" ));
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit2;
    }
    if ( slot->advance.y >=  0x8000L * 64 ||
         slot->advance.y <= -0x8000L * 64 )
    {
      FT_TS_ERROR(( "FT_TS_Get_Glyph: advance height too large\n" ));
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit2;
    }

    glyph->advance.x = slot->advance.x * 1024;
    glyph->advance.y = slot->advance.y * 1024;

    /* now import the image from the glyph slot */
    error = glyph->clazz->glyph_init( glyph, slot );

  Exit2:
    /* if an error occurred, destroy the glyph */
    if ( error )
      FT_TS_Done_Glyph( glyph );
    else
      *aglyph = glyph;

  Exit:
    return error;
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Glyph_Transform( FT_TS_Glyph          glyph,
                      const FT_TS_Matrix*  matrix,
                      const FT_TS_Vector*  delta )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    if ( !glyph || !glyph->clazz )
      error = FT_TS_THROW( Invalid_Argument );
    else
    {
      const FT_TS_Glyph_Class*  clazz = glyph->clazz;


      if ( clazz->glyph_transform )
      {
        /* transform glyph image */
        clazz->glyph_transform( glyph, matrix, delta );

        /* transform advance vector */
        if ( matrix )
          FT_TS_Vector_Transform( &glyph->advance, matrix );
      }
      else
        error = FT_TS_THROW( Invalid_Glyph_Format );
    }
    return error;
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Glyph_Get_CBox( FT_TS_Glyph  glyph,
                     FT_TS_UInt   bbox_mode,
                     FT_TS_BBox  *acbox )
  {
    const FT_TS_Glyph_Class*  clazz;


    if ( !acbox )
      return;

    acbox->xMin = acbox->yMin = acbox->xMax = acbox->yMax = 0;

    if ( !glyph || !glyph->clazz )
      return;

    clazz = glyph->clazz;
    if ( !clazz->glyph_bbox )
      return;

    /* retrieve bbox in 26.6 coordinates */
    clazz->glyph_bbox( glyph, acbox );

    /* perform grid fitting if needed */
    if ( bbox_mode == FT_TS_GLYPH_BBOX_GRIDFIT ||
         bbox_mode == FT_TS_GLYPH_BBOX_PIXELS  )
    {
      acbox->xMin = FT_TS_PIX_FLOOR( acbox->xMin );
      acbox->yMin = FT_TS_PIX_FLOOR( acbox->yMin );
      acbox->xMax = FT_TS_PIX_CEIL_LONG( acbox->xMax );
      acbox->yMax = FT_TS_PIX_CEIL_LONG( acbox->yMax );
    }

    /* convert to integer pixels if needed */
    if ( bbox_mode == FT_TS_GLYPH_BBOX_TRUNCATE ||
         bbox_mode == FT_TS_GLYPH_BBOX_PIXELS   )
    {
      acbox->xMin >>= 6;
      acbox->yMin >>= 6;
      acbox->xMax >>= 6;
      acbox->yMax >>= 6;
    }
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Glyph_To_Bitmap( FT_TS_Glyph*         the_glyph,
                      FT_TS_Render_Mode    render_mode,
                      const FT_TS_Vector*  origin,
                      FT_TS_Bool           destroy )
  {
    FT_TS_GlyphSlotRec           dummy;
    FT_TS_GlyphSlot_InternalRec  dummy_internal;
    FT_TS_Error                  error = FT_TS_Err_Ok;
    FT_TS_Glyph                  b, glyph;
    FT_TS_BitmapGlyph            bitmap = NULL;
    const FT_TS_Glyph_Class*     clazz;

    FT_TS_Library                library;


    /* check argument */
    if ( !the_glyph )
      goto Bad;
    glyph = *the_glyph;
    if ( !glyph )
      goto Bad;

    clazz   = glyph->clazz;
    library = glyph->library;
    if ( !library || !clazz )
      goto Bad;

    /* when called with a bitmap glyph, do nothing and return successfully */
    if ( clazz == &ft_bitmap_glyph_class )
      goto Exit;

    if ( !clazz->glyph_prepare )
      goto Bad;

    /* we render the glyph into a glyph bitmap using a `dummy' glyph slot */
    /* then calling FT_TS_Render_Glyph_Internal()                            */

    FT_TS_ZERO( &dummy );
    FT_TS_ZERO( &dummy_internal );
    dummy.internal = &dummy_internal;
    dummy.library  = library;
    dummy.format   = clazz->glyph_format;

    /* create result bitmap glyph */
    error = ft_new_glyph( library, &ft_bitmap_glyph_class, &b );
    if ( error )
      goto Exit;
    bitmap = (FT_TS_BitmapGlyph)b;

#if 1
    /* if `origin' is set, translate the glyph image */
    if ( origin )
      FT_TS_Glyph_Transform( glyph, NULL, origin );
#else
    FT_TS_UNUSED( origin );
#endif

    /* prepare dummy slot for rendering */
    error = clazz->glyph_prepare( glyph, &dummy );
    if ( !error )
      error = FT_TS_Render_Glyph_Internal( glyph->library, &dummy, render_mode );

#ifdef FT_TS_CONFIG_OPTION_SVG
    if ( clazz == &ft_svg_glyph_class )
    {
      FT_TS_Memory  memory = library->memory;


      FT_TS_FREE( dummy.other );
    }
#endif

#if 1
    if ( !destroy && origin )
    {
      FT_TS_Vector  v;


      v.x = -origin->x;
      v.y = -origin->y;
      FT_TS_Glyph_Transform( glyph, NULL, &v );
    }
#endif

    if ( error )
      goto Exit;

    /* in case of success, copy the bitmap to the glyph bitmap */
    error = ft_bitmap_glyph_init( (FT_TS_Glyph)bitmap, &dummy );
    if ( error )
      goto Exit;

    /* copy advance */
    bitmap->root.advance = glyph->advance;

    if ( destroy )
      FT_TS_Done_Glyph( glyph );

    *the_glyph = FT_TS_GLYPH( bitmap );

  Exit:
    if ( error && bitmap )
      FT_TS_Done_Glyph( FT_TS_GLYPH( bitmap ) );

    return error;

  Bad:
    error = FT_TS_THROW( Invalid_Argument );
    goto Exit;
  }


  /* documentation is in ftglyph.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Done_Glyph( FT_TS_Glyph  glyph )
  {
    if ( glyph )
    {
      FT_TS_Memory              memory = glyph->library->memory;
      const FT_TS_Glyph_Class*  clazz  = glyph->clazz;


      if ( clazz->glyph_done )
        clazz->glyph_done( glyph );

      FT_TS_FREE( glyph );
    }
  }


/* END */
