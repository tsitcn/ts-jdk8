/****************************************************************************
 *
 * ftobjs.c
 *
 *   The FreeType private base classes (body).
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


#include <freetype/ftlist.h>
#include <freetype/ftoutln.h>
#include <freetype/ftfntfmt.h>
#include <freetype/otsvg.h>

#include <freetype/internal/ftvalid.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftrfork.h>
#include <freetype/internal/ftstream.h>
#include <freetype/internal/sfnt.h>          /* for SFNT_Load_Table_Func */
#include <freetype/internal/psaux.h>         /* for PS_Driver            */
#include <freetype/internal/svginterface.h>

#include <freetype/tttables.h>
#include <freetype/tttags.h>
#include <freetype/ttnameid.h>

#include <freetype/internal/services/svprop.h>
#include <freetype/internal/services/svsfnt.h>
#include <freetype/internal/services/svpostnm.h>
#include <freetype/internal/services/svgldict.h>
#include <freetype/internal/services/svttcmap.h>
#include <freetype/internal/services/svkern.h>
#include <freetype/internal/services/svtteng.h>

#include <freetype/ftdriver.h>
#include <freetype/ftsynth.h>

#include <freetype/ftbitmap.h>

#ifdef FT_TS_CONFIG_OPTION_MAC_FONTS
#include "ftbase.h"
#endif


#ifdef FT_TS_DEBUG_LEVEL_TRACE

#include <freetype/ftbitmap.h>

#if defined( _MSC_VER )      /* Visual C++ (and Intel C++)   */
  /* We disable the warning `conversion from XXX to YYY,     */
  /* possible loss of data' in order to compile cleanly with */
  /* the maximum level of warnings: `md5.c' is non-FreeType  */
  /* code, and it gets used during development builds only.  */
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif /* _MSC_VER */

  /* It's easiest to include `md5.c' directly.  However, since OpenSSL */
  /* also provides the same functions, there might be conflicts if     */
  /* both FreeType and OpenSSL are built as static libraries.  For     */
  /* this reason, we put the MD5 stuff into the `FT_TS_' namespace.       */
#define MD5_u32plus  FT_TS_MD5_u32plus
#define MD5_CTX      FT_TS_MD5_CTX
#define MD5_Init     FT_TS_MD5_Init
#define MD5_Update   FT_TS_MD5_Update
#define MD5_Final    FT_TS_MD5_Final

#undef  HAVE_OPENSSL

#include "md5.c"

#if defined( _MSC_VER )
#pragma warning( pop )
#endif

  /* This array must stay in sync with the @FT_TS_Pixel_Mode enumeration */
  /* (in file `ftimage.h`).                                           */

  static const char* const  pixel_modes[] =
  {
    "none",
    "monochrome bitmap",
    "gray 8-bit bitmap",
    "gray 2-bit bitmap",
    "gray 4-bit bitmap",
    "LCD 8-bit bitmap",
    "vertical LCD 8-bit bitmap",
    "BGRA 32-bit color image bitmap",
    "SDF 8-bit bitmap"
  };

#endif /* FT_TS_DEBUG_LEVEL_TRACE */


#define GRID_FIT_METRICS


  /* forward declaration */
  static FT_TS_Error
  ft_open_face_internal( FT_TS_Library           library,
                         const FT_TS_Open_Args*  args,
                         FT_TS_Long              face_index,
                         FT_TS_Face             *aface,
                         FT_TS_Bool              test_mac_fonts );


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_service_list_lookup( FT_TS_ServiceDesc  service_descriptors,
                          const char*     service_id )
  {
    FT_TS_Pointer      result = NULL;
    FT_TS_ServiceDesc  desc   = service_descriptors;


    if ( desc && service_id )
    {
      for ( ; desc->serv_id != NULL; desc++ )
      {
        if ( ft_strcmp( desc->serv_id, service_id ) == 0 )
        {
          result = (FT_TS_Pointer)desc->serv_data;
          break;
        }
      }
    }

    return result;
  }


  FT_TS_BASE_DEF( void )
  ft_validator_init( FT_TS_Validator        valid,
                     const FT_TS_Byte*      base,
                     const FT_TS_Byte*      limit,
                     FT_TS_ValidationLevel  level )
  {
    valid->base  = base;
    valid->limit = limit;
    valid->level = level;
    valid->error = FT_TS_Err_Ok;
  }


  FT_TS_BASE_DEF( FT_TS_Int )
  ft_validator_run( FT_TS_Validator  valid )
  {
    /* This function doesn't work!  None should call it. */
    FT_TS_UNUSED( valid );

    return -1;
  }


  FT_TS_BASE_DEF( void )
  ft_validator_error( FT_TS_Validator  valid,
                      FT_TS_Error      error )
  {
    /* since the cast below also disables the compiler's */
    /* type check, we introduce a dummy variable, which  */
    /* will be optimized away                            */
    volatile ft_jmp_buf* jump_buffer = &valid->jump_buffer;


    valid->error = error;

    /* throw away volatileness; use `jump_buffer' or the  */
    /* compiler may warn about an unused local variable   */
    ft_longjmp( *(ft_jmp_buf*) jump_buffer, 1 );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           S T R E A M                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* create a new input stream from an FT_TS_Open_Args structure */
  /*                                                          */
  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Stream_New( FT_TS_Library           library,
                 const FT_TS_Open_Args*  args,
                 FT_TS_Stream           *astream )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory;
    FT_TS_Stream  stream = NULL;
    FT_TS_UInt    mode;


    *astream = NULL;

    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !args )
      return FT_TS_THROW( Invalid_Argument );

    memory = library->memory;
    mode   = args->flags &
               ( FT_TS_OPEN_MEMORY | FT_TS_OPEN_STREAM | FT_TS_OPEN_PATHNAME );

    if ( mode == FT_TS_OPEN_MEMORY )
    {
      /* create a memory-based stream */
      if ( FT_TS_NEW( stream ) )
        goto Exit;

      FT_TS_Stream_OpenMemory( stream,
                            (const FT_TS_Byte*)args->memory_base,
                            (FT_TS_ULong)args->memory_size );
      stream->memory = memory;
    }

#ifndef FT_TS_CONFIG_OPTION_DISABLE_STREAM_SUPPORT

    else if ( mode == FT_TS_OPEN_PATHNAME )
    {
      /* create a normal system stream */
      if ( FT_TS_NEW( stream ) )
        goto Exit;

      stream->memory = memory;
      error = FT_TS_Stream_Open( stream, args->pathname );
      if ( error )
        FT_TS_FREE( stream );
    }
    else if ( ( mode == FT_TS_OPEN_STREAM ) && args->stream )
    {
      /* use an existing, user-provided stream */

      /* in this case, we do not need to allocate a new stream object */
      /* since the caller is responsible for closing it himself       */
      stream         = args->stream;
      stream->memory = memory;
      error          = FT_TS_Err_Ok;
    }

#endif

    else
    {
      error = FT_TS_THROW( Invalid_Argument );
      if ( ( args->flags & FT_TS_OPEN_STREAM ) && args->stream )
        FT_TS_Stream_Close( args->stream );
    }

    if ( !error )
      *astream       = stream;

  Exit:
    return error;
  }


  FT_TS_BASE_DEF( void )
  FT_TS_Stream_Free( FT_TS_Stream  stream,
                  FT_TS_Int     external )
  {
    if ( stream )
    {
      FT_TS_Memory  memory = stream->memory;


      FT_TS_Stream_Close( stream );

      if ( !external )
        FT_TS_FREE( stream );
    }
  }


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  objs


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****               FACE, SIZE & GLYPH SLOT OBJECTS                   ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  static FT_TS_Error
  ft_glyphslot_init( FT_TS_GlyphSlot  slot )
  {
    FT_TS_Driver         driver   = slot->face->driver;
    FT_TS_Driver_Class   clazz    = driver->clazz;
    FT_TS_Memory         memory   = driver->root.memory;
    FT_TS_Error          error    = FT_TS_Err_Ok;
    FT_TS_Slot_Internal  internal = NULL;


    slot->library = driver->root.library;

    if ( FT_TS_NEW( internal ) )
      goto Exit;

    slot->internal = internal;

    if ( FT_TS_DRIVER_USES_OUTLINES( driver ) )
      error = FT_TS_GlyphLoader_New( memory, &internal->loader );

    if ( !error && clazz->init_slot )
      error = clazz->init_slot( slot );

#ifdef FT_TS_CONFIG_OPTION_SVG
    /* if SVG table exists, allocate the space in `slot->other` */
    if ( slot->face->face_flags & FT_TS_FACE_FLAG_SVG )
    {
      FT_TS_SVG_Document  document = NULL;


      if ( FT_TS_NEW( document ) )
        goto Exit;
      slot->other = document;
    }
#endif

  Exit:
    return error;
  }


  FT_TS_BASE_DEF( void )
  ft_glyphslot_free_bitmap( FT_TS_GlyphSlot  slot )
  {
    if ( slot->internal && ( slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP ) )
    {
      FT_TS_Memory  memory = FT_TS_FACE_MEMORY( slot->face );


      FT_TS_FREE( slot->bitmap.buffer );
      slot->internal->flags &= ~FT_TS_GLYPH_OWN_BITMAP;
    }
    else
    {
      /* assume that the bitmap buffer was stolen or not */
      /* allocated from the heap                         */
      slot->bitmap.buffer = NULL;
    }
  }


  /* overflow-resistant presetting of bitmap position and dimensions; */
  /* also check whether the size is too large for rendering           */
  FT_TS_BASE_DEF( FT_TS_Bool )
  ft_glyphslot_preset_bitmap( FT_TS_GlyphSlot      slot,
                              FT_TS_Render_Mode    mode,
                              const FT_TS_Vector*  origin )
  {
    FT_TS_Outline*  outline = &slot->outline;
    FT_TS_Bitmap*   bitmap  = &slot->bitmap;

    FT_TS_Pixel_Mode  pixel_mode;

    FT_TS_BBox  cbox, pbox;
    FT_TS_Pos   x_shift = 0;
    FT_TS_Pos   y_shift = 0;
    FT_TS_Pos   x_left, y_top;
    FT_TS_Pos   width, height, pitch;


    if ( slot->format == FT_TS_GLYPH_FORMAT_SVG )
    {
      FT_TS_Module    module;
      SVG_Service  svg_service;


      module      = FT_TS_Get_Module( slot->library, "ot-svg" );
      svg_service = (SVG_Service)module->clazz->module_interface;

      return (FT_TS_Bool)svg_service->preset_slot( module, slot, FALSE );
    }
    else if ( slot->format != FT_TS_GLYPH_FORMAT_OUTLINE )
      return 1;

    if ( origin )
    {
      x_shift = origin->x;
      y_shift = origin->y;
    }

    /* compute the control box, and grid-fit it, */
    /* taking into account the origin shift      */
    FT_TS_Outline_Get_CBox( outline, &cbox );

    /* rough estimate of pixel box */
    pbox.xMin = ( cbox.xMin >> 6 ) + ( x_shift >> 6 );
    pbox.yMin = ( cbox.yMin >> 6 ) + ( y_shift >> 6 );
    pbox.xMax = ( cbox.xMax >> 6 ) + ( x_shift >> 6 );
    pbox.yMax = ( cbox.yMax >> 6 ) + ( y_shift >> 6 );

    /* tiny remainder box */
    cbox.xMin = ( cbox.xMin & 63 ) + ( x_shift & 63 );
    cbox.yMin = ( cbox.yMin & 63 ) + ( y_shift & 63 );
    cbox.xMax = ( cbox.xMax & 63 ) + ( x_shift & 63 );
    cbox.yMax = ( cbox.yMax & 63 ) + ( y_shift & 63 );

    switch ( mode )
    {
    case FT_TS_RENDER_MODE_MONO:
      pixel_mode = FT_TS_PIXEL_MODE_MONO;
#if 1
      /* x */

      /* undocumented but confirmed: bbox values get rounded;    */
      /* we do asymmetric rounding so that the center of a pixel */
      /* gets always included                                    */

      pbox.xMin += ( cbox.xMin + 31 ) >> 6;
      pbox.xMax += ( cbox.xMax + 32 ) >> 6;

      /* if the bbox collapsed, we add a pixel based on the total */
      /* rounding remainder to cover most of the original cbox    */

      if ( pbox.xMin == pbox.xMax )
      {
        if ( ( ( cbox.xMin + 31 ) & 63 ) - 31 +
             ( ( cbox.xMax + 32 ) & 63 ) - 32 < 0 )
          pbox.xMin -= 1;
        else
          pbox.xMax += 1;
      }

      /* y */

      pbox.yMin += ( cbox.yMin + 31 ) >> 6;
      pbox.yMax += ( cbox.yMax + 32 ) >> 6;

      if ( pbox.yMin == pbox.yMax )
      {
        if ( ( ( cbox.yMin + 31 ) & 63 ) - 31 +
             ( ( cbox.yMax + 32 ) & 63 ) - 32 < 0 )
          pbox.yMin -= 1;
        else
          pbox.yMax += 1;
      }

      break;
#else
      goto Adjust;
#endif

    case FT_TS_RENDER_MODE_LCD:
      pixel_mode = FT_TS_PIXEL_MODE_LCD;
      ft_lcd_padding( &cbox, slot, mode );
      goto Adjust;

    case FT_TS_RENDER_MODE_LCD_V:
      pixel_mode = FT_TS_PIXEL_MODE_LCD_V;
      ft_lcd_padding( &cbox, slot, mode );
      goto Adjust;

    case FT_TS_RENDER_MODE_NORMAL:
    case FT_TS_RENDER_MODE_LIGHT:
    default:
      pixel_mode = FT_TS_PIXEL_MODE_GRAY;
    Adjust:
      pbox.xMin += cbox.xMin >> 6;
      pbox.yMin += cbox.yMin >> 6;
      pbox.xMax += ( cbox.xMax + 63 ) >> 6;
      pbox.yMax += ( cbox.yMax + 63 ) >> 6;
    }

    x_left = pbox.xMin;
    y_top  = pbox.yMax;

    width  = pbox.xMax - pbox.xMin;
    height = pbox.yMax - pbox.yMin;

    switch ( pixel_mode )
    {
    case FT_TS_PIXEL_MODE_MONO:
      pitch = ( ( width + 15 ) >> 4 ) << 1;
      break;

    case FT_TS_PIXEL_MODE_LCD:
      width *= 3;
      pitch  = FT_TS_PAD_CEIL( width, 4 );
      break;

    case FT_TS_PIXEL_MODE_LCD_V:
      height *= 3;
      /* fall through */

    case FT_TS_PIXEL_MODE_GRAY:
    default:
      pitch = width;
    }

    slot->bitmap_left = (FT_TS_Int)x_left;
    slot->bitmap_top  = (FT_TS_Int)y_top;

    bitmap->pixel_mode = (unsigned char)pixel_mode;
    bitmap->num_grays  = 256;
    bitmap->width      = (unsigned int)width;
    bitmap->rows       = (unsigned int)height;
    bitmap->pitch      = pitch;

    if ( pbox.xMin < -0x8000 || pbox.xMax > 0x7FFF ||
         pbox.yMin < -0x8000 || pbox.yMax > 0x7FFF )
    {
      FT_TS_TRACE3(( "ft_glyphslot_preset_bitmap: [%ld %ld %ld %ld]\n",
                  pbox.xMin, pbox.yMin, pbox.xMax, pbox.yMax ));
      return 1;
    }

    return 0;
  }


  FT_TS_BASE_DEF( void )
  ft_glyphslot_set_bitmap( FT_TS_GlyphSlot  slot,
                           FT_TS_Byte*      buffer )
  {
    ft_glyphslot_free_bitmap( slot );

    slot->bitmap.buffer = buffer;

    FT_TS_ASSERT( (slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP) == 0 );
  }


  FT_TS_BASE_DEF( FT_TS_Error )
  ft_glyphslot_alloc_bitmap( FT_TS_GlyphSlot  slot,
                             FT_TS_ULong      size )
  {
    FT_TS_Memory  memory = FT_TS_FACE_MEMORY( slot->face );
    FT_TS_Error   error;


    if ( slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP )
      FT_TS_FREE( slot->bitmap.buffer );
    else
      slot->internal->flags |= FT_TS_GLYPH_OWN_BITMAP;

    FT_TS_MEM_ALLOC( slot->bitmap.buffer, size );
    return error;
  }


  static void
  ft_glyphslot_clear( FT_TS_GlyphSlot  slot )
  {
    /* free bitmap if needed */
    ft_glyphslot_free_bitmap( slot );

    /* clear all public fields in the glyph slot */
    slot->glyph_index = 0;

    FT_TS_ZERO( &slot->metrics );
    FT_TS_ZERO( &slot->outline );

    slot->bitmap.width      = 0;
    slot->bitmap.rows       = 0;
    slot->bitmap.pitch      = 0;
    slot->bitmap.pixel_mode = 0;
    /* `slot->bitmap.buffer' has been handled by ft_glyphslot_free_bitmap */

    slot->bitmap_left   = 0;
    slot->bitmap_top    = 0;
    slot->num_subglyphs = 0;
    slot->subglyphs     = NULL;
    slot->control_data  = NULL;
    slot->control_len   = 0;

#ifndef FT_TS_CONFIG_OPTION_SVG
    slot->other = NULL;
#else
    if ( !( slot->face->face_flags & FT_TS_FACE_FLAG_SVG ) )
      slot->other = NULL;
    else
    {
      if ( slot->internal->flags & FT_TS_GLYPH_OWN_GZIP_SVG )
      {
        FT_TS_Memory        memory = slot->face->memory;
        FT_TS_SVG_Document  doc    = (FT_TS_SVG_Document)slot->other;


        FT_TS_FREE( doc->svg_document );
        slot->internal->load_flags &= ~FT_TS_GLYPH_OWN_GZIP_SVG;
      }
    }
#endif

    slot->format = FT_TS_GLYPH_FORMAT_NONE;

    slot->linearHoriAdvance = 0;
    slot->linearVertAdvance = 0;
    slot->advance.x         = 0;
    slot->advance.y         = 0;
    slot->lsb_delta         = 0;
    slot->rsb_delta         = 0;
  }


  static void
  ft_glyphslot_done( FT_TS_GlyphSlot  slot )
  {
    FT_TS_Driver        driver = slot->face->driver;
    FT_TS_Driver_Class  clazz  = driver->clazz;
    FT_TS_Memory        memory = driver->root.memory;

#ifdef FT_TS_CONFIG_OPTION_SVG
    if ( slot->face->face_flags & FT_TS_FACE_FLAG_SVG )
    {
      /* free memory in case SVG was there */
      if ( slot->internal->flags & FT_TS_GLYPH_OWN_GZIP_SVG )
      {
        FT_TS_SVG_Document  doc = (FT_TS_SVG_Document)slot->other;


        FT_TS_FREE( doc->svg_document );

        slot->internal->flags &= ~FT_TS_GLYPH_OWN_GZIP_SVG;
      }

      FT_TS_FREE( slot->other );
    }
#endif

    if ( clazz->done_slot )
      clazz->done_slot( slot );

    /* free bitmap buffer if needed */
    ft_glyphslot_free_bitmap( slot );

    /* slot->internal might be NULL in out-of-memory situations */
    if ( slot->internal )
    {
      /* free glyph loader */
      if ( FT_TS_DRIVER_USES_OUTLINES( driver ) )
      {
        FT_TS_GlyphLoader_Done( slot->internal->loader );
        slot->internal->loader = NULL;
      }

      FT_TS_FREE( slot->internal );
    }
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_New_GlyphSlot( FT_TS_Face        face,
                    FT_TS_GlyphSlot  *aslot )
  {
    FT_TS_Error         error;
    FT_TS_Driver        driver;
    FT_TS_Driver_Class  clazz;
    FT_TS_Memory        memory;
    FT_TS_GlyphSlot     slot = NULL;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !face->driver )
      return FT_TS_THROW( Invalid_Argument );

    driver = face->driver;
    clazz  = driver->clazz;
    memory = driver->root.memory;

    FT_TS_TRACE4(( "FT_TS_New_GlyphSlot: Creating new slot object\n" ));
    if ( !FT_TS_ALLOC( slot, clazz->slot_object_size ) )
    {
      slot->face = face;

      error = ft_glyphslot_init( slot );
      if ( error )
      {
        ft_glyphslot_done( slot );
        FT_TS_FREE( slot );
        goto Exit;
      }

      slot->next  = face->glyph;
      face->glyph = slot;

      if ( aslot )
        *aslot = slot;
    }
    else if ( aslot )
      *aslot = NULL;


  Exit:
    FT_TS_TRACE4(( "FT_TS_New_GlyphSlot: Return 0x%x\n", error ));

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( void )
  FT_TS_Done_GlyphSlot( FT_TS_GlyphSlot  slot )
  {
    if ( slot )
    {
      FT_TS_Driver     driver = slot->face->driver;
      FT_TS_Memory     memory = driver->root.memory;
      FT_TS_GlyphSlot  prev;
      FT_TS_GlyphSlot  cur;


      /* Remove slot from its parent face's list */
      prev = NULL;
      cur  = slot->face->glyph;

      while ( cur )
      {
        if ( cur == slot )
        {
          if ( !prev )
            slot->face->glyph = cur->next;
          else
            prev->next = cur->next;

          /* finalize client-specific data */
          if ( slot->generic.finalizer )
            slot->generic.finalizer( slot );

          ft_glyphslot_done( slot );
          FT_TS_FREE( slot );
          break;
        }
        prev = cur;
        cur  = cur->next;
      }
    }
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Set_Transform( FT_TS_Face     face,
                    FT_TS_Matrix*  matrix,
                    FT_TS_Vector*  delta )
  {
    FT_TS_Face_Internal  internal;


    if ( !face )
      return;

    internal = face->internal;

    internal->transform_flags = 0;

    if ( !matrix )
    {
      internal->transform_matrix.xx = 0x10000L;
      internal->transform_matrix.xy = 0;
      internal->transform_matrix.yx = 0;
      internal->transform_matrix.yy = 0x10000L;

      matrix = &internal->transform_matrix;
    }
    else
      internal->transform_matrix = *matrix;

    /* set transform_flags bit flag 0 if `matrix' isn't the identity */
    if ( ( matrix->xy | matrix->yx ) ||
         matrix->xx != 0x10000L      ||
         matrix->yy != 0x10000L      )
      internal->transform_flags |= 1;

    if ( !delta )
    {
      internal->transform_delta.x = 0;
      internal->transform_delta.y = 0;

      delta = &internal->transform_delta;
    }
    else
      internal->transform_delta = *delta;

    /* set transform_flags bit flag 1 if `delta' isn't the null vector */
    if ( delta->x | delta->y )
      internal->transform_flags |= 2;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Get_Transform( FT_TS_Face     face,
                    FT_TS_Matrix*  matrix,
                    FT_TS_Vector*  delta )
  {
    FT_TS_Face_Internal  internal;


    if ( !face )
      return;

    internal = face->internal;

    if ( matrix )
      *matrix = internal->transform_matrix;

    if ( delta )
      *delta = internal->transform_delta;
  }


  static FT_TS_Renderer
  ft_lookup_glyph_renderer( FT_TS_GlyphSlot  slot );


#ifdef GRID_FIT_METRICS
  static void
  ft_glyphslot_grid_fit_metrics( FT_TS_GlyphSlot  slot,
                                 FT_TS_Bool       vertical )
  {
    FT_TS_Glyph_Metrics*  metrics = &slot->metrics;
    FT_TS_Pos             right, bottom;


    if ( vertical )
    {
      metrics->horiBearingX = FT_TS_PIX_FLOOR( metrics->horiBearingX );
      metrics->horiBearingY = FT_TS_PIX_CEIL_LONG( metrics->horiBearingY );

      right  = FT_TS_PIX_CEIL_LONG( ADD_LONG( metrics->vertBearingX,
                                           metrics->width ) );
      bottom = FT_TS_PIX_CEIL_LONG( ADD_LONG( metrics->vertBearingY,
                                           metrics->height ) );

      metrics->vertBearingX = FT_TS_PIX_FLOOR( metrics->vertBearingX );
      metrics->vertBearingY = FT_TS_PIX_FLOOR( metrics->vertBearingY );

      metrics->width  = SUB_LONG( right,
                                  metrics->vertBearingX );
      metrics->height = SUB_LONG( bottom,
                                  metrics->vertBearingY );
    }
    else
    {
      metrics->vertBearingX = FT_TS_PIX_FLOOR( metrics->vertBearingX );
      metrics->vertBearingY = FT_TS_PIX_FLOOR( metrics->vertBearingY );

      right  = FT_TS_PIX_CEIL_LONG( ADD_LONG( metrics->horiBearingX,
                                           metrics->width ) );
      bottom = FT_TS_PIX_FLOOR( SUB_LONG( metrics->horiBearingY,
                                       metrics->height ) );

      metrics->horiBearingX = FT_TS_PIX_FLOOR( metrics->horiBearingX );
      metrics->horiBearingY = FT_TS_PIX_CEIL_LONG( metrics->horiBearingY );

      metrics->width  = SUB_LONG( right,
                                  metrics->horiBearingX );
      metrics->height = SUB_LONG( metrics->horiBearingY,
                                  bottom );
    }

    metrics->horiAdvance = FT_TS_PIX_ROUND_LONG( metrics->horiAdvance );
    metrics->vertAdvance = FT_TS_PIX_ROUND_LONG( metrics->vertAdvance );
  }
#endif /* GRID_FIT_METRICS */


  /* documentation is in freetype.h */

/**
 * TSIT {{{{{{{{{{
 */
  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Load_Glyph( FT_TS_Face   face,
                 FT_TS_UInt   glyph_index,
                 FT_TS_Int32  load_flags )
  {
      return FT_TS_Load_Glyph_Office(face, glyph_index, load_flags, 0);
  }

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Load_Glyph_Office( FT_TS_Face   face,
                 FT_TS_UInt   glyph_index,
                 FT_TS_Int32  load_flags,
                 FT_TS_Int32  office_flags)
  {
/**
 * TSIT }}}}}}}}}}
 */
    FT_TS_Error      error;
    FT_TS_Driver     driver;
    FT_TS_GlyphSlot  slot;
    FT_TS_Library    library;
    FT_TS_Bool       autohint = FALSE;
    FT_TS_Module     hinter;
    TT_Face       ttface = (TT_Face)face;


    if ( !face || !face->size || !face->glyph )
      return FT_TS_THROW( Invalid_Face_Handle );

    /* The validity test for `glyph_index' is performed by the */
    /* font drivers.                                           */

    slot = face->glyph;
    ft_glyphslot_clear( slot );

    driver  = face->driver;
    library = driver->root.library;
    hinter  = library->auto_hinter;

    /* undefined scale means no scale */
    if ( face->size->metrics.x_ppem == 0 ||
         face->size->metrics.y_ppem == 0 )
      load_flags |= FT_TS_LOAD_NO_SCALE;

    /* resolve load flags dependencies */

    if ( load_flags & FT_TS_LOAD_NO_RECURSE )
      load_flags |= FT_TS_LOAD_NO_SCALE         |
                    FT_TS_LOAD_IGNORE_TRANSFORM;

    if ( load_flags & FT_TS_LOAD_NO_SCALE )
    {
      load_flags |= FT_TS_LOAD_NO_HINTING |
                    FT_TS_LOAD_NO_BITMAP;

      load_flags &= ~FT_TS_LOAD_RENDER;
    }

    if ( load_flags & FT_TS_LOAD_BITMAP_METRICS_ONLY )
      load_flags &= ~FT_TS_LOAD_RENDER;

    /*
     * Determine whether we need to auto-hint or not.
     * The general rules are:
     *
     * - Do only auto-hinting if we have
     *
     *   - a hinter module,
     *   - a scalable font,
     *   - not a tricky font, and
     *   - no transforms except simple slants and/or rotations by
     *     integer multiples of 90 degrees.
     *
     * - Then, auto-hint if FT_TS_LOAD_FORCE_AUTOHINT is set or if we don't
     *   have a native font hinter.
     *
     * - Otherwise, auto-hint for LIGHT hinting mode or if there isn't
     *   any hinting bytecode in the TrueType/OpenType font.
     *
     * - Exception: The font is `tricky' and requires the native hinter to
     *   load properly.
     */

    if ( hinter                                           &&
         !( load_flags & FT_TS_LOAD_NO_HINTING )             &&
         !( load_flags & FT_TS_LOAD_NO_AUTOHINT )            &&
         FT_TS_IS_SCALABLE( face )                           &&
         !FT_TS_IS_TRICKY( face )                            &&
         ( ( load_flags & FT_TS_LOAD_IGNORE_TRANSFORM )    ||
           ( face->internal->transform_matrix.yx == 0 &&
             face->internal->transform_matrix.xx != 0 ) ||
           ( face->internal->transform_matrix.xx == 0 &&
             face->internal->transform_matrix.yx != 0 ) ) )
    {
      if ( ( load_flags & FT_TS_LOAD_FORCE_AUTOHINT ) ||
           !FT_TS_DRIVER_HAS_HINTER( driver )         )
        autohint = TRUE;
      else
      {
        FT_TS_Render_Mode  mode = FT_TS_LOAD_TARGET_MODE( load_flags );
        FT_TS_Bool         is_light_type1;


        /* only the new Adobe engine (for both CFF and Type 1) is `light'; */
        /* we use `strstr' to catch both `Type 1' and `CID Type 1'         */
        is_light_type1 =
          ft_strstr( FT_TS_Get_Font_Format( face ), "Type 1" ) != NULL &&
          ((PS_Driver)driver)->hinting_engine == FT_TS_HINTING_ADOBE;

        /* the check for `num_locations' assures that we actually    */
        /* test for instructions in a TTF and not in a CFF-based OTF */
        /*                                                           */
        /* since `maxSizeOfInstructions' might be unreliable, we     */
        /* check the size of the `fpgm' and `prep' tables, too --    */
        /* the assumption is that there don't exist real TTFs where  */
        /* both `fpgm' and `prep' tables are missing                 */
        if ( ( mode == FT_TS_RENDER_MODE_LIGHT           &&
               ( !FT_TS_DRIVER_HINTS_LIGHTLY( driver ) &&
                 !is_light_type1                    ) )         ||
             ( FT_TS_IS_SFNT( face )                             &&
               ttface->num_locations                          &&
               ttface->max_profile.maxSizeOfInstructions == 0 &&
               ttface->font_program_size == 0                 &&
               ttface->cvt_program_size == 0                  ) )
          autohint = TRUE;
      }
    }

    if ( autohint )
    {
      FT_TS_AutoHinter_Interface  hinting;


      /* XXX: The use of the `FT_TS_LOAD_XXX_ONLY` flags is not very */
      /*      elegant.                                            */

      /* try to load SVG documents if available */
      if ( FT_TS_HAS_SVG( face ) )
      {
        error = driver->clazz->load_glyph( slot, face->size,
                                           glyph_index,
                                           load_flags | FT_TS_LOAD_SVG_ONLY );

        if ( !error && slot->format == FT_TS_GLYPH_FORMAT_SVG )
          goto Load_Ok;
      }

      /* try to load embedded bitmaps if available */
      if ( FT_TS_HAS_FIXED_SIZES( face )              &&
           ( load_flags & FT_TS_LOAD_NO_BITMAP ) == 0 )
      {
        error = driver->clazz->load_glyph( slot, face->size,
                                           glyph_index,
                                           load_flags | FT_TS_LOAD_SBITS_ONLY );

        if ( !error && slot->format == FT_TS_GLYPH_FORMAT_BITMAP )
          goto Load_Ok;
      }

      {
        FT_TS_Face_Internal  internal        = face->internal;
        FT_TS_Int            transform_flags = internal->transform_flags;


        /* since the auto-hinter calls FT_TS_Load_Glyph by itself, */
        /* make sure that glyphs aren't transformed             */
        internal->transform_flags = 0;

        /* load auto-hinted outline */
        hinting = (FT_TS_AutoHinter_Interface)hinter->clazz->module_interface;

        error   = hinting->load_glyph( (FT_TS_AutoHinter)hinter,
                                       slot, face->size,
                                       glyph_index, load_flags );

        internal->transform_flags = transform_flags;
      }
    }
    else
    {
      error = driver->clazz->load_glyph( slot,
                                         face->size,
                                         glyph_index,
                                         load_flags );
      if ( error )
        goto Exit;

      if ( slot->format == FT_TS_GLYPH_FORMAT_OUTLINE )
      {
        /* check that the loaded outline is correct */
        error = FT_TS_Outline_Check( &slot->outline );
        if ( error )
          goto Exit;

#ifdef GRID_FIT_METRICS
        if ( !( load_flags & FT_TS_LOAD_NO_HINTING ) )
          ft_glyphslot_grid_fit_metrics(
            slot,
            FT_TS_BOOL( load_flags & FT_TS_LOAD_VERTICAL_LAYOUT ) );
#endif
      }
    }

  Load_Ok:
    /* compute the advance */
    if ( load_flags & FT_TS_LOAD_VERTICAL_LAYOUT )
    {
      slot->advance.x = 0;
      slot->advance.y = slot->metrics.vertAdvance;
    }
    else
    {
      slot->advance.x = slot->metrics.horiAdvance;
      slot->advance.y = 0;
    }

    /* compute the linear advance in 16.16 pixels */
    if ( ( load_flags & FT_TS_LOAD_LINEAR_DESIGN ) == 0 &&
         FT_TS_IS_SCALABLE( face )                      )
    {
      FT_TS_Size_Metrics*  metrics = &face->size->metrics;


      /* it's tricky! */
      slot->linearHoriAdvance = FT_TS_MulDiv( slot->linearHoriAdvance,
                                           metrics->x_scale, 64 );

      slot->linearVertAdvance = FT_TS_MulDiv( slot->linearVertAdvance,
                                           metrics->y_scale, 64 );
    }

    if ( ( load_flags & FT_TS_LOAD_IGNORE_TRANSFORM ) == 0 )
    {
      FT_TS_Face_Internal  internal = face->internal;

/**
 TSIT {{{{{{{{{{
 */

      /* now, transform the glyph image if needed */
      if ( internal->transform_flags
          || FT_TS_CHECK_FLIP_L2R(office_flags)
          || FT_TS_CHECK_FLIP_T2B(office_flags))
      {
        /* get renderer */
        FT_TS_Renderer  renderer = ft_lookup_glyph_renderer( slot );
        int degree = FT_TS_GlyphSlot_Get_Degree_From_Slot(slot);
        /* if bitmap and rotate right degrees(like 90, 180, 270), use FT_TS_GlyphSlot_Transform to do it. */
        if ( renderer
#if defined(FT_TS_BITMAP_RIGHT_ANGLE_ENABLE)
             && (slot->format != FT_TS_GLYPH_FORMAT_BITMAP || !FT_TS_GlyphSlot_Is_Valid_BitmapDegree(degree))
#endif
            )
        {
          FT_TS_Matrix  transform_matrix = {0};
          
          if (FT_TS_CHECK_FLIP_L2R(office_flags))
          {
              transform_matrix.xx = -0x10000L;
              transform_matrix.yx =  0;
              transform_matrix.xy =  0;
              transform_matrix.yy =  0x10000L;
              FT_TS_Outline_Transform( &slot->outline, &transform_matrix );
          }

          if (FT_TS_CHECK_FLIP_T2B(office_flags))
          {
              transform_matrix.xx =  0x10000L;
              transform_matrix.yx =  0;
              transform_matrix.xy =  0;
              transform_matrix.yy = -0x10000L;
              FT_TS_Outline_Transform( &slot->outline, &transform_matrix );
          }

          error = renderer->clazz->transform_glyph(
                                     renderer, slot,
                                     &internal->transform_matrix,
                                     &internal->transform_delta );
        }
        else if ( slot->format == FT_TS_GLYPH_FORMAT_OUTLINE )
        {
          /* apply `standard' transformation if no renderer is available */
          if ( internal->transform_flags & 1 )
            FT_TS_Outline_Transform( &slot->outline,
                                  &internal->transform_matrix );

          if ( internal->transform_flags & 2 )
            FT_TS_Outline_Translate( &slot->outline,
                                  internal->transform_delta.x,
                                  internal->transform_delta.y );
        }
        else
        {
            FT_TS_Bitmap_Load_Glyph(slot->library, &slot->bitmap, slot, load_flags, office_flags);
        }

/**
 TSIT }}}}}}}}}}
 */
 
        /* transform advance */
        FT_TS_Vector_Transform( &slot->advance, &internal->transform_matrix );
      }
    }

    slot->glyph_index          = glyph_index;
    slot->internal->load_flags = load_flags;

    /* do we need to render the image or preset the bitmap now? */
    if ( !error                                    &&
         ( load_flags & FT_TS_LOAD_NO_SCALE ) == 0    &&
         slot->format != FT_TS_GLYPH_FORMAT_BITMAP    &&
         slot->format != FT_TS_GLYPH_FORMAT_COMPOSITE )
    {
      FT_TS_Render_Mode  mode = FT_TS_LOAD_TARGET_MODE( load_flags );


      if ( mode == FT_TS_RENDER_MODE_NORMAL   &&
           load_flags & FT_TS_LOAD_MONOCHROME )
        mode = FT_TS_RENDER_MODE_MONO;

      if ( load_flags & FT_TS_LOAD_RENDER )
        error = FT_TS_Render_Glyph( slot, mode );
      else
        ft_glyphslot_preset_bitmap( slot, mode, NULL );
    }

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    FT_TS_TRACE5(( "FT_TS_Load_Glyph: index %d, flags 0x%x\n",
                glyph_index, load_flags ));
    FT_TS_TRACE5(( "  bitmap %dx%d %s, %s (mode %d)\n",
                slot->bitmap.width,
                slot->bitmap.rows,
                slot->outline.points ?
                  slot->bitmap.buffer ? "rendered"
                                      : "preset"
                                     :
                  slot->internal->flags & FT_TS_GLYPH_OWN_BITMAP ? "owned"
                                                              : "unowned",
                pixel_modes[slot->bitmap.pixel_mode],
                slot->bitmap.pixel_mode ));
    FT_TS_TRACE5(( "\n" ));
    FT_TS_TRACE5(( "  x advance: %f\n", slot->advance.x / 64.0 ));
    FT_TS_TRACE5(( "  y advance: %f\n", slot->advance.y / 64.0 ));
    FT_TS_TRACE5(( "  linear x advance: %f\n",
                slot->linearHoriAdvance / 65536.0 ));
    FT_TS_TRACE5(( "  linear y advance: %f\n",
                slot->linearVertAdvance / 65536.0 ));

    {
      FT_TS_Glyph_Metrics*  metrics = &slot->metrics;


      FT_TS_TRACE5(( "  metrics:\n" ));
      FT_TS_TRACE5(( "    width:  %f\n", metrics->width  / 64.0 ));
      FT_TS_TRACE5(( "    height: %f\n", metrics->height / 64.0 ));
      FT_TS_TRACE5(( "\n" ));
      FT_TS_TRACE5(( "    horiBearingX: %f\n", metrics->horiBearingX / 64.0 ));
      FT_TS_TRACE5(( "    horiBearingY: %f\n", metrics->horiBearingY / 64.0 ));
      FT_TS_TRACE5(( "    horiAdvance:  %f\n", metrics->horiAdvance  / 64.0 ));
      FT_TS_TRACE5(( "\n" ));
      FT_TS_TRACE5(( "    vertBearingX: %f\n", metrics->vertBearingX / 64.0 ));
      FT_TS_TRACE5(( "    vertBearingY: %f\n", metrics->vertBearingY / 64.0 ));
      FT_TS_TRACE5(( "    vertAdvance:  %f\n", metrics->vertAdvance  / 64.0 ));
    }
#endif

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Load_Char( FT_TS_Face   face,
                FT_TS_ULong  char_code,
                FT_TS_Int32  load_flags )
  {
    FT_TS_UInt  glyph_index;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    glyph_index = (FT_TS_UInt)char_code;
    if ( face->charmap )
      glyph_index = FT_TS_Get_Char_Index( face, char_code );

    return FT_TS_Load_Glyph( face, glyph_index, load_flags );
  }


  /* destructor for sizes list */
  static void
  destroy_size( FT_TS_Memory  memory,
                FT_TS_Size    size,
                FT_TS_Driver  driver )
  {
    /* finalize client-specific data */
    if ( size->generic.finalizer )
      size->generic.finalizer( size );

    /* finalize format-specific stuff */
    if ( driver->clazz->done_size )
      driver->clazz->done_size( size );

    FT_TS_FREE( size->internal );
    FT_TS_FREE( size );
  }


  static void
  ft_cmap_done_internal( FT_TS_CMap  cmap );


  static void
  destroy_charmaps( FT_TS_Face    face,
                    FT_TS_Memory  memory )
  {
    FT_TS_Int  n;


    if ( !face )
      return;

    for ( n = 0; n < face->num_charmaps; n++ )
    {
      FT_TS_CMap  cmap = FT_TS_CMAP( face->charmaps[n] );


      ft_cmap_done_internal( cmap );

      face->charmaps[n] = NULL;
    }

    FT_TS_FREE( face->charmaps );
    face->num_charmaps = 0;
  }


  /* destructor for faces list */
  static void
  destroy_face( FT_TS_Memory  memory,
                FT_TS_Face    face,
                FT_TS_Driver  driver )
  {
    FT_TS_Driver_Class  clazz = driver->clazz;


    /* discard auto-hinting data */
    if ( face->autohint.finalizer )
      face->autohint.finalizer( face->autohint.data );

    /* Discard glyph slots for this face.                           */
    /* Beware!  FT_TS_Done_GlyphSlot() changes the field `face->glyph' */
    while ( face->glyph )
      FT_TS_Done_GlyphSlot( face->glyph );

    /* discard all sizes for this face */
    FT_TS_List_Finalize( &face->sizes_list,
                      (FT_TS_List_Destructor)destroy_size,
                      memory,
                      driver );
    face->size = NULL;

    /* now discard client data */
    if ( face->generic.finalizer )
      face->generic.finalizer( face );

    /* discard charmaps */
    destroy_charmaps( face, memory );

    /* finalize format-specific stuff */
    if ( clazz->done_face )
      clazz->done_face( face );

    /* close the stream for this face if needed */
    FT_TS_Stream_Free(
      face->stream,
      ( face->face_flags & FT_TS_FACE_FLAG_EXTERNAL_STREAM ) != 0 );

    face->stream = NULL;

    /* get rid of it */
    if ( face->internal )
    {
      FT_TS_FREE( face->internal );
    }
    FT_TS_FREE( face );
  }


  static void
  Destroy_Driver( FT_TS_Driver  driver )
  {
    FT_TS_List_Finalize( &driver->faces_list,
                      (FT_TS_List_Destructor)destroy_face,
                      driver->root.memory,
                      driver );
  }


  /**************************************************************************
   *
   * @Function:
   *   find_unicode_charmap
   *
   * @Description:
   *   This function finds a Unicode charmap, if there is one.
   *   And if there is more than one, it tries to favour the more
   *   extensive one, i.e., one that supports UCS-4 against those which
   *   are limited to the BMP (said UCS-2 encoding.)
   *
   *   This function is called from open_face() (just below), and also
   *   from FT_TS_Select_Charmap( ..., FT_TS_ENCODING_UNICODE ).
   */
  static FT_TS_Error
  find_unicode_charmap( FT_TS_Face  face )
  {
    FT_TS_CharMap*  first;
    FT_TS_CharMap*  cur;


    /* caller should have already checked that `face' is valid */
    FT_TS_ASSERT( face );

    first = face->charmaps;

    if ( !first )
      return FT_TS_THROW( Invalid_CharMap_Handle );

    /*
     * The original TrueType specification(s) only specified charmap
     * formats that are capable of mapping 8 or 16 bit character codes to
     * glyph indices.
     *
     * However, recent updates to the Apple and OpenType specifications
     * introduced new formats that are capable of mapping 32-bit character
     * codes as well.  And these are already used on some fonts, mainly to
     * map non-BMP Asian ideographs as defined in Unicode.
     *
     * For compatibility purposes, these fonts generally come with
     * *several* Unicode charmaps:
     *
     * - One of them in the "old" 16-bit format, that cannot access
     *   all glyphs in the font.
     *
     * - Another one in the "new" 32-bit format, that can access all
     *   the glyphs.
     *
     * This function has been written to always favor a 32-bit charmap
     * when found.  Otherwise, a 16-bit one is returned when found.
     */

    /* Since the `interesting' table, with IDs (3,10), is normally the */
    /* last one, we loop backwards.  This loses with type1 fonts with  */
    /* non-BMP characters (<.0001%), this wins with .ttf with non-BMP  */
    /* chars (.01% ?), and this is the same about 99.99% of the time!  */

    cur = first + face->num_charmaps;  /* points after the last one */

    for ( ; --cur >= first; )
    {
      if ( cur[0]->encoding == FT_TS_ENCODING_UNICODE )
      {
        /* XXX If some new encodings to represent UCS-4 are added, */
        /*     they should be added here.                          */
        if ( ( cur[0]->platform_id == TT_PLATFORM_MICROSOFT &&
               cur[0]->encoding_id == TT_MS_ID_UCS_4        )     ||
             ( cur[0]->platform_id == TT_PLATFORM_APPLE_UNICODE &&
               cur[0]->encoding_id == TT_APPLE_ID_UNICODE_32    ) )
        {
          face->charmap = cur[0];
          return FT_TS_Err_Ok;
        }
      }
    }

    /* We do not have any UCS-4 charmap.                */
    /* Do the loop again and search for UCS-2 charmaps. */
    cur = first + face->num_charmaps;

    for ( ; --cur >= first; )
    {
      if ( cur[0]->encoding == FT_TS_ENCODING_UNICODE )
      {
        face->charmap = cur[0];
        return FT_TS_Err_Ok;
      }
    }

    return FT_TS_THROW( Invalid_CharMap_Handle );
  }


  /**************************************************************************
   *
   * @Function:
   *   find_variant_selector_charmap
   *
   * @Description:
   *   This function finds the variant selector charmap, if there is one.
   *   There can only be one (platform=0, specific=5, format=14).
   */
  static FT_TS_CharMap
  find_variant_selector_charmap( FT_TS_Face  face )
  {
    FT_TS_CharMap*  first;
    FT_TS_CharMap*  end;
    FT_TS_CharMap*  cur;


    /* caller should have already checked that `face' is valid */
    FT_TS_ASSERT( face );

    first = face->charmaps;

    if ( !first )
      return NULL;

    end = first + face->num_charmaps;  /* points after the last one */

    for ( cur = first; cur < end; cur++ )
    {
      if ( cur[0]->platform_id == TT_PLATFORM_APPLE_UNICODE    &&
           cur[0]->encoding_id == TT_APPLE_ID_VARIANT_SELECTOR &&
           FT_TS_Get_CMap_Format( cur[0] ) == 14                  )
        return cur[0];
    }

    return NULL;
  }


  /**************************************************************************
   *
   * @Function:
   *   open_face
   *
   * @Description:
   *   This function does some work for FT_TS_Open_Face().
   */
  static FT_TS_Error
  open_face( FT_TS_Driver      driver,
             FT_TS_Stream      *astream,
             FT_TS_Bool        external_stream,
             FT_TS_Long        face_index,
             FT_TS_Int         num_params,
             FT_TS_Parameter*  params,
             FT_TS_Face       *aface )
  {
    FT_TS_Memory         memory;
    FT_TS_Driver_Class   clazz;
    FT_TS_Face           face     = NULL;
    FT_TS_Face_Internal  internal = NULL;

    FT_TS_Error          error, error2;


    clazz  = driver->clazz;
    memory = driver->root.memory;

    /* allocate the face object and perform basic initialization */
    if ( FT_TS_ALLOC( face, clazz->face_object_size ) )
      goto Fail;

    face->driver = driver;
    face->memory = memory;
    face->stream = *astream;

    /* set the FT_TS_FACE_FLAG_EXTERNAL_STREAM bit for FT_TS_Done_Face */
    if ( external_stream )
      face->face_flags |= FT_TS_FACE_FLAG_EXTERNAL_STREAM;

    if ( FT_TS_NEW( internal ) )
      goto Fail;

    face->internal = internal;

#ifdef FT_TS_CONFIG_OPTION_INCREMENTAL
    {
      int  i;


      face->internal->incremental_interface = NULL;
      for ( i = 0; i < num_params && !face->internal->incremental_interface;
            i++ )
        if ( params[i].tag == FT_TS_PARAM_TAG_INCREMENTAL )
          face->internal->incremental_interface =
            (FT_TS_Incremental_Interface)params[i].data;
    }
#endif

    face->internal->random_seed = -1;

    if ( clazz->init_face )
      error = clazz->init_face( *astream,
                                face,
                                (FT_TS_Int)face_index,
                                num_params,
                                params );
    *astream = face->stream; /* Stream may have been changed. */
    if ( error )
      goto Fail;

    /* select Unicode charmap by default */
    error2 = find_unicode_charmap( face );

    /* if no Unicode charmap can be found, FT_TS_Err_Invalid_CharMap_Handle */
    /* is returned.                                                      */

    /* no error should happen, but we want to play safe */
    if ( error2 && FT_TS_ERR_NEQ( error2, Invalid_CharMap_Handle ) )
    {
      error = error2;
      goto Fail;
    }

    *aface = face;

  Fail:
    if ( error )
    {
      destroy_charmaps( face, memory );
      if ( clazz->done_face )
        clazz->done_face( face );
      FT_TS_FREE( internal );
      FT_TS_FREE( face );
      *aface = NULL;
    }

    return error;
  }


  /* there's a Mac-specific extended implementation of FT_TS_New_Face() */
  /* in src/base/ftmac.c                                             */

#ifndef FT_TS_MACINTOSH

  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_New_Face( FT_TS_Library   library,
               const char*  pathname,
               FT_TS_Long      face_index,
               FT_TS_Face     *aface )
  {
    FT_TS_Open_Args  args;


    /* test for valid `library' and `aface' delayed to `FT_TS_Open_Face' */
    if ( !pathname )
      return FT_TS_THROW( Invalid_Argument );

    args.flags    = FT_TS_OPEN_PATHNAME;
    args.pathname = (char*)pathname;
    args.stream   = NULL;

    return ft_open_face_internal( library, &args, face_index, aface, 1 );
  }

#endif


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_New_Memory_Face( FT_TS_Library      library,
                      const FT_TS_Byte*  file_base,
                      FT_TS_Long         file_size,
                      FT_TS_Long         face_index,
                      FT_TS_Face        *aface )
  {
    FT_TS_Open_Args  args;


    /* test for valid `library' and `face' delayed to `FT_TS_Open_Face' */
    if ( !file_base )
      return FT_TS_THROW( Invalid_Argument );

    args.flags       = FT_TS_OPEN_MEMORY;
    args.memory_base = file_base;
    args.memory_size = file_size;
    args.stream      = NULL;

    return ft_open_face_internal( library, &args, face_index, aface, 1 );
  }


#ifdef FT_TS_CONFIG_OPTION_MAC_FONTS

  /* The behavior here is very similar to that in base/ftmac.c, but it     */
  /* is designed to work on non-mac systems, so no mac specific calls.     */
  /*                                                                       */
  /* We look at the file and determine if it is a mac dfont file or a mac  */
  /* resource file, or a macbinary file containing a mac resource file.    */
  /*                                                                       */
  /* Unlike ftmac I'm not going to look at a `FOND'.  I don't really see   */
  /* the point, especially since there may be multiple `FOND' resources.   */
  /* Instead I'll just look for `sfnt' and `POST' resources, ordered as    */
  /* they occur in the file.                                               */
  /*                                                                       */
  /* Note that multiple `POST' resources do not mean multiple postscript   */
  /* fonts; they all get jammed together to make what is essentially a     */
  /* pfb file.                                                             */
  /*                                                                       */
  /* We aren't interested in `NFNT' or `FONT' bitmap resources.            */
  /*                                                                       */
  /* As soon as we get an `sfnt' load it into memory and pass it off to    */
  /* FT_TS_Open_Face.                                                         */
  /*                                                                       */
  /* If we have a (set of) `POST' resources, massage them into a (memory)  */
  /* pfb file and pass that to FT_TS_Open_Face.  (As with ftmac.c I'm not     */
  /* going to try to save the kerning info.  After all that lives in the   */
  /* `FOND' which isn't in the file containing the `POST' resources so     */
  /* we don't really have access to it.                                    */


  /* Finalizer for a memory stream; gets called by FT_TS_Done_Face(). */
  /* It frees the memory it uses.                                  */
  /* From `ftmac.c'.                                               */
  static void
  memory_stream_close( FT_TS_Stream  stream )
  {
    FT_TS_Memory  memory = stream->memory;


    FT_TS_FREE( stream->base );

    stream->size  = 0;
    stream->close = NULL;
  }


  /* Create a new memory stream from a buffer and a size. */
  /* From `ftmac.c'.                                      */
  static FT_TS_Error
  new_memory_stream( FT_TS_Library           library,
                     FT_TS_Byte*             base,
                     FT_TS_ULong             size,
                     FT_TS_Stream_CloseFunc  close,
                     FT_TS_Stream           *astream )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory;
    FT_TS_Stream  stream = NULL;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !base )
      return FT_TS_THROW( Invalid_Argument );

    *astream = NULL;
    memory   = library->memory;
    if ( FT_TS_NEW( stream ) )
      goto Exit;

    FT_TS_Stream_OpenMemory( stream, base, size );

    stream->close = close;

    *astream = stream;

  Exit:
    return error;
  }


  /* Create a new FT_TS_Face given a buffer and a driver name. */
  /* From `ftmac.c'.                                        */
  FT_TS_LOCAL_DEF( FT_TS_Error )
  open_face_from_buffer( FT_TS_Library   library,
                         FT_TS_Byte*     base,
                         FT_TS_ULong     size,
                         FT_TS_Long      face_index,
                         const char*  driver_name,
                         FT_TS_Face     *aface )
  {
    FT_TS_Open_Args  args;
    FT_TS_Error      error;
    FT_TS_Stream     stream = NULL;
    FT_TS_Memory     memory = library->memory;


    error = new_memory_stream( library,
                               base,
                               size,
                               memory_stream_close,
                               &stream );
    if ( error )
    {
      FT_TS_FREE( base );
      return error;
    }

    args.flags  = FT_TS_OPEN_STREAM;
    args.stream = stream;
    if ( driver_name )
    {
      args.flags  = args.flags | FT_TS_OPEN_DRIVER;
      args.driver = FT_TS_Get_Module( library, driver_name );
    }

#ifdef FT_TS_MACINTOSH
    /* At this point, the face index has served its purpose;  */
    /* whoever calls this function has already used it to     */
    /* locate the correct font data.  We should not propagate */
    /* this index to FT_TS_Open_Face() (unless it is negative).  */

    if ( face_index > 0 )
      face_index &= 0x7FFF0000L; /* retain GX data */
#endif

    error = ft_open_face_internal( library, &args, face_index, aface, 0 );

    if ( !error )
      (*aface)->face_flags &= ~FT_TS_FACE_FLAG_EXTERNAL_STREAM;
    else
#ifdef FT_TS_MACINTOSH
      FT_TS_Stream_Free( stream, 0 );
#else
    {
      FT_TS_Stream_Close( stream );
      FT_TS_FREE( stream );
    }
#endif

    return error;
  }


  /* Look up `TYP1' or `CID ' table from sfnt table directory.       */
  /* `offset' and `length' must exclude the binary header in tables. */

  /* Type 1 and CID-keyed font drivers should recognize sfnt-wrapped */
  /* format too.  Here, since we can't expect that the TrueType font */
  /* driver is loaded unconditionally, we must parse the font by     */
  /* ourselves.  We are only interested in the name of the table and */
  /* the offset.                                                     */

  static FT_TS_Error
  ft_lookup_PS_in_sfnt_stream( FT_TS_Stream  stream,
                               FT_TS_Long    face_index,
                               FT_TS_ULong*  offset,
                               FT_TS_ULong*  length,
                               FT_TS_Bool*   is_sfnt_cid )
  {
    FT_TS_Error   error;
    FT_TS_UShort  numTables;
    FT_TS_Long    pstable_index;
    FT_TS_ULong   tag;
    int        i;


    *offset = 0;
    *length = 0;
    *is_sfnt_cid = FALSE;

    /* TODO: support for sfnt-wrapped PS/CID in TTC format */

    /* version check for 'typ1' (should be ignored?) */
    if ( FT_TS_READ_ULONG( tag ) )
      return error;
    if ( tag != TTAG_typ1 )
      return FT_TS_THROW( Unknown_File_Format );

    if ( FT_TS_READ_USHORT( numTables ) )
      return error;
    if ( FT_TS_STREAM_SKIP( 2 * 3 ) ) /* skip binary search header */
      return error;

    pstable_index = -1;
    *is_sfnt_cid  = FALSE;

    for ( i = 0; i < numTables; i++ )
    {
      if ( FT_TS_READ_ULONG( tag )     || FT_TS_STREAM_SKIP( 4 )      ||
           FT_TS_READ_ULONG( *offset ) || FT_TS_READ_ULONG( *length ) )
        return error;

      if ( tag == TTAG_CID )
      {
        pstable_index++;
        *offset += 22;
        *length -= 22;
        *is_sfnt_cid = TRUE;
        if ( face_index < 0 )
          return FT_TS_Err_Ok;
      }
      else if ( tag == TTAG_TYP1 )
      {
        pstable_index++;
        *offset += 24;
        *length -= 24;
        *is_sfnt_cid = FALSE;
        if ( face_index < 0 )
          return FT_TS_Err_Ok;
      }
      if ( face_index >= 0 && pstable_index == face_index )
        return FT_TS_Err_Ok;
    }

    return FT_TS_THROW( Table_Missing );
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  open_face_PS_from_sfnt_stream( FT_TS_Library     library,
                                 FT_TS_Stream      stream,
                                 FT_TS_Long        face_index,
                                 FT_TS_Int         num_params,
                                 FT_TS_Parameter  *params,
                                 FT_TS_Face       *aface )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory = library->memory;
    FT_TS_ULong   offset, length;
    FT_TS_ULong   pos;
    FT_TS_Bool    is_sfnt_cid;
    FT_TS_Byte*   sfnt_ps = NULL;

    FT_TS_UNUSED( num_params );
    FT_TS_UNUSED( params );


    /* ignore GX stuff */
    if ( face_index > 0 )
      face_index &= 0xFFFFL;

    pos = FT_TS_STREAM_POS();

    error = ft_lookup_PS_in_sfnt_stream( stream,
                                         face_index,
                                         &offset,
                                         &length,
                                         &is_sfnt_cid );
    if ( error )
      goto Exit;

    if ( offset > stream->size )
    {
      FT_TS_TRACE2(( "open_face_PS_from_sfnt_stream: invalid table offset\n" ));
      error = FT_TS_THROW( Invalid_Table );
      goto Exit;
    }
    else if ( length > stream->size - offset )
    {
      FT_TS_TRACE2(( "open_face_PS_from_sfnt_stream: invalid table length\n" ));
      error = FT_TS_THROW( Invalid_Table );
      goto Exit;
    }

    error = FT_TS_Stream_Seek( stream, pos + offset );
    if ( error )
      goto Exit;

    if ( FT_TS_QALLOC( sfnt_ps, (FT_TS_Long)length ) )
      goto Exit;

    error = FT_TS_Stream_Read( stream, (FT_TS_Byte *)sfnt_ps, length );
    if ( error )
    {
      FT_TS_FREE( sfnt_ps );
      goto Exit;
    }

    error = open_face_from_buffer( library,
                                   sfnt_ps,
                                   length,
                                   FT_TS_MIN( face_index, 0 ),
                                   is_sfnt_cid ? "cid" : "type1",
                                   aface );
  Exit:
    {
      FT_TS_Error  error1;


      if ( FT_TS_ERR_EQ( error, Unknown_File_Format ) )
      {
        error1 = FT_TS_Stream_Seek( stream, pos );
        if ( error1 )
          return error1;
      }

      return error;
    }
  }


#ifndef FT_TS_MACINTOSH

  /* The resource header says we've got resource_cnt `POST' (type1) */
  /* resources in this file.  They all need to be coalesced into    */
  /* one lump which gets passed on to the type1 driver.             */
  /* Here can be only one PostScript font in a file so face_index   */
  /* must be 0 (or -1).                                             */
  /*                                                                */
  static FT_TS_Error
  Mac_Read_POST_Resource( FT_TS_Library  library,
                          FT_TS_Stream   stream,
                          FT_TS_Long    *offsets,
                          FT_TS_Long     resource_cnt,
                          FT_TS_Long     face_index,
                          FT_TS_Face    *aface )
  {
    FT_TS_Error   error  = FT_TS_ERR( Cannot_Open_Resource );
    FT_TS_Memory  memory = library->memory;

    FT_TS_Byte*   pfb_data = NULL;
    int        i, type, flags;
    FT_TS_ULong   len;
    FT_TS_ULong   pfb_len, pfb_pos, pfb_lenpos;
    FT_TS_ULong   rlen, temp;


    if ( face_index == -1 )
      face_index = 0;
    if ( face_index != 0 )
      return error;

    /* Find the length of all the POST resources, concatenated.  Assume */
    /* worst case (each resource in its own section).                   */
    pfb_len = 0;
    for ( i = 0; i < resource_cnt; i++ )
    {
      error = FT_TS_Stream_Seek( stream, (FT_TS_ULong)offsets[i] );
      if ( error )
        goto Exit;
      if ( FT_TS_READ_ULONG( temp ) )  /* actually LONG */
        goto Exit;

      /* FT2 allocator takes signed long buffer length,
       * too large value causing overflow should be checked
       */
      FT_TS_TRACE4(( "                 POST fragment #%d: length=0x%08lx"
                  " total pfb_len=0x%08lx\n",
                  i, temp, pfb_len + temp + 6 ));

      if ( FT_TS_MAC_RFORK_MAX_LEN < temp               ||
           FT_TS_MAC_RFORK_MAX_LEN - temp < pfb_len + 6 )
      {
        FT_TS_TRACE2(( "             MacOS resource length cannot exceed"
                    " 0x%08lx\n",
                    FT_TS_MAC_RFORK_MAX_LEN ));

        error = FT_TS_THROW( Invalid_Offset );
        goto Exit;
      }

      pfb_len += temp + 6;
    }

    FT_TS_TRACE2(( "             total buffer size to concatenate"
                " %ld POST fragments: 0x%08lx\n",
                 resource_cnt, pfb_len + 2 ));

    if ( pfb_len + 2 < 6 )
    {
      FT_TS_TRACE2(( "             too long fragment length makes"
                  " pfb_len confused: pfb_len=0x%08lx\n",
                  pfb_len ));

      error = FT_TS_THROW( Array_Too_Large );
      goto Exit;
    }

    if ( FT_TS_QALLOC( pfb_data, (FT_TS_Long)pfb_len + 2 ) )
      goto Exit;

    pfb_data[0] = 0x80;
    pfb_data[1] = 1;            /* Ascii section */
    pfb_data[2] = 0;            /* 4-byte length, fill in later */
    pfb_data[3] = 0;
    pfb_data[4] = 0;
    pfb_data[5] = 0;
    pfb_pos     = 6;
    pfb_lenpos  = 2;

    len  = 0;
    type = 1;

    for ( i = 0; i < resource_cnt; i++ )
    {
      error = FT_TS_Stream_Seek( stream, (FT_TS_ULong)offsets[i] );
      if ( error )
        goto Exit2;
      if ( FT_TS_READ_ULONG( rlen ) )
        goto Exit2;

      /* FT2 allocator takes signed long buffer length,
       * too large fragment length causing overflow should be checked
       */
      if ( 0x7FFFFFFFUL < rlen )
      {
        error = FT_TS_THROW( Invalid_Offset );
        goto Exit2;
      }

      if ( FT_TS_READ_USHORT( flags ) )
        goto Exit2;

      FT_TS_TRACE3(( "POST fragment[%d]:"
                  " offsets=0x%08lx, rlen=0x%08lx, flags=0x%04x\n",
                  i, offsets[i], rlen, flags ));

      error = FT_TS_ERR( Array_Too_Large );

      /* postpone the check of `rlen longer than buffer' */
      /* until `FT_TS_Stream_Read'                          */

      if ( ( flags >> 8 ) == 0 )        /* Comment, should not be loaded */
      {
        FT_TS_TRACE3(( "    Skip POST fragment #%d because it is a comment\n",
                    i ));
        continue;
      }

      /* the flags are part of the resource, so rlen >= 2,  */
      /* but some fonts declare rlen = 0 for empty fragment */
      if ( rlen > 2 )
        rlen -= 2;
      else
        rlen = 0;

      if ( ( flags >> 8 ) == type )
        len += rlen;
      else
      {
        FT_TS_TRACE3(( "    Write POST fragment #%d header (4-byte) to buffer"
                    " %p + 0x%08lx\n",
                    i, (void*)pfb_data, pfb_lenpos ));

        if ( pfb_lenpos + 3 > pfb_len + 2 )
          goto Exit2;

        pfb_data[pfb_lenpos    ] = (FT_TS_Byte)( len );
        pfb_data[pfb_lenpos + 1] = (FT_TS_Byte)( len >> 8 );
        pfb_data[pfb_lenpos + 2] = (FT_TS_Byte)( len >> 16 );
        pfb_data[pfb_lenpos + 3] = (FT_TS_Byte)( len >> 24 );

        if ( ( flags >> 8 ) == 5 )      /* End of font mark */
          break;

        FT_TS_TRACE3(( "    Write POST fragment #%d header (6-byte) to buffer"
                    " %p + 0x%08lx\n",
                    i, (void*)pfb_data, pfb_pos ));

        if ( pfb_pos + 6 > pfb_len + 2 )
          goto Exit2;

        pfb_data[pfb_pos++] = 0x80;

        type = flags >> 8;
        len  = rlen;

        pfb_data[pfb_pos++] = (FT_TS_Byte)type;
        pfb_lenpos          = pfb_pos;
        pfb_data[pfb_pos++] = 0;        /* 4-byte length, fill in later */
        pfb_data[pfb_pos++] = 0;
        pfb_data[pfb_pos++] = 0;
        pfb_data[pfb_pos++] = 0;
      }

      if ( pfb_pos > pfb_len || pfb_pos + rlen > pfb_len )
        goto Exit2;

      FT_TS_TRACE3(( "    Load POST fragment #%d (%ld byte) to buffer"
                  " %p + 0x%08lx\n",
                  i, rlen, (void*)pfb_data, pfb_pos ));

      error = FT_TS_Stream_Read( stream, (FT_TS_Byte *)pfb_data + pfb_pos, rlen );
      if ( error )
        goto Exit2;

      pfb_pos += rlen;
    }

    error = FT_TS_ERR( Array_Too_Large );

    if ( pfb_pos + 2 > pfb_len + 2 )
      goto Exit2;
    pfb_data[pfb_pos++] = 0x80;
    pfb_data[pfb_pos++] = 3;

    if ( pfb_lenpos + 3 > pfb_len + 2 )
      goto Exit2;
    pfb_data[pfb_lenpos    ] = (FT_TS_Byte)( len );
    pfb_data[pfb_lenpos + 1] = (FT_TS_Byte)( len >> 8 );
    pfb_data[pfb_lenpos + 2] = (FT_TS_Byte)( len >> 16 );
    pfb_data[pfb_lenpos + 3] = (FT_TS_Byte)( len >> 24 );

    return open_face_from_buffer( library,
                                  pfb_data,
                                  pfb_pos,
                                  face_index,
                                  "type1",
                                  aface );

  Exit2:
    if ( FT_TS_ERR_EQ( error, Array_Too_Large ) )
      FT_TS_TRACE2(( "  Abort due to too-short buffer to store"
                  " all POST fragments\n" ));
    else if ( FT_TS_ERR_EQ( error, Invalid_Offset ) )
      FT_TS_TRACE2(( "  Abort due to invalid offset in a POST fragment\n" ));

    if ( error )
      error = FT_TS_ERR( Cannot_Open_Resource );
    FT_TS_FREE( pfb_data );

  Exit:
    return error;
  }


  /* The resource header says we've got resource_cnt `sfnt'      */
  /* (TrueType/OpenType) resources in this file.  Look through   */
  /* them for the one indicated by face_index, load it into mem, */
  /* pass it on to the truetype driver, and return it.           */
  /*                                                             */
  static FT_TS_Error
  Mac_Read_sfnt_Resource( FT_TS_Library  library,
                          FT_TS_Stream   stream,
                          FT_TS_Long    *offsets,
                          FT_TS_Long     resource_cnt,
                          FT_TS_Long     face_index,
                          FT_TS_Face    *aface )
  {
    FT_TS_Memory  memory = library->memory;
    FT_TS_Byte*   sfnt_data = NULL;
    FT_TS_Error   error;
    FT_TS_ULong   flag_offset;
    FT_TS_Long    rlen;
    int        is_cff;
    FT_TS_Long    face_index_in_resource = 0;


    if ( face_index < 0 )
      face_index = -face_index - 1;
    if ( face_index >= resource_cnt )
      return FT_TS_THROW( Cannot_Open_Resource );

    flag_offset = (FT_TS_ULong)offsets[face_index];
    error = FT_TS_Stream_Seek( stream, flag_offset );
    if ( error )
      goto Exit;

    if ( FT_TS_READ_LONG( rlen ) )
      goto Exit;
    if ( rlen < 1 )
      return FT_TS_THROW( Cannot_Open_Resource );
    if ( (FT_TS_ULong)rlen > FT_TS_MAC_RFORK_MAX_LEN )
      return FT_TS_THROW( Invalid_Offset );

    error = open_face_PS_from_sfnt_stream( library,
                                           stream,
                                           face_index,
                                           0, NULL,
                                           aface );
    if ( !error )
      goto Exit;

    /* rewind sfnt stream before open_face_PS_from_sfnt_stream() */
    error = FT_TS_Stream_Seek( stream, flag_offset + 4 );
    if ( error )
      goto Exit;

    if ( FT_TS_QALLOC( sfnt_data, rlen ) )
      return error;
    error = FT_TS_Stream_Read( stream, (FT_TS_Byte *)sfnt_data, (FT_TS_ULong)rlen );
    if ( error ) {
      FT_TS_FREE( sfnt_data );
      goto Exit;
    }

    is_cff = rlen > 4 && !ft_memcmp( sfnt_data, "OTTO", 4 );
    error = open_face_from_buffer( library,
                                   sfnt_data,
                                   (FT_TS_ULong)rlen,
                                   face_index_in_resource,
                                   is_cff ? "cff" : "truetype",
                                   aface );

  Exit:
    return error;
  }


  /* Check for a valid resource fork header, or a valid dfont    */
  /* header.  In a resource fork the first 16 bytes are repeated */
  /* at the location specified by bytes 4-7.  In a dfont bytes   */
  /* 4-7 point to 16 bytes of zeroes instead.                    */
  /*                                                             */
  static FT_TS_Error
  IsMacResource( FT_TS_Library  library,
                 FT_TS_Stream   stream,
                 FT_TS_Long     resource_offset,
                 FT_TS_Long     face_index,
                 FT_TS_Face    *aface )
  {
    FT_TS_Memory  memory = library->memory;
    FT_TS_Error   error;
    FT_TS_Long    map_offset, rdata_pos;
    FT_TS_Long    *data_offsets;
    FT_TS_Long    count;


    error = FT_TS_Raccess_Get_HeaderInfo( library, stream, resource_offset,
                                       &map_offset, &rdata_pos );
    if ( error )
      return error;

    /* POST resources must be sorted to concatenate properly */
    error = FT_TS_Raccess_Get_DataOffsets( library, stream,
                                        map_offset, rdata_pos,
                                        TTAG_POST, TRUE,
                                        &data_offsets, &count );
    if ( !error )
    {
      error = Mac_Read_POST_Resource( library, stream, data_offsets, count,
                                      face_index, aface );
      FT_TS_FREE( data_offsets );
      /* POST exists in an LWFN providing a single face */
      if ( !error )
        (*aface)->num_faces = 1;
      return error;
    }

    /* sfnt resources should not be sorted to preserve the face order by
       QuickDraw API */
    error = FT_TS_Raccess_Get_DataOffsets( library, stream,
                                        map_offset, rdata_pos,
                                        TTAG_sfnt, FALSE,
                                        &data_offsets, &count );
    if ( !error )
    {
      FT_TS_Long  face_index_internal = face_index % count;


      error = Mac_Read_sfnt_Resource( library, stream, data_offsets, count,
                                      face_index_internal, aface );
      FT_TS_FREE( data_offsets );
      if ( !error )
        (*aface)->num_faces = count;
    }

    return error;
  }


  /* Check for a valid macbinary header, and if we find one   */
  /* check that the (flattened) resource fork in it is valid. */
  /*                                                          */
  static FT_TS_Error
  IsMacBinary( FT_TS_Library  library,
               FT_TS_Stream   stream,
               FT_TS_Long     face_index,
               FT_TS_Face    *aface )
  {
    unsigned char  header[128];
    FT_TS_Error       error;
    FT_TS_Long        dlen, offset;


    if ( !stream )
      return FT_TS_THROW( Invalid_Stream_Operation );

    error = FT_TS_Stream_Seek( stream, 0 );
    if ( error )
      goto Exit;

    error = FT_TS_Stream_Read( stream, (FT_TS_Byte*)header, 128 );
    if ( error )
      goto Exit;

    if (            header[ 0] !=   0 ||
                    header[74] !=   0 ||
                    header[82] !=   0 ||
                    header[ 1] ==   0 ||
                    header[ 1] >   33 ||
                    header[63] !=   0 ||
         header[2 + header[1]] !=   0 ||
                  header[0x53] > 0x7F )
      return FT_TS_THROW( Unknown_File_Format );

    dlen = ( header[0x53] << 24 ) |
           ( header[0x54] << 16 ) |
           ( header[0x55] <<  8 ) |
             header[0x56];
#if 0
    rlen = ( header[0x57] << 24 ) |
           ( header[0x58] << 16 ) |
           ( header[0x59] <<  8 ) |
             header[0x5A];
#endif /* 0 */
    offset = 128 + ( ( dlen + 127 ) & ~127 );

    return IsMacResource( library, stream, offset, face_index, aface );

  Exit:
    return error;
  }


  static FT_TS_Error
  load_face_in_embedded_rfork( FT_TS_Library           library,
                               FT_TS_Stream            stream,
                               FT_TS_Long              face_index,
                               FT_TS_Face             *aface,
                               const FT_TS_Open_Args  *args )
  {

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  raccess

    FT_TS_Memory  memory = library->memory;
    FT_TS_Error   error  = FT_TS_ERR( Unknown_File_Format );
    FT_TS_UInt    i;

    char*      file_names[FT_TS_RACCESS_N_RULES];
    FT_TS_Long    offsets[FT_TS_RACCESS_N_RULES];
    FT_TS_Error   errors[FT_TS_RACCESS_N_RULES];
    FT_TS_Bool    is_darwin_vfs, vfs_rfork_has_no_font = FALSE; /* not tested */

    FT_TS_Open_Args  args2;
    FT_TS_Stream     stream2 = NULL;


    FT_TS_Raccess_Guess( library, stream,
                      args->pathname, file_names, offsets, errors );

    for ( i = 0; i < FT_TS_RACCESS_N_RULES; i++ )
    {
      is_darwin_vfs = ft_raccess_rule_by_darwin_vfs( library, i );
      if ( is_darwin_vfs && vfs_rfork_has_no_font )
      {
        FT_TS_TRACE3(( "Skip rule %d: darwin vfs resource fork"
                    " is already checked and"
                    " no font is found\n",
                    i ));
        continue;
      }

      if ( errors[i] )
      {
        FT_TS_TRACE3(( "Error 0x%x has occurred in rule %d\n",
                    errors[i], i ));
        continue;
      }

      args2.flags    = FT_TS_OPEN_PATHNAME;
      args2.pathname = file_names[i] ? file_names[i] : args->pathname;

      FT_TS_TRACE3(( "Try rule %d: %s (offset=%ld) ...",
                  i, args2.pathname, offsets[i] ));

      error = FT_TS_Stream_New( library, &args2, &stream2 );
      if ( is_darwin_vfs && FT_TS_ERR_EQ( error, Cannot_Open_Stream ) )
        vfs_rfork_has_no_font = TRUE;

      if ( error )
      {
        FT_TS_TRACE3(( "failed\n" ));
        continue;
      }

      error = IsMacResource( library, stream2, offsets[i],
                             face_index, aface );
      FT_TS_Stream_Free( stream2, 0 );

      FT_TS_TRACE3(( "%s\n", error ? "failed": "successful" ));

      if ( !error )
          break;
      else if ( is_darwin_vfs )
          vfs_rfork_has_no_font = TRUE;
    }

    for (i = 0; i < FT_TS_RACCESS_N_RULES; i++)
    {
      if ( file_names[i] )
        FT_TS_FREE( file_names[i] );
    }

    /* Caller (load_mac_face) requires FT_TS_Err_Unknown_File_Format. */
    if ( error )
      error = FT_TS_ERR( Unknown_File_Format );

    return error;

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  objs

  }


  /* Check for some macintosh formats without Carbon framework.    */
  /* Is this a macbinary file?  If so look at the resource fork.   */
  /* Is this a mac dfont file?                                     */
  /* Is this an old style resource fork? (in data)                 */
  /* Else call load_face_in_embedded_rfork to try extra rules      */
  /* (defined in `ftrfork.c').                                     */
  /*                                                               */
  static FT_TS_Error
  load_mac_face( FT_TS_Library           library,
                 FT_TS_Stream            stream,
                 FT_TS_Long              face_index,
                 FT_TS_Face             *aface,
                 const FT_TS_Open_Args  *args )
  {
    FT_TS_Error error;
    FT_TS_UNUSED( args );


    error = IsMacBinary( library, stream, face_index, aface );
    if ( FT_TS_ERR_EQ( error, Unknown_File_Format ) )
    {

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  raccess

#ifdef FT_TS_DEBUG_LEVEL_TRACE
      FT_TS_TRACE3(( "Try as dfont: " ));
      if ( !( args->flags & FT_TS_OPEN_MEMORY ) )
        FT_TS_TRACE3(( "%s ...", args->pathname ));
#endif

      error = IsMacResource( library, stream, 0, face_index, aface );

      FT_TS_TRACE3(( "%s\n", error ? "failed" : "successful" ));

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  objs

    }

    if ( ( FT_TS_ERR_EQ( error, Unknown_File_Format )      ||
           FT_TS_ERR_EQ( error, Invalid_Stream_Operation ) ) &&
         ( args->flags & FT_TS_OPEN_PATHNAME )               )
      error = load_face_in_embedded_rfork( library, stream,
                                           face_index, aface, args );
    return error;
  }
#endif

#endif  /* !FT_TS_MACINTOSH && FT_TS_CONFIG_OPTION_MAC_FONTS */


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Open_Face( FT_TS_Library           library,
                const FT_TS_Open_Args*  args,
                FT_TS_Long              face_index,
                FT_TS_Face             *aface )
  {
    return ft_open_face_internal( library, args, face_index, aface, 1 );
  }


  static FT_TS_Error
  ft_open_face_internal( FT_TS_Library           library,
                         const FT_TS_Open_Args*  args,
                         FT_TS_Long              face_index,
                         FT_TS_Face             *aface,
                         FT_TS_Bool              test_mac_fonts )
  {
    FT_TS_Error     error;
    FT_TS_Driver    driver = NULL;
    FT_TS_Memory    memory = NULL;
    FT_TS_Stream    stream = NULL;
    FT_TS_Face      face   = NULL;
    FT_TS_ListNode  node   = NULL;
    FT_TS_Bool      external_stream;
    FT_TS_Module*   cur;
    FT_TS_Module*   limit;

#ifndef FT_TS_CONFIG_OPTION_MAC_FONTS
    FT_TS_UNUSED( test_mac_fonts );
#endif


    /* only use lower 31 bits together with sign bit */
    if ( face_index > 0 )
      face_index &= 0x7FFFFFFFL;
    else
    {
      face_index  = -face_index;
      face_index &= 0x7FFFFFFFL;
      face_index  = -face_index;
    }

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    FT_TS_TRACE3(( "FT_TS_Open_Face: " ));
    if ( face_index < 0 )
      FT_TS_TRACE3(( "Requesting number of faces and named instances\n"));
    else
    {
      FT_TS_TRACE3(( "Requesting face %ld", face_index & 0xFFFFL ));
      if ( face_index & 0x7FFF0000L )
        FT_TS_TRACE3(( ", named instance %ld", face_index >> 16 ));
      FT_TS_TRACE3(( "\n" ));
    }
#endif

    /* test for valid `library' delayed to `FT_TS_Stream_New' */

    if ( ( !aface && face_index >= 0 ) || !args )
      return FT_TS_THROW( Invalid_Argument );

    external_stream = FT_TS_BOOL( ( args->flags & FT_TS_OPEN_STREAM ) &&
                               args->stream                     );

    /* create input stream */
    error = FT_TS_Stream_New( library, args, &stream );
    if ( error )
      goto Fail3;

    memory = library->memory;

    /* If the font driver is specified in the `args' structure, use */
    /* it.  Otherwise, we scan the list of registered drivers.      */
    if ( ( args->flags & FT_TS_OPEN_DRIVER ) && args->driver )
    {
      driver = FT_TS_DRIVER( args->driver );

      /* not all modules are drivers, so check... */
      if ( FT_TS_MODULE_IS_DRIVER( driver ) )
      {
        FT_TS_Int         num_params = 0;
        FT_TS_Parameter*  params     = NULL;


        if ( args->flags & FT_TS_OPEN_PARAMS )
        {
          num_params = args->num_params;
          params     = args->params;
        }

        error = open_face( driver, &stream, external_stream, face_index,
                           num_params, params, &face );
        if ( !error )
          goto Success;
      }
      else
        error = FT_TS_THROW( Invalid_Handle );

      FT_TS_Stream_Free( stream, external_stream );
      goto Fail;
    }
    else
    {
      error = FT_TS_ERR( Missing_Module );

      /* check each font driver for an appropriate format */
      cur   = library->modules;
      limit = cur + library->num_modules;

      for ( ; cur < limit; cur++ )
      {
        /* not all modules are font drivers, so check... */
        if ( FT_TS_MODULE_IS_DRIVER( cur[0] ) )
        {
          FT_TS_Int         num_params = 0;
          FT_TS_Parameter*  params     = NULL;


          driver = FT_TS_DRIVER( cur[0] );

          if ( args->flags & FT_TS_OPEN_PARAMS )
          {
            num_params = args->num_params;
            params     = args->params;
          }

          error = open_face( driver, &stream, external_stream, face_index,
                             num_params, params, &face );
          if ( !error )
            goto Success;

#ifdef FT_TS_CONFIG_OPTION_MAC_FONTS
          if ( test_mac_fonts                                           &&
               ft_strcmp( cur[0]->clazz->module_name, "truetype" ) == 0 &&
               FT_TS_ERR_EQ( error, Table_Missing )                        )
          {
            /* TrueType but essential tables are missing */
            error = FT_TS_Stream_Seek( stream, 0 );
            if ( error )
              break;

            error = open_face_PS_from_sfnt_stream( library,
                                                   stream,
                                                   face_index,
                                                   num_params,
                                                   params,
                                                   aface );
            if ( !error )
            {
              FT_TS_Stream_Free( stream, external_stream );
              return error;
            }
          }
#endif

          if ( FT_TS_ERR_NEQ( error, Unknown_File_Format ) )
            goto Fail3;
        }
      }

    Fail3:
      /* If we are on the mac, and we get an                          */
      /* FT_TS_Err_Invalid_Stream_Operation it may be because we have an */
      /* empty data fork, so we need to check the resource fork.      */
      if ( FT_TS_ERR_NEQ( error, Cannot_Open_Stream )       &&
           FT_TS_ERR_NEQ( error, Unknown_File_Format )      &&
           FT_TS_ERR_NEQ( error, Invalid_Stream_Operation ) )
        goto Fail2;

#if !defined( FT_TS_MACINTOSH ) && defined( FT_TS_CONFIG_OPTION_MAC_FONTS )
      if ( test_mac_fonts )
      {
        error = load_mac_face( library, stream, face_index, aface, args );
        if ( !error )
        {
          /* We don't want to go to Success here.  We've already done   */
          /* that.  On the other hand, if we succeeded we still need to */
          /* close this stream (we opened a different stream which      */
          /* extracted the interesting information out of this stream   */
          /* here.  That stream will still be open and the face will    */
          /* point to it).                                              */
          FT_TS_Stream_Free( stream, external_stream );
          return error;
        }
      }

      if ( FT_TS_ERR_NEQ( error, Unknown_File_Format ) )
        goto Fail2;
#endif  /* !FT_TS_MACINTOSH && FT_TS_CONFIG_OPTION_MAC_FONTS */

      /* no driver is able to handle this format */
      error = FT_TS_THROW( Unknown_File_Format );

  Fail2:
      FT_TS_Stream_Free( stream, external_stream );
      goto Fail;
    }

  Success:
    FT_TS_TRACE4(( "FT_TS_Open_Face: New face object, adding to list\n" ));

    /* add the face object to its driver's list */
    if ( FT_TS_QNEW( node ) )
      goto Fail;

    node->data = face;
    /* don't assume driver is the same as face->driver, so use */
    /* face->driver instead.                                   */
    FT_TS_List_Add( &face->driver->faces_list, node );

    /* now allocate a glyph slot object for the face */
    FT_TS_TRACE4(( "FT_TS_Open_Face: Creating glyph slot\n" ));

    if ( face_index >= 0 )
    {
      error = FT_TS_New_GlyphSlot( face, NULL );
      if ( error )
        goto Fail;

      /* finally, allocate a size object for the face */
      {
        FT_TS_Size  size;


        FT_TS_TRACE4(( "FT_TS_Open_Face: Creating size object\n" ));

        error = FT_TS_New_Size( face, &size );
        if ( error )
          goto Fail;

        face->size = size;
      }
    }

    /* some checks */

    if ( FT_TS_IS_SCALABLE( face ) )
    {
      if ( face->height < 0 )
        face->height = (FT_TS_Short)-face->height;

      if ( !FT_TS_HAS_VERTICAL( face ) )
        face->max_advance_height = (FT_TS_Short)face->height;
    }

    if ( FT_TS_HAS_FIXED_SIZES( face ) )
    {
      FT_TS_Int  i;


      for ( i = 0; i < face->num_fixed_sizes; i++ )
      {
        FT_TS_Bitmap_Size*  bsize = face->available_sizes + i;


        if ( bsize->height < 0 )
          bsize->height = -bsize->height;
        if ( bsize->x_ppem < 0 )
          bsize->x_ppem = -bsize->x_ppem;
        if ( bsize->y_ppem < 0 )
          bsize->y_ppem = -bsize->y_ppem;

        /* check whether negation actually has worked */
        if ( bsize->height < 0 || bsize->x_ppem < 0 || bsize->y_ppem < 0 )
        {
          FT_TS_TRACE0(( "FT_TS_Open_Face:"
                      " Invalid bitmap dimensions for strike %d,"
                      " now disabled\n", i ));
          bsize->width  = 0;
          bsize->height = 0;
          bsize->size   = 0;
          bsize->x_ppem = 0;
          bsize->y_ppem = 0;
        }
      }
    }

    /* initialize internal face data */
    {
      FT_TS_Face_Internal  internal = face->internal;


      internal->transform_matrix.xx = 0x10000L;
      internal->transform_matrix.xy = 0;
      internal->transform_matrix.yx = 0;
      internal->transform_matrix.yy = 0x10000L;

      internal->transform_delta.x = 0;
      internal->transform_delta.y = 0;

      internal->refcount = 1;

      internal->no_stem_darkening = -1;

#ifdef FT_TS_CONFIG_OPTION_SUBPIXEL_RENDERING
      /* Per-face filtering can only be set up by FT_TS_Face_Properties */
      internal->lcd_filter_func = NULL;
#endif
    }

    if ( aface )
      *aface = face;
    else
      FT_TS_Done_Face( face );

    goto Exit;

  Fail:
    if ( node )
      FT_TS_Done_Face( face );    /* face must be in the driver's list */
    else if ( face )
      destroy_face( memory, face, driver );

  Exit:
#ifdef FT_TS_DEBUG_LEVEL_TRACE
    if ( !error && face_index < 0 )
    {
      FT_TS_TRACE3(( "FT_TS_Open_Face: The font has %ld face%s\n",
                  face->num_faces,
                  face->num_faces == 1 ? "" : "s" ));
      FT_TS_TRACE3(( "              and %ld named instance%s for face %ld\n",
                  face->style_flags >> 16,
                  ( face->style_flags >> 16 ) == 1 ? "" : "s",
                  -face_index - 1 ));
    }
#endif

    FT_TS_TRACE4(( "FT_TS_Open_Face: Return 0x%x\n", error ));

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Attach_File( FT_TS_Face      face,
                  const char*  filepathname )
  {
    FT_TS_Open_Args  open;


    /* test for valid `face' delayed to `FT_TS_Attach_Stream' */

    if ( !filepathname )
      return FT_TS_THROW( Invalid_Argument );

    open.stream   = NULL;
    open.flags    = FT_TS_OPEN_PATHNAME;
    open.pathname = (char*)filepathname;

    return FT_TS_Attach_Stream( face, &open );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Attach_Stream( FT_TS_Face        face,
                    FT_TS_Open_Args*  parameters )
  {
    FT_TS_Stream  stream;
    FT_TS_Error   error;
    FT_TS_Driver  driver;

    FT_TS_Driver_Class  clazz;


    /* test for valid `parameters' delayed to `FT_TS_Stream_New' */

    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    driver = face->driver;
    if ( !driver )
      return FT_TS_THROW( Invalid_Driver_Handle );

    error = FT_TS_Stream_New( driver->root.library, parameters, &stream );
    if ( error )
      goto Exit;

    /* we implement FT_TS_Attach_Stream in each driver through the */
    /* `attach_file' interface                                  */

    error = FT_TS_ERR( Unimplemented_Feature );
    clazz = driver->clazz;
    if ( clazz->attach_file )
      error = clazz->attach_file( face, stream );

    /* close the attached stream */
    FT_TS_Stream_Free( stream,
                    FT_TS_BOOL( parameters->stream                     &&
                             ( parameters->flags & FT_TS_OPEN_STREAM ) ) );

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Reference_Face( FT_TS_Face  face )
  {
    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    face->internal->refcount++;

    return FT_TS_Err_Ok;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Done_Face( FT_TS_Face  face )
  {
    FT_TS_Error     error;
    FT_TS_Driver    driver;
    FT_TS_Memory    memory;
    FT_TS_ListNode  node;


    error = FT_TS_ERR( Invalid_Face_Handle );
    if ( face && face->driver )
    {
      face->internal->refcount--;
      if ( face->internal->refcount > 0 )
        error = FT_TS_Err_Ok;
      else
      {
        driver = face->driver;
        memory = driver->root.memory;

        /* find face in driver's list */
        node = FT_TS_List_Find( &driver->faces_list, face );
        if ( node )
        {
          /* remove face object from the driver's list */
          FT_TS_List_Remove( &driver->faces_list, node );
          FT_TS_FREE( node );

          /* now destroy the object proper */
          destroy_face( memory, face, driver );
          error = FT_TS_Err_Ok;
        }
      }
    }

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_New_Size( FT_TS_Face   face,
               FT_TS_Size  *asize )
  {
    FT_TS_Error         error;
    FT_TS_Memory        memory;
    FT_TS_Driver        driver;
    FT_TS_Driver_Class  clazz;

    FT_TS_Size          size = NULL;
    FT_TS_ListNode      node = NULL;

    FT_TS_Size_Internal  internal = NULL;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !asize )
      return FT_TS_THROW( Invalid_Argument );

    if ( !face->driver )
      return FT_TS_THROW( Invalid_Driver_Handle );

    *asize = NULL;

    driver = face->driver;
    clazz  = driver->clazz;
    memory = face->memory;

    /* Allocate new size object and perform basic initialisation */
    if ( FT_TS_ALLOC( size, clazz->size_object_size ) || FT_TS_QNEW( node ) )
      goto Exit;

    size->face = face;

    if ( FT_TS_NEW( internal ) )
      goto Exit;

    size->internal = internal;

    if ( clazz->init_size )
      error = clazz->init_size( size );

    /* in case of success, add to the face's list */
    if ( !error )
    {
      *asize     = size;
      node->data = size;
      FT_TS_List_Add( &face->sizes_list, node );
    }

  Exit:
    if ( error )
    {
      FT_TS_FREE( node );
      if ( size )
        FT_TS_FREE( size->internal );
      FT_TS_FREE( size );
    }

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Done_Size( FT_TS_Size  size )
  {
    FT_TS_Error     error;
    FT_TS_Driver    driver;
    FT_TS_Memory    memory;
    FT_TS_Face      face;
    FT_TS_ListNode  node;


    if ( !size )
      return FT_TS_THROW( Invalid_Size_Handle );

    face = size->face;
    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    driver = face->driver;
    if ( !driver )
      return FT_TS_THROW( Invalid_Driver_Handle );

    memory = driver->root.memory;

    error = FT_TS_Err_Ok;
    node  = FT_TS_List_Find( &face->sizes_list, size );
    if ( node )
    {
      FT_TS_List_Remove( &face->sizes_list, node );
      FT_TS_FREE( node );

      if ( face->size == size )
      {
        face->size = NULL;
        if ( face->sizes_list.head )
          face->size = (FT_TS_Size)(face->sizes_list.head->data);
      }

      destroy_size( memory, size, driver );
    }
    else
      error = FT_TS_THROW( Invalid_Size_Handle );

    return error;
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Match_Size( FT_TS_Face          face,
                 FT_TS_Size_Request  req,
                 FT_TS_Bool          ignore_width,
                 FT_TS_ULong*        size_index )
  {
    FT_TS_Int   i;
    FT_TS_Long  w, h;


    if ( !FT_TS_HAS_FIXED_SIZES( face ) )
      return FT_TS_THROW( Invalid_Face_Handle );

    /* FT_TS_Bitmap_Size doesn't provide enough info... */
    if ( req->type != FT_TS_SIZE_REQUEST_TYPE_NOMINAL )
      return FT_TS_THROW( Unimplemented_Feature );

    w = FT_TS_REQUEST_WIDTH ( req );
    h = FT_TS_REQUEST_HEIGHT( req );

    if ( req->width && !req->height )
      h = w;
    else if ( !req->width && req->height )
      w = h;

    w = FT_TS_PIX_ROUND( w );
    h = FT_TS_PIX_ROUND( h );

    if ( !w || !h )
      return FT_TS_THROW( Invalid_Pixel_Size );

    for ( i = 0; i < face->num_fixed_sizes; i++ )
    {
      FT_TS_Bitmap_Size*  bsize = face->available_sizes + i;


      if ( h != FT_TS_PIX_ROUND( bsize->y_ppem ) )
        continue;

      if ( w == FT_TS_PIX_ROUND( bsize->x_ppem ) || ignore_width )
      {
        FT_TS_TRACE3(( "FT_TS_Match_Size: bitmap strike %d matches\n", i ));

        if ( size_index )
          *size_index = (FT_TS_ULong)i;

        return FT_TS_Err_Ok;
      }
    }

    FT_TS_TRACE3(( "FT_TS_Match_Size: no matching bitmap strike\n" ));

    return FT_TS_THROW( Invalid_Pixel_Size );
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( void )
  ft_synthesize_vertical_metrics( FT_TS_Glyph_Metrics*  metrics,
                                  FT_TS_Pos             advance )
  {
    FT_TS_Pos  height = metrics->height;


    /* compensate for glyph with bbox above/below the baseline */
    if ( metrics->horiBearingY < 0 )
    {
      if ( height < metrics->horiBearingY )
        height = metrics->horiBearingY;
    }
    else if ( metrics->horiBearingY > 0 )
      height -= metrics->horiBearingY;

    /* the factor 1.2 is a heuristical value */
    if ( !advance )
      advance = height * 12 / 10;

    metrics->vertBearingX = metrics->horiBearingX - metrics->horiAdvance / 2;
    metrics->vertBearingY = ( advance - height ) / 2;
    metrics->vertAdvance  = advance;
  }


  static void
  ft_recompute_scaled_metrics( FT_TS_Face           face,
                               FT_TS_Size_Metrics*  metrics )
  {
    /* Compute root ascender, descender, test height, and max_advance */

#ifdef GRID_FIT_METRICS
    metrics->ascender    = FT_TS_PIX_CEIL( FT_TS_MulFix( face->ascender,
                                                   metrics->y_scale ) );

    metrics->descender   = FT_TS_PIX_FLOOR( FT_TS_MulFix( face->descender,
                                                    metrics->y_scale ) );

    metrics->height      = FT_TS_PIX_ROUND( FT_TS_MulFix( face->height,
                                                    metrics->y_scale ) );

    metrics->max_advance = FT_TS_PIX_ROUND( FT_TS_MulFix( face->max_advance_width,
                                                    metrics->x_scale ) );
#else /* !GRID_FIT_METRICS */
    metrics->ascender    = FT_TS_MulFix( face->ascender,
                                      metrics->y_scale );

    metrics->descender   = FT_TS_MulFix( face->descender,
                                      metrics->y_scale );

    metrics->height      = FT_TS_MulFix( face->height,
                                      metrics->y_scale );

    metrics->max_advance = FT_TS_MulFix( face->max_advance_width,
                                      metrics->x_scale );
#endif /* !GRID_FIT_METRICS */
  }


  FT_TS_BASE_DEF( void )
  FT_TS_Select_Metrics( FT_TS_Face   face,
                     FT_TS_ULong  strike_index )
  {
    FT_TS_Size_Metrics*  metrics;
    FT_TS_Bitmap_Size*   bsize;


    metrics = &face->size->metrics;
    bsize   = face->available_sizes + strike_index;

    metrics->x_ppem = (FT_TS_UShort)( ( bsize->x_ppem + 32 ) >> 6 );
    metrics->y_ppem = (FT_TS_UShort)( ( bsize->y_ppem + 32 ) >> 6 );

    if ( FT_TS_IS_SCALABLE( face ) )
    {
      metrics->x_scale = FT_TS_DivFix( bsize->x_ppem,
                                    face->units_per_EM );
      metrics->y_scale = FT_TS_DivFix( bsize->y_ppem,
                                    face->units_per_EM );

      ft_recompute_scaled_metrics( face, metrics );
    }
    else
    {
      metrics->x_scale     = 1L << 16;
      metrics->y_scale     = 1L << 16;
      metrics->ascender    = bsize->y_ppem;
      metrics->descender   = 0;
      metrics->height      = bsize->height << 6;
      metrics->max_advance = bsize->x_ppem;
    }
  }


  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Request_Metrics( FT_TS_Face          face,
                      FT_TS_Size_Request  req )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;

    FT_TS_Size_Metrics*  metrics;


    metrics = &face->size->metrics;

    if ( FT_TS_IS_SCALABLE( face ) )
    {
      FT_TS_Long  w = 0, h = 0, scaled_w = 0, scaled_h = 0;


      switch ( req->type )
      {
      case FT_TS_SIZE_REQUEST_TYPE_NOMINAL:
        w = h = face->units_per_EM;
        break;

      case FT_TS_SIZE_REQUEST_TYPE_REAL_DIM:
        w = h = face->ascender - face->descender;
        break;

      case FT_TS_SIZE_REQUEST_TYPE_BBOX:
        w = face->bbox.xMax - face->bbox.xMin;
        h = face->bbox.yMax - face->bbox.yMin;
        break;

      case FT_TS_SIZE_REQUEST_TYPE_CELL:
        w = face->max_advance_width;
        h = face->ascender - face->descender;
        break;

      case FT_TS_SIZE_REQUEST_TYPE_SCALES:
        metrics->x_scale = (FT_TS_Fixed)req->width;
        metrics->y_scale = (FT_TS_Fixed)req->height;
        if ( !metrics->x_scale )
          metrics->x_scale = metrics->y_scale;
        else if ( !metrics->y_scale )
          metrics->y_scale = metrics->x_scale;
        goto Calculate_Ppem;

      case FT_TS_SIZE_REQUEST_TYPE_MAX:
        break;
      }

      /* to be on the safe side */
      if ( w < 0 )
        w = -w;

      if ( h < 0 )
        h = -h;

      scaled_w = FT_TS_REQUEST_WIDTH ( req );
      scaled_h = FT_TS_REQUEST_HEIGHT( req );

      /* determine scales */
      if ( req->width )
      {
        metrics->x_scale = FT_TS_DivFix( scaled_w, w );

        if ( req->height )
        {
          metrics->y_scale = FT_TS_DivFix( scaled_h, h );

          if ( req->type == FT_TS_SIZE_REQUEST_TYPE_CELL )
          {
            if ( metrics->y_scale > metrics->x_scale )
              metrics->y_scale = metrics->x_scale;
            else
              metrics->x_scale = metrics->y_scale;
          }
        }
        else
        {
          metrics->y_scale = metrics->x_scale;
          scaled_h = FT_TS_MulDiv( scaled_w, h, w );
        }
      }
      else
      {
        metrics->x_scale = metrics->y_scale = FT_TS_DivFix( scaled_h, h );
        scaled_w = FT_TS_MulDiv( scaled_h, w, h );
      }

  Calculate_Ppem:
      /* calculate the ppems */
      if ( req->type != FT_TS_SIZE_REQUEST_TYPE_NOMINAL )
      {
        scaled_w = FT_TS_MulFix( face->units_per_EM, metrics->x_scale );
        scaled_h = FT_TS_MulFix( face->units_per_EM, metrics->y_scale );
      }

      scaled_w = ( scaled_w + 32 ) >> 6;
      scaled_h = ( scaled_h + 32 ) >> 6;
      if ( scaled_w > (FT_TS_Long)FT_TS_USHORT_MAX ||
           scaled_h > (FT_TS_Long)FT_TS_USHORT_MAX )
      {
        FT_TS_ERROR(( "FT_TS_Request_Metrics: Resulting ppem size too large\n" ));
        error = FT_TS_ERR( Invalid_Pixel_Size );
        goto Exit;
      }

      metrics->x_ppem = (FT_TS_UShort)scaled_w;
      metrics->y_ppem = (FT_TS_UShort)scaled_h;

      ft_recompute_scaled_metrics( face, metrics );
    }
    else
    {
      FT_TS_ZERO( metrics );
      metrics->x_scale = 1L << 16;
      metrics->y_scale = 1L << 16;
    }

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Select_Size( FT_TS_Face  face,
                  FT_TS_Int   strike_index )
  {
    FT_TS_Error         error = FT_TS_Err_Ok;
    FT_TS_Driver_Class  clazz;


    if ( !face || !FT_TS_HAS_FIXED_SIZES( face ) )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( strike_index < 0 || strike_index >= face->num_fixed_sizes )
      return FT_TS_THROW( Invalid_Argument );

    clazz = face->driver->clazz;

    if ( clazz->select_size )
    {
      error = clazz->select_size( face->size, (FT_TS_ULong)strike_index );

      FT_TS_TRACE5(( "FT_TS_Select_Size (%s driver):\n",
                  face->driver->root.clazz->module_name ));
    }
    else
    {
      FT_TS_Select_Metrics( face, (FT_TS_ULong)strike_index );

      FT_TS_TRACE5(( "FT_TS_Select_Size:\n" ));
    }

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    {
      FT_TS_Size_Metrics*  metrics = &face->size->metrics;


      FT_TS_TRACE5(( "  x scale: %ld (%f)\n",
                  metrics->x_scale, metrics->x_scale / 65536.0 ));
      FT_TS_TRACE5(( "  y scale: %ld (%f)\n",
                  metrics->y_scale, metrics->y_scale / 65536.0 ));
      FT_TS_TRACE5(( "  ascender: %f\n",    metrics->ascender / 64.0 ));
      FT_TS_TRACE5(( "  descender: %f\n",   metrics->descender / 64.0 ));
      FT_TS_TRACE5(( "  height: %f\n",      metrics->height / 64.0 ));
      FT_TS_TRACE5(( "  max advance: %f\n", metrics->max_advance / 64.0 ));
      FT_TS_TRACE5(( "  x ppem: %d\n",      metrics->x_ppem ));
      FT_TS_TRACE5(( "  y ppem: %d\n",      metrics->y_ppem ));
    }
#endif

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Request_Size( FT_TS_Face          face,
                   FT_TS_Size_Request  req )
  {
    FT_TS_Error         error;
    FT_TS_Driver_Class  clazz;
    FT_TS_ULong         strike_index;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !face->size )
      return FT_TS_THROW( Invalid_Size_Handle );

    if ( !req || req->width < 0 || req->height < 0 ||
         req->type >= FT_TS_SIZE_REQUEST_TYPE_MAX )
      return FT_TS_THROW( Invalid_Argument );

    /* signal the auto-hinter to recompute its size metrics */
    /* (if requested)                                       */
    face->size->internal->autohint_metrics.x_scale = 0;

    clazz = face->driver->clazz;

    if ( clazz->request_size )
    {
      error = clazz->request_size( face->size, req );

      FT_TS_TRACE5(( "FT_TS_Request_Size (%s driver):\n",
                  face->driver->root.clazz->module_name ));
    }
    else if ( !FT_TS_IS_SCALABLE( face ) && FT_TS_HAS_FIXED_SIZES( face ) )
    {
      /*
       * The reason that a driver doesn't have `request_size' defined is
       * either that the scaling here suffices or that the supported formats
       * are bitmap-only and size matching is not implemented.
       *
       * In the latter case, a simple size matching is done.
       */
      error = FT_TS_Match_Size( face, req, 0, &strike_index );
      if ( error )
        goto Exit;

      return FT_TS_Select_Size( face, (FT_TS_Int)strike_index );
    }
    else
    {
      error = FT_TS_Request_Metrics( face, req );
      if ( error )
        goto Exit;

      FT_TS_TRACE5(( "FT_TS_Request_Size:\n" ));
    }

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    {
      FT_TS_Size_Metrics*  metrics = &face->size->metrics;


      FT_TS_TRACE5(( "  x scale: %ld (%f)\n",
                  metrics->x_scale, metrics->x_scale / 65536.0 ));
      FT_TS_TRACE5(( "  y scale: %ld (%f)\n",
                  metrics->y_scale, metrics->y_scale / 65536.0 ));
      FT_TS_TRACE5(( "  ascender: %f\n",    metrics->ascender / 64.0 ));
      FT_TS_TRACE5(( "  descender: %f\n",   metrics->descender / 64.0 ));
      FT_TS_TRACE5(( "  height: %f\n",      metrics->height / 64.0 ));
      FT_TS_TRACE5(( "  max advance: %f\n", metrics->max_advance / 64.0 ));
      FT_TS_TRACE5(( "  x ppem: %d\n",      metrics->x_ppem ));
      FT_TS_TRACE5(( "  y ppem: %d\n",      metrics->y_ppem ));
    }
#endif

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Char_Size( FT_TS_Face     face,
                    FT_TS_F26Dot6  char_width,
                    FT_TS_F26Dot6  char_height,
                    FT_TS_UInt     horz_resolution,
                    FT_TS_UInt     vert_resolution )
  {
    FT_TS_Size_RequestRec  req;


    /* check of `face' delayed to `FT_TS_Request_Size' */

    if ( !char_width )
      char_width = char_height;
    else if ( !char_height )
      char_height = char_width;

    if ( !horz_resolution )
      horz_resolution = vert_resolution;
    else if ( !vert_resolution )
      vert_resolution = horz_resolution;

    if ( char_width  < 1 * 64 )
      char_width  = 1 * 64;
    if ( char_height < 1 * 64 )
      char_height = 1 * 64;

    if ( !horz_resolution )
      horz_resolution = vert_resolution = 72;

    req.type           = FT_TS_SIZE_REQUEST_TYPE_NOMINAL;
    req.width          = char_width;
    req.height         = char_height;
    req.horiResolution = horz_resolution;
    req.vertResolution = vert_resolution;

    return FT_TS_Request_Size( face, &req );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Pixel_Sizes( FT_TS_Face  face,
                      FT_TS_UInt  pixel_width,
                      FT_TS_UInt  pixel_height )
  {
    FT_TS_Size_RequestRec  req;


    /* check of `face' delayed to `FT_TS_Request_Size' */

    if ( pixel_width == 0 )
      pixel_width = pixel_height;
    else if ( pixel_height == 0 )
      pixel_height = pixel_width;

    if ( pixel_width  < 1 )
      pixel_width  = 1;
    if ( pixel_height < 1 )
      pixel_height = 1;

    /* use `>=' to avoid potential compiler warning on 16bit platforms */
    if ( pixel_width >= 0xFFFFU )
      pixel_width = 0xFFFFU;
    if ( pixel_height >= 0xFFFFU )
      pixel_height = 0xFFFFU;

    req.type           = FT_TS_SIZE_REQUEST_TYPE_NOMINAL;
    req.width          = (FT_TS_Long)( pixel_width << 6 );
    req.height         = (FT_TS_Long)( pixel_height << 6 );
    req.horiResolution = 0;
    req.vertResolution = 0;

    return FT_TS_Request_Size( face, &req );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Kerning( FT_TS_Face     face,
                  FT_TS_UInt     left_glyph,
                  FT_TS_UInt     right_glyph,
                  FT_TS_UInt     kern_mode,
                  FT_TS_Vector  *akerning )
  {
    FT_TS_Error   error = FT_TS_Err_Ok;
    FT_TS_Driver  driver;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !akerning )
      return FT_TS_THROW( Invalid_Argument );

    driver = face->driver;

    akerning->x = 0;
    akerning->y = 0;

    if ( driver->clazz->get_kerning )
    {
      error = driver->clazz->get_kerning( face,
                                          left_glyph,
                                          right_glyph,
                                          akerning );
      if ( !error )
      {
        if ( kern_mode != FT_TS_KERNING_UNSCALED )
        {
          akerning->x = FT_TS_MulFix( akerning->x, face->size->metrics.x_scale );
          akerning->y = FT_TS_MulFix( akerning->y, face->size->metrics.y_scale );

          if ( kern_mode != FT_TS_KERNING_UNFITTED )
          {
            FT_TS_Pos  orig_x = akerning->x;
            FT_TS_Pos  orig_y = akerning->y;


            /* we scale down kerning values for small ppem values */
            /* to avoid that rounding makes them too big.         */
            /* `25' has been determined heuristically.            */
            if ( face->size->metrics.x_ppem < 25 )
              akerning->x = FT_TS_MulDiv( orig_x,
                                       face->size->metrics.x_ppem, 25 );
            if ( face->size->metrics.y_ppem < 25 )
              akerning->y = FT_TS_MulDiv( orig_y,
                                       face->size->metrics.y_ppem, 25 );

            akerning->x = FT_TS_PIX_ROUND( akerning->x );
            akerning->y = FT_TS_PIX_ROUND( akerning->y );

#ifdef FT_TS_DEBUG_LEVEL_TRACE
            {
              FT_TS_Pos  orig_x_rounded = FT_TS_PIX_ROUND( orig_x );
              FT_TS_Pos  orig_y_rounded = FT_TS_PIX_ROUND( orig_y );


              if ( akerning->x != orig_x_rounded ||
                   akerning->y != orig_y_rounded )
                FT_TS_TRACE5(( "FT_TS_Get_Kerning: horizontal kerning"
                            " (%ld, %ld) scaled down to (%ld, %ld) pixels\n",
                            orig_x_rounded / 64, orig_y_rounded / 64,
                            akerning->x / 64, akerning->y / 64 ));
            }
#endif
          }
        }
      }
    }

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Track_Kerning( FT_TS_Face    face,
                        FT_TS_Fixed   point_size,
                        FT_TS_Int     degree,
                        FT_TS_Fixed*  akerning )
  {
    FT_TS_Service_Kerning  service;
    FT_TS_Error            error = FT_TS_Err_Ok;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !akerning )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_FACE_FIND_SERVICE( face, service, KERNING );
    if ( !service )
      return FT_TS_THROW( Unimplemented_Feature );

    error = service->get_track( face,
                                point_size,
                                degree,
                                akerning );

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Select_Charmap( FT_TS_Face      face,
                     FT_TS_Encoding  encoding )
  {
    FT_TS_CharMap*  cur;
    FT_TS_CharMap*  limit;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    /* FT_TS_ENCODING_NONE is a valid encoding for BDF, PCF, and Windows FNT */
    if ( encoding == FT_TS_ENCODING_NONE && !face->num_charmaps )
      return FT_TS_THROW( Invalid_Argument );

    /* FT_TS_ENCODING_UNICODE is special.  We try to find the `best' Unicode */
    /* charmap available, i.e., one with UCS-4 characters, if possible.   */
    /*                                                                    */
    /* This is done by find_unicode_charmap() above, to share code.       */
    if ( encoding == FT_TS_ENCODING_UNICODE )
      return find_unicode_charmap( face );

    cur = face->charmaps;
    if ( !cur )
      return FT_TS_THROW( Invalid_CharMap_Handle );

    limit = cur + face->num_charmaps;

    for ( ; cur < limit; cur++ )
    {
      if ( cur[0]->encoding == encoding )
      {
        face->charmap = cur[0];
        return FT_TS_Err_Ok;
      }
    }

    return FT_TS_THROW( Invalid_Argument );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Charmap( FT_TS_Face     face,
                  FT_TS_CharMap  charmap )
  {
    FT_TS_CharMap*  cur;
    FT_TS_CharMap*  limit;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    cur = face->charmaps;
    if ( !cur || !charmap )
      return FT_TS_THROW( Invalid_CharMap_Handle );

    limit = cur + face->num_charmaps;

    for ( ; cur < limit; cur++ )
    {
      if ( cur[0] == charmap                    &&
           FT_TS_Get_CMap_Format ( charmap ) != 14 )
      {
        face->charmap = cur[0];
        return FT_TS_Err_Ok;
      }
    }

    return FT_TS_THROW( Invalid_Argument );
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Int )
  FT_TS_Get_Charmap_Index( FT_TS_CharMap  charmap )
  {
    FT_TS_Int  i;


    if ( !charmap || !charmap->face )
      return -1;

    for ( i = 0; i < charmap->face->num_charmaps; i++ )
      if ( charmap->face->charmaps[i] == charmap )
        break;

    FT_TS_ASSERT( i < charmap->face->num_charmaps );

    return i;
  }


  static void
  ft_cmap_done_internal( FT_TS_CMap  cmap )
  {
    FT_TS_CMap_Class  clazz  = cmap->clazz;
    FT_TS_Face        face   = cmap->charmap.face;
    FT_TS_Memory      memory = FT_TS_FACE_MEMORY( face );


    if ( clazz->done )
      clazz->done( cmap );

    FT_TS_FREE( cmap );
  }


  FT_TS_BASE_DEF( void )
  FT_TS_CMap_Done( FT_TS_CMap  cmap )
  {
    if ( cmap )
    {
      FT_TS_Face    face   = cmap->charmap.face;
      FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );
      FT_TS_Error   error;
      FT_TS_Int     i, j;


      for ( i = 0; i < face->num_charmaps; i++ )
      {
        if ( (FT_TS_CMap)face->charmaps[i] == cmap )
        {
          FT_TS_CharMap  last_charmap = face->charmaps[face->num_charmaps - 1];


          if ( FT_TS_QRENEW_ARRAY( face->charmaps,
                                face->num_charmaps,
                                face->num_charmaps - 1 ) )
            return;

          /* remove it from our list of charmaps */
          for ( j = i + 1; j < face->num_charmaps; j++ )
          {
            if ( j == face->num_charmaps - 1 )
              face->charmaps[j - 1] = last_charmap;
            else
              face->charmaps[j - 1] = face->charmaps[j];
          }

          face->num_charmaps--;

          if ( (FT_TS_CMap)face->charmap == cmap )
            face->charmap = NULL;

          ft_cmap_done_internal( cmap );

          break;
        }
      }
    }
  }


  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_CMap_New( FT_TS_CMap_Class  clazz,
               FT_TS_Pointer     init_data,
               FT_TS_CharMap     charmap,
               FT_TS_CMap       *acmap )
  {
    FT_TS_Error   error;
    FT_TS_Face    face;
    FT_TS_Memory  memory;
    FT_TS_CMap    cmap = NULL;


    if ( !clazz || !charmap || !charmap->face )
      return FT_TS_THROW( Invalid_Argument );

    face   = charmap->face;
    memory = FT_TS_FACE_MEMORY( face );

    if ( !FT_TS_ALLOC( cmap, clazz->size ) )
    {
      cmap->charmap = *charmap;
      cmap->clazz   = clazz;

      if ( clazz->init )
      {
        error = clazz->init( cmap, init_data );
        if ( error )
          goto Fail;
      }

      /* add it to our list of charmaps */
      if ( FT_TS_QRENEW_ARRAY( face->charmaps,
                            face->num_charmaps,
                            face->num_charmaps + 1 ) )
        goto Fail;

      face->charmaps[face->num_charmaps++] = (FT_TS_CharMap)cmap;
    }

  Exit:
    if ( acmap )
      *acmap = cmap;

    return error;

  Fail:
    ft_cmap_done_internal( cmap );
    cmap = NULL;
    goto Exit;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt )
  FT_TS_Get_Char_Index( FT_TS_Face   face,
                     FT_TS_ULong  charcode )
  {
    FT_TS_UInt  result = 0;


    if ( face && face->charmap )
    {
      FT_TS_CMap  cmap = FT_TS_CMAP( face->charmap );


      if ( charcode > 0xFFFFFFFFUL )
      {
        FT_TS_TRACE1(( "FT_TS_Get_Char_Index: too large charcode" ));
        FT_TS_TRACE1(( " 0x%lx is truncated\n", charcode ));
      }

      result = cmap->clazz->char_index( cmap, (FT_TS_UInt32)charcode );
      if ( result >= (FT_TS_UInt)face->num_glyphs )
        result = 0;
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_ULong )
  FT_TS_Get_First_Char( FT_TS_Face   face,
                     FT_TS_UInt  *agindex )
  {
    FT_TS_ULong  result = 0;
    FT_TS_UInt   gindex = 0;


    /* only do something if we have a charmap, and we have glyphs at all */
    if ( face && face->charmap && face->num_glyphs )
    {
      gindex = FT_TS_Get_Char_Index( face, 0 );
      if ( gindex == 0 )
        result = FT_TS_Get_Next_Char( face, 0, &gindex );
    }

    if ( agindex )
      *agindex = gindex;

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_ULong )
  FT_TS_Get_Next_Char( FT_TS_Face   face,
                    FT_TS_ULong  charcode,
                    FT_TS_UInt  *agindex )
  {
    FT_TS_ULong  result = 0;
    FT_TS_UInt   gindex = 0;


    if ( face && face->charmap && face->num_glyphs )
    {
      FT_TS_UInt32  code = (FT_TS_UInt32)charcode;
      FT_TS_CMap    cmap = FT_TS_CMAP( face->charmap );


      do
      {
        gindex = cmap->clazz->char_next( cmap, &code );

      } while ( gindex >= (FT_TS_UInt)face->num_glyphs );

      result = ( gindex == 0 ) ? 0 : code;
    }

    if ( agindex )
      *agindex = gindex;

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Face_Properties( FT_TS_Face        face,
                      FT_TS_UInt        num_properties,
                      FT_TS_Parameter*  properties )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    if ( num_properties > 0 && !properties )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    for ( ; num_properties > 0; num_properties-- )
    {
      if ( properties->tag == FT_TS_PARAM_TAG_STEM_DARKENING )
      {
        if ( properties->data )
        {
          if ( *( (FT_TS_Bool*)properties->data ) == TRUE )
            face->internal->no_stem_darkening = FALSE;
          else
            face->internal->no_stem_darkening = TRUE;
        }
        else
        {
          /* use module default */
          face->internal->no_stem_darkening = -1;
        }
      }
      else if ( properties->tag == FT_TS_PARAM_TAG_LCD_FILTER_WEIGHTS )
      {
#ifdef FT_TS_CONFIG_OPTION_SUBPIXEL_RENDERING
        if ( properties->data )
        {
          ft_memcpy( face->internal->lcd_weights,
                     properties->data,
                     FT_TS_LCD_FILTER_FIVE_TAPS );
          face->internal->lcd_filter_func = ft_lcd_filter_fir;
        }
#else
        error = FT_TS_THROW( Unimplemented_Feature );
        goto Exit;
#endif
      }
      else if ( properties->tag == FT_TS_PARAM_TAG_RANDOM_SEED )
      {
        if ( properties->data )
        {
          face->internal->random_seed = *( (FT_TS_Int32*)properties->data );
          if ( face->internal->random_seed < 0 )
            face->internal->random_seed = 0;
        }
        else
        {
          /* use module default */
          face->internal->random_seed = -1;
        }
      }
      else
      {
        error = FT_TS_THROW( Invalid_Argument );
        goto Exit;
      }

      if ( error )
        break;

      properties++;
    }

  Exit:
    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt )
  FT_TS_Face_GetCharVariantIndex( FT_TS_Face   face,
                               FT_TS_ULong  charcode,
                               FT_TS_ULong  variantSelector )
  {
    FT_TS_UInt  result = 0;


    if ( face                                           &&
         face->charmap                                  &&
         face->charmap->encoding == FT_TS_ENCODING_UNICODE )
    {
      FT_TS_CharMap  charmap = find_variant_selector_charmap( face );
      FT_TS_CMap     ucmap = FT_TS_CMAP( face->charmap );


      if ( charmap )
      {
        FT_TS_CMap  vcmap = FT_TS_CMAP( charmap );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Face_GetCharVariantIndex:"
                      " too large charcode" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", charcode ));
        }
        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Face_GetCharVariantIndex:"
                      " too large variantSelector" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->char_var_index( vcmap, ucmap,
                                               (FT_TS_UInt32)charcode,
                                               (FT_TS_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Int )
  FT_TS_Face_GetCharVariantIsDefault( FT_TS_Face   face,
                                   FT_TS_ULong  charcode,
                                   FT_TS_ULong  variantSelector )
  {
    FT_TS_Int  result = -1;


    if ( face )
    {
      FT_TS_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap )
      {
        FT_TS_CMap  vcmap = FT_TS_CMAP( charmap );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Face_GetCharVariantIsDefault:"
                      " too large charcode" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", charcode ));
        }
        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Face_GetCharVariantIsDefault:"
                      " too large variantSelector" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->char_var_default( vcmap,
                                                 (FT_TS_UInt32)charcode,
                                                 (FT_TS_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt32* )
  FT_TS_Face_GetVariantSelectors( FT_TS_Face  face )
  {
    FT_TS_UInt32  *result = NULL;


    if ( face )
    {
      FT_TS_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap )
      {
        FT_TS_CMap    vcmap  = FT_TS_CMAP( charmap );
        FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


        result = vcmap->clazz->variant_list( vcmap, memory );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt32* )
  FT_TS_Face_GetVariantsOfChar( FT_TS_Face   face,
                             FT_TS_ULong  charcode )
  {
    FT_TS_UInt32  *result = NULL;


    if ( face )
    {
      FT_TS_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap )
      {
        FT_TS_CMap    vcmap  = FT_TS_CMAP( charmap );
        FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


        if ( charcode > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Face_GetVariantsOfChar: too large charcode" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", charcode ));
        }

        result = vcmap->clazz->charvariant_list( vcmap, memory,
                                                 (FT_TS_UInt32)charcode );
      }
    }
    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt32* )
  FT_TS_Face_GetCharsOfVariant( FT_TS_Face   face,
                             FT_TS_ULong  variantSelector )
  {
    FT_TS_UInt32  *result = NULL;


    if ( face )
    {
      FT_TS_CharMap  charmap = find_variant_selector_charmap( face );


      if ( charmap )
      {
        FT_TS_CMap    vcmap  = FT_TS_CMAP( charmap );
        FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


        if ( variantSelector > 0xFFFFFFFFUL )
        {
          FT_TS_TRACE1(( "FT_TS_Get_Char_Index: too large variantSelector" ));
          FT_TS_TRACE1(( " 0x%lx is truncated\n", variantSelector ));
        }

        result = vcmap->clazz->variantchar_list( vcmap, memory,
                                                 (FT_TS_UInt32)variantSelector );
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt )
  FT_TS_Get_Name_Index( FT_TS_Face           face,
                     const FT_TS_String*  glyph_name )
  {
    FT_TS_UInt  result = 0;


    if ( face                       &&
         FT_TS_HAS_GLYPH_NAMES( face ) &&
         glyph_name                 )
    {
      FT_TS_Service_GlyphDict  service;


      FT_TS_FACE_LOOKUP_SERVICE( face,
                              service,
                              GLYPH_DICT );

      if ( service && service->name_index )
        result = service->name_index( face, glyph_name );
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Glyph_Name( FT_TS_Face     face,
                     FT_TS_UInt     glyph_index,
                     FT_TS_Pointer  buffer,
                     FT_TS_UInt     buffer_max )
  {
    FT_TS_Error              error;
    FT_TS_Service_GlyphDict  service;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !buffer || buffer_max == 0 )
      return FT_TS_THROW( Invalid_Argument );

    /* clean up buffer */
    ((FT_TS_Byte*)buffer)[0] = '\0';

    if ( (FT_TS_Long)glyph_index >= face->num_glyphs )
      return FT_TS_THROW( Invalid_Glyph_Index );

    if ( !FT_TS_HAS_GLYPH_NAMES( face ) )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_FACE_LOOKUP_SERVICE( face, service, GLYPH_DICT );
    if ( service && service->get_name )
      error = service->get_name( face, glyph_index, buffer, buffer_max );
    else
      error = FT_TS_THROW( Invalid_Argument );

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( const char* )
  FT_TS_Get_Postscript_Name( FT_TS_Face  face )
  {
    const char*  result = NULL;


    if ( !face )
      goto Exit;

    if ( !result )
    {
      FT_TS_Service_PsFontName  service;


      FT_TS_FACE_LOOKUP_SERVICE( face,
                              service,
                              POSTSCRIPT_FONT_NAME );

      if ( service && service->get_ps_font_name )
        result = service->get_ps_font_name( face );
    }

  Exit:
    return result;
  }


  /* documentation is in tttables.h */

  FT_TS_EXPORT_DEF( void* )
  FT_TS_Get_Sfnt_Table( FT_TS_Face      face,
                     FT_TS_Sfnt_Tag  tag )
  {
    void*                  table = NULL;
    FT_TS_Service_SFNT_Table  service;


    if ( face && FT_TS_IS_SFNT( face ) )
    {
      FT_TS_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
      if ( service )
        table = service->get_table( face, tag );
    }

    return table;
  }


  /* documentation is in tttables.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Load_Sfnt_Table( FT_TS_Face    face,
                      FT_TS_ULong   tag,
                      FT_TS_Long    offset,
                      FT_TS_Byte*   buffer,
                      FT_TS_ULong*  length )
  {
    FT_TS_Service_SFNT_Table  service;


    if ( !face || !FT_TS_IS_SFNT( face ) )
      return FT_TS_THROW( Invalid_Face_Handle );

    FT_TS_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
    if ( !service )
      return FT_TS_THROW( Unimplemented_Feature );

    return service->load_table( face, tag, offset, buffer, length );
  }


  /* documentation is in tttables.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Sfnt_Table_Info( FT_TS_Face    face,
                      FT_TS_UInt    table_index,
                      FT_TS_ULong  *tag,
                      FT_TS_ULong  *length )
  {
    FT_TS_Service_SFNT_Table  service;
    FT_TS_ULong               offset;


    /* test for valid `length' delayed to `service->table_info' */

    if ( !face || !FT_TS_IS_SFNT( face ) )
      return FT_TS_THROW( Invalid_Face_Handle );

    FT_TS_FACE_FIND_SERVICE( face, service, SFNT_TABLE );
    if ( !service )
      return FT_TS_THROW( Unimplemented_Feature );

    return service->table_info( face, table_index, tag, &offset, length );
  }


  /* documentation is in tttables.h */

  FT_TS_EXPORT_DEF( FT_TS_ULong )
  FT_TS_Get_CMap_Language_ID( FT_TS_CharMap  charmap )
  {
    FT_TS_Service_TTCMaps  service;
    FT_TS_Face             face;
    TT_CMapInfo         cmap_info;


    if ( !charmap || !charmap->face )
      return 0;

    face = charmap->face;
    FT_TS_FACE_FIND_SERVICE( face, service, TT_CMAP );
    if ( !service )
      return 0;
    if ( service->get_cmap_info( charmap, &cmap_info ))
      return 0;

    return cmap_info.language;
  }


  /* documentation is in tttables.h */

  FT_TS_EXPORT_DEF( FT_TS_Long )
  FT_TS_Get_CMap_Format( FT_TS_CharMap  charmap )
  {
    FT_TS_Service_TTCMaps  service;
    FT_TS_Face             face;
    TT_CMapInfo         cmap_info;


    if ( !charmap || !charmap->face )
      return -1;

    face = charmap->face;
    FT_TS_FACE_FIND_SERVICE( face, service, TT_CMAP );
    if ( !service )
      return -1;
    if ( service->get_cmap_info( charmap, &cmap_info ))
      return -1;

    return cmap_info.format;
  }


  /* documentation is in ftsizes.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Activate_Size( FT_TS_Size  size )
  {
    FT_TS_Face  face;


    if ( !size )
      return FT_TS_THROW( Invalid_Size_Handle );

    face = size->face;
    if ( !face || !face->driver )
      return FT_TS_THROW( Invalid_Face_Handle );

    /* we don't need anything more complex than that; all size objects */
    /* are already listed by the face                                  */
    face->size = size;

    return FT_TS_Err_Ok;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                        R E N D E R E R S                        ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /* lookup a renderer by glyph format in the library's list */
  FT_TS_BASE_DEF( FT_TS_Renderer )
  FT_TS_Lookup_Renderer( FT_TS_Library       library,
                      FT_TS_Glyph_Format  format,
                      FT_TS_ListNode*     node )
  {
    FT_TS_ListNode  cur;
    FT_TS_Renderer  result = NULL;


    if ( !library )
      goto Exit;

    cur = library->renderers.head;

    if ( node )
    {
      if ( *node )
        cur = (*node)->next;
      *node = NULL;
    }

    while ( cur )
    {
      FT_TS_Renderer  renderer = FT_TS_RENDERER( cur->data );


      if ( renderer->glyph_format == format )
      {
        if ( node )
          *node = cur;

        result = renderer;
        break;
      }
      cur = cur->next;
    }

  Exit:
    return result;
  }


  static FT_TS_Renderer
  ft_lookup_glyph_renderer( FT_TS_GlyphSlot  slot )
  {
    FT_TS_Face      face    = slot->face;
    FT_TS_Library   library = FT_TS_FACE_LIBRARY( face );
    FT_TS_Renderer  result  = library->cur_renderer;


    if ( !result || result->glyph_format != slot->format )
      result = FT_TS_Lookup_Renderer( library, slot->format, 0 );

    return result;
  }


  static void
  ft_set_current_renderer( FT_TS_Library  library )
  {
    FT_TS_Renderer  renderer;


    renderer = FT_TS_Lookup_Renderer( library, FT_TS_GLYPH_FORMAT_OUTLINE, 0 );
    library->cur_renderer = renderer;
  }


  static FT_TS_Error
  ft_add_renderer( FT_TS_Module  module )
  {
    FT_TS_Library   library = module->library;
    FT_TS_Memory    memory  = library->memory;
    FT_TS_Error     error;
    FT_TS_ListNode  node    = NULL;


    if ( FT_TS_QNEW( node ) )
      goto Exit;

    {
      FT_TS_Renderer         render = FT_TS_RENDERER( module );
      FT_TS_Renderer_Class*  clazz  = (FT_TS_Renderer_Class*)module->clazz;


      render->clazz        = clazz;
      render->glyph_format = clazz->glyph_format;

      /* allocate raster object if needed */
      if ( clazz->raster_class && clazz->raster_class->raster_new )
      {
        error = clazz->raster_class->raster_new( memory, &render->raster );
        if ( error )
          goto Fail;

        render->raster_render = clazz->raster_class->raster_render;
        render->render        = clazz->render_glyph;
      }

#ifdef FT_TS_CONFIG_OPTION_SVG
      if ( clazz->glyph_format == FT_TS_GLYPH_FORMAT_SVG )
        render->render = clazz->render_glyph;
#endif

      /* add to list */
      node->data = module;
      FT_TS_List_Add( &library->renderers, node );

      ft_set_current_renderer( library );
    }

  Fail:
    if ( error )
      FT_TS_FREE( node );

  Exit:
    return error;
  }


  static void
  ft_remove_renderer( FT_TS_Module  module )
  {
    FT_TS_Library   library;
    FT_TS_Memory    memory;
    FT_TS_ListNode  node;


    library = module->library;
    if ( !library )
      return;

    memory = library->memory;

    node = FT_TS_List_Find( &library->renderers, module );
    if ( node )
    {
      FT_TS_Renderer  render = FT_TS_RENDERER( module );


      /* release raster object, if any */
      if ( render->raster )
        render->clazz->raster_class->raster_done( render->raster );

      /* remove from list */
      FT_TS_List_Remove( &library->renderers, node );
      FT_TS_FREE( node );

      ft_set_current_renderer( library );
    }
  }


  /* documentation is in ftrender.h */

  FT_TS_EXPORT_DEF( FT_TS_Renderer )
  FT_TS_Get_Renderer( FT_TS_Library       library,
                   FT_TS_Glyph_Format  format )
  {
    /* test for valid `library' delayed to `FT_TS_Lookup_Renderer' */

    return FT_TS_Lookup_Renderer( library, format, 0 );
  }


  /* documentation is in ftrender.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Renderer( FT_TS_Library     library,
                   FT_TS_Renderer    renderer,
                   FT_TS_UInt        num_params,
                   FT_TS_Parameter*  parameters )
  {
    FT_TS_ListNode  node;
    FT_TS_Error     error = FT_TS_Err_Ok;

    FT_TS_Renderer_SetModeFunc  set_mode;


    if ( !library )
    {
      error = FT_TS_THROW( Invalid_Library_Handle );
      goto Exit;
    }

    if ( !renderer )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    if ( num_params > 0 && !parameters )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    node = FT_TS_List_Find( &library->renderers, renderer );
    if ( !node )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TS_List_Up( &library->renderers, node );

    if ( renderer->glyph_format == FT_TS_GLYPH_FORMAT_OUTLINE )
      library->cur_renderer = renderer;

    set_mode = renderer->clazz->set_mode;

    for ( ; num_params > 0; num_params-- )
    {
      error = set_mode( renderer, parameters->tag, parameters->data );
      if ( error )
        break;
      parameters++;
    }

  Exit:
    return error;
  }


  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Render_Glyph_Internal( FT_TS_Library      library,
                            FT_TS_GlyphSlot    slot,
                            FT_TS_Render_Mode  render_mode )
  {
    FT_TS_Error     error = FT_TS_Err_Ok;
    FT_TS_Face      face  = slot->face;
    FT_TS_Renderer  renderer;


    switch ( slot->format )
    {
    default:
      if ( slot->internal->load_flags & FT_TS_LOAD_COLOR )
      {
        FT_TS_LayerIterator  iterator;

        FT_TS_UInt  base_glyph = slot->glyph_index;

        FT_TS_Bool  have_layers;
        FT_TS_UInt  glyph_index;
        FT_TS_UInt  color_index;


        /* check whether we have colored glyph layers */
        iterator.p  = NULL;
        have_layers = FT_TS_Get_Color_Glyph_Layer( face,
                                                base_glyph,
                                                &glyph_index,
                                                &color_index,
                                                &iterator );
        if ( have_layers )
        {
          error = FT_TS_New_GlyphSlot( face, NULL );
          if ( !error )
          {
            TT_Face       ttface = (TT_Face)face;
            SFNT_Service  sfnt   = (SFNT_Service)ttface->sfnt;


            do
            {
              FT_TS_Int32  load_flags = slot->internal->load_flags;


              /* disable the `FT_TS_LOAD_COLOR' flag to avoid recursion */
              /* right here in this function                         */
              load_flags &= ~FT_TS_LOAD_COLOR;

              /* render into the new `face->glyph' glyph slot */
              load_flags |= FT_TS_LOAD_RENDER;

              error = FT_TS_Load_Glyph( face, glyph_index, load_flags );
              if ( error )
                break;

              /* blend new `face->glyph' into old `slot'; */
              /* at the first call, `slot' is still empty */
              error = sfnt->colr_blend( ttface,
                                        color_index,
                                        slot,
                                        face->glyph );
              if ( error )
                break;

            } while ( FT_TS_Get_Color_Glyph_Layer( face,
                                                base_glyph,
                                                &glyph_index,
                                                &color_index,
                                                &iterator ) );

            if ( !error )
              slot->format = FT_TS_GLYPH_FORMAT_BITMAP;

            /* this call also restores `slot' as the glyph slot */
            FT_TS_Done_GlyphSlot( face->glyph );
          }

          if ( !error )
            return error;

          /* Failed to do the colored layer.  Draw outline instead. */
          slot->format = FT_TS_GLYPH_FORMAT_OUTLINE;
        }
      }

      {
        FT_TS_ListNode  node = NULL;


        /* small shortcut for the very common case */
        if ( slot->format == FT_TS_GLYPH_FORMAT_OUTLINE )
        {
          renderer = library->cur_renderer;
          node     = library->renderers.head;
        }
        else
          renderer = FT_TS_Lookup_Renderer( library, slot->format, &node );

        error = FT_TS_ERR( Cannot_Render_Glyph );
        while ( renderer )
        {
          error = renderer->render( renderer, slot, render_mode, NULL );
          if ( !error                                   ||
               FT_TS_ERR_NEQ( error, Cannot_Render_Glyph ) )
            break;

          /* FT_TS_Err_Cannot_Render_Glyph is returned if the render mode   */
          /* is unsupported by the current renderer for this glyph image */
          /* format.                                                     */

          /* now, look for another renderer that supports the same */
          /* format.                                               */
          renderer = FT_TS_Lookup_Renderer( library, slot->format, &node );
        }

        /* it is not an error if we cannot render a bitmap glyph */
        if ( FT_TS_ERR_EQ( error, Cannot_Render_Glyph ) &&
             slot->format == FT_TS_GLYPH_FORMAT_BITMAP  )
          error = FT_TS_Err_Ok;
      }
    }

#ifdef FT_TS_DEBUG_LEVEL_TRACE

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  checksum

    /*
     * Computing the MD5 checksum is expensive, unnecessarily distorting a
     * possible profiling of FreeType if compiled with tracing support.  For
     * this reason, we execute the following code only if explicitly
     * requested.
     */

    /* we use FT_TS_TRACE3 in this block */
    if ( !error                               &&
         ft_trace_levels[trace_checksum] >= 3 &&
         slot->bitmap.buffer                  )
    {
      FT_TS_Bitmap  bitmap;
      FT_TS_Error   err;


      FT_TS_Bitmap_Init( &bitmap );

      /* we convert to a single bitmap format for computing the checksum */
      /* this also converts the bitmap flow to `down' (i.e., pitch > 0)  */
      err = FT_TS_Bitmap_Convert( library, &slot->bitmap, &bitmap, 1 );
      if ( !err )
      {
        MD5_CTX        ctx;
        unsigned char  md5[16];
        unsigned long  coverage = 0;
        int            i, j;
        int            rows  = (int)bitmap.rows;
        int            pitch = bitmap.pitch;


        FT_TS_TRACE3(( "FT_TS_Render_Glyph: bitmap %dx%d, %s (mode %d)\n",
                    pitch,
                    rows,
                    pixel_modes[slot->bitmap.pixel_mode],
                    slot->bitmap.pixel_mode ));

        for ( i = 0; i < rows; i++ )
          for ( j = 0; j < pitch; j++ )
            coverage += bitmap.buffer[i * pitch + j];

        FT_TS_TRACE3(( "  Total coverage: %lu\n", coverage ));

        MD5_Init( &ctx );
        if ( bitmap.buffer )
          MD5_Update( &ctx, bitmap.buffer,
                      (unsigned long)rows * (unsigned long)pitch );
        MD5_Final( md5, &ctx );

        FT_TS_TRACE3(( "  MD5 checksum: " ));
        for ( i = 0; i < 16; i++ )
          FT_TS_TRACE3(( "%02X", md5[i] ));
        FT_TS_TRACE3(( "\n" ));
      }

      FT_TS_Bitmap_Done( library, &bitmap );
    }

    /*
     * Dump bitmap in Netpbm format (PBM or PGM).
     */

    /* we use FT_TS_TRACE7 in this block */
    if ( !error                               &&
         ft_trace_levels[trace_checksum] >= 7 &&
         slot->bitmap.buffer                  )
    {
      if ( slot->bitmap.rows  < 128U &&
           slot->bitmap.width < 128U )
      {
        int  rows  = (int)slot->bitmap.rows;
        int  width = (int)slot->bitmap.width;
        int  pitch =      slot->bitmap.pitch;
        int  i, j, m;

        unsigned char*  topleft = slot->bitmap.buffer;


        if ( pitch < 0 )
          topleft -= pitch * ( rows - 1 );

        FT_TS_TRACE7(( "Netpbm image: start\n" ));
        switch ( slot->bitmap.pixel_mode )
        {
        case FT_TS_PIXEL_MODE_MONO:
          FT_TS_TRACE7(( "P1 %d %d\n", width, rows ));
          for ( i = 0; i < rows; i++ )
          {
            for ( j = 0; j < width; )
              for ( m = 128; m > 0 && j < width; m >>= 1, j++ )
                FT_TS_TRACE7(( " %d",
                            ( topleft[i * pitch + j / 8] & m ) != 0 ));
            FT_TS_TRACE7(( "\n" ));
          }
          break;

        default:
          FT_TS_TRACE7(( "P2 %d %d 255\n", width, rows ));
          for ( i = 0; i < rows; i++ )
          {
            for ( j = 0; j < width; j += 1 )
              FT_TS_TRACE7(( " %3u", topleft[i * pitch + j] ));
            FT_TS_TRACE7(( "\n" ));
          }
        }
        FT_TS_TRACE7(( "Netpbm image: end\n" ));
      }
      else
        FT_TS_TRACE7(( "Netpbm image: too large, omitted\n" ));
    }

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  objs

#endif /* FT_TS_DEBUG_LEVEL_TRACE */

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Render_Glyph( FT_TS_GlyphSlot    slot,
                   FT_TS_Render_Mode  render_mode )
  {
    FT_TS_Library  library;


    if ( !slot || !slot->face )
      return FT_TS_THROW( Invalid_Argument );

    library = FT_TS_FACE_LIBRARY( slot->face );

    return FT_TS_Render_Glyph_Internal( library, slot, render_mode );
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         M O D U L E S                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /**************************************************************************
   *
   * @Function:
   *   Destroy_Module
   *
   * @Description:
   *   Destroys a given module object.  For drivers, this also destroys
   *   all child faces.
   *
   * @InOut:
   *   module ::
   *     A handle to the target driver object.
   *
   * @Note:
   *   The driver _must_ be LOCKED!
   */
  static void
  Destroy_Module( FT_TS_Module  module )
  {
    FT_TS_Memory         memory  = module->memory;
    FT_TS_Module_Class*  clazz   = module->clazz;
    FT_TS_Library        library = module->library;


    if ( library && library->auto_hinter == module )
      library->auto_hinter = NULL;

    /* if the module is a renderer */
    if ( FT_TS_MODULE_IS_RENDERER( module ) )
      ft_remove_renderer( module );

    /* if the module is a font driver, add some steps */
    if ( FT_TS_MODULE_IS_DRIVER( module ) )
      Destroy_Driver( FT_TS_DRIVER( module ) );

    /* finalize the module object */
    if ( clazz->module_done )
      clazz->module_done( module );

    /* discard it */
    FT_TS_FREE( module );
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Add_Module( FT_TS_Library              library,
                 const FT_TS_Module_Class*  clazz )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory;
    FT_TS_Module  module = NULL;
    FT_TS_UInt    nn;


#define FREETYPE_VER_FIXED  ( ( (FT_TS_Long)FREETYPE_MAJOR << 16 ) | \
                                FREETYPE_MINOR                  )

    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !clazz )
      return FT_TS_THROW( Invalid_Argument );

    /* check FreeType version */
    if ( clazz->module_requires > FREETYPE_VER_FIXED )
      return FT_TS_THROW( Invalid_Version );

    /* look for a module with the same name in the library's table */
    for ( nn = 0; nn < library->num_modules; nn++ )
    {
      module = library->modules[nn];
      if ( ft_strcmp( module->clazz->module_name, clazz->module_name ) == 0 )
      {
        /* this installed module has the same name, compare their versions */
        if ( clazz->module_version <= module->clazz->module_version )
          return FT_TS_THROW( Lower_Module_Version );

        /* remove the module from our list, then exit the loop to replace */
        /* it by our new version..                                        */
        FT_TS_Remove_Module( library, module );
        break;
      }
    }

    memory = library->memory;
    error  = FT_TS_Err_Ok;

    if ( library->num_modules >= FT_TS_MAX_MODULES )
    {
      error = FT_TS_THROW( Too_Many_Drivers );
      goto Exit;
    }

    /* allocate module object */
    if ( FT_TS_ALLOC( module, clazz->module_size ) )
      goto Exit;

    /* base initialization */
    module->library = library;
    module->memory  = memory;
    module->clazz   = (FT_TS_Module_Class*)clazz;

    /* check whether the module is a renderer - this must be performed */
    /* before the normal module initialization                         */
    if ( FT_TS_MODULE_IS_RENDERER( module ) )
    {
      /* add to the renderers list */
      error = ft_add_renderer( module );
      if ( error )
        goto Fail;
    }

    /* is the module a auto-hinter? */
    if ( FT_TS_MODULE_IS_HINTER( module ) )
      library->auto_hinter = module;

    /* if the module is a font driver */
    if ( FT_TS_MODULE_IS_DRIVER( module ) )
    {
      FT_TS_Driver  driver = FT_TS_DRIVER( module );


      driver->clazz = (FT_TS_Driver_Class)module->clazz;
    }

    if ( clazz->module_init )
    {
      error = clazz->module_init( module );
      if ( error )
        goto Fail;
    }

    /* add module to the library's table */
    library->modules[library->num_modules++] = module;

  Exit:
    return error;

  Fail:
    if ( FT_TS_MODULE_IS_RENDERER( module ) )
    {
      FT_TS_Renderer  renderer = FT_TS_RENDERER( module );


      if ( renderer->clazz                                          &&
           renderer->clazz->glyph_format == FT_TS_GLYPH_FORMAT_OUTLINE &&
           renderer->raster                                         )
        renderer->clazz->raster_class->raster_done( renderer->raster );
    }

    FT_TS_FREE( module );
    goto Exit;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Module )
  FT_TS_Get_Module( FT_TS_Library   library,
                 const char*  module_name )
  {
    FT_TS_Module   result = NULL;
    FT_TS_Module*  cur;
    FT_TS_Module*  limit;


    if ( !library || !module_name )
      return result;

    cur   = library->modules;
    limit = cur + library->num_modules;

    for ( ; cur < limit; cur++ )
      if ( ft_strcmp( cur[0]->clazz->module_name, module_name ) == 0 )
      {
        result = cur[0];
        break;
      }

    return result;
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( const void* )
  FT_TS_Get_Module_Interface( FT_TS_Library   library,
                           const char*  mod_name )
  {
    FT_TS_Module  module;


    /* test for valid `library' delayed to FT_TS_Get_Module() */

    module = FT_TS_Get_Module( library, mod_name );

    return module ? module->clazz->module_interface : 0;
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_module_get_service( FT_TS_Module    module,
                         const char*  service_id,
                         FT_TS_Bool      global )
  {
    FT_TS_Pointer  result = NULL;


    if ( module )
    {
      FT_TS_ASSERT( module->clazz && module->clazz->get_interface );

      /* first, look for the service in the module */
      if ( module->clazz->get_interface )
        result = module->clazz->get_interface( module, service_id );

      if ( global && !result )
      {
        /* we didn't find it, look in all other modules then */
        FT_TS_Library  library = module->library;
        FT_TS_Module*  cur     = library->modules;
        FT_TS_Module*  limit   = cur + library->num_modules;


        for ( ; cur < limit; cur++ )
        {
          if ( cur[0] != module )
          {
            FT_TS_ASSERT( cur[0]->clazz );

            if ( cur[0]->clazz->get_interface )
            {
              result = cur[0]->clazz->get_interface( cur[0], service_id );
              if ( result )
                break;
            }
          }
        }
      }
    }

    return result;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Remove_Module( FT_TS_Library  library,
                    FT_TS_Module   module )
  {
    /* try to find the module from the table, then remove it from there */

    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( module )
    {
      FT_TS_Module*  cur   = library->modules;
      FT_TS_Module*  limit = cur + library->num_modules;


      for ( ; cur < limit; cur++ )
      {
        if ( cur[0] == module )
        {
          /* remove it from the table */
          library->num_modules--;
          limit--;
          while ( cur < limit )
          {
            cur[0] = cur[1];
            cur++;
          }
          limit[0] = NULL;

          /* destroy the module */
          Destroy_Module( module );

          return FT_TS_Err_Ok;
        }
      }
    }
    return FT_TS_THROW( Invalid_Driver_Handle );
  }


  static FT_TS_Error
  ft_property_do( FT_TS_Library        library,
                  const FT_TS_String*  module_name,
                  const FT_TS_String*  property_name,
                  void*             value,
                  FT_TS_Bool           set,
                  FT_TS_Bool           value_is_string )
  {
    FT_TS_Module*           cur;
    FT_TS_Module*           limit;
    FT_TS_Module_Interface  interface;

    FT_TS_Service_Properties  service;

#ifdef FT_TS_DEBUG_LEVEL_ERROR
    const FT_TS_String*  set_name  = "FT_TS_Property_Set";
    const FT_TS_String*  get_name  = "FT_TS_Property_Get";
    const FT_TS_String*  func_name = set ? set_name : get_name;
#endif

    FT_TS_Bool  missing_func;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    if ( !module_name || !property_name || !value )
      return FT_TS_THROW( Invalid_Argument );

    cur   = library->modules;
    limit = cur + library->num_modules;

    /* search module */
    for ( ; cur < limit; cur++ )
      if ( !ft_strcmp( cur[0]->clazz->module_name, module_name ) )
        break;

    if ( cur == limit )
    {
      FT_TS_TRACE2(( "%s: can't find module `%s'\n",
                  func_name, module_name ));
      return FT_TS_THROW( Missing_Module );
    }

    /* check whether we have a service interface */
    if ( !cur[0]->clazz->get_interface )
    {
      FT_TS_TRACE2(( "%s: module `%s' doesn't support properties\n",
                  func_name, module_name ));
      return FT_TS_THROW( Unimplemented_Feature );
    }

    /* search property service */
    interface = cur[0]->clazz->get_interface( cur[0],
                                              FT_TS_SERVICE_ID_PROPERTIES );
    if ( !interface )
    {
      FT_TS_TRACE2(( "%s: module `%s' doesn't support properties\n",
                  func_name, module_name ));
      return FT_TS_THROW( Unimplemented_Feature );
    }

    service = (FT_TS_Service_Properties)interface;

    if ( set )
      missing_func = FT_TS_BOOL( !service->set_property );
    else
      missing_func = FT_TS_BOOL( !service->get_property );

    if ( missing_func )
    {
      FT_TS_TRACE2(( "%s: property service of module `%s' is broken\n",
                  func_name, module_name ));
      return FT_TS_THROW( Unimplemented_Feature );
    }

    return set ? service->set_property( cur[0],
                                        property_name,
                                        value,
                                        value_is_string )
               : service->get_property( cur[0],
                                        property_name,
                                        value );
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Property_Set( FT_TS_Library        library,
                   const FT_TS_String*  module_name,
                   const FT_TS_String*  property_name,
                   const void*       value )
  {
    return ft_property_do( library,
                           module_name,
                           property_name,
                           (void*)value,
                           TRUE,
                           FALSE );
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Property_Get( FT_TS_Library        library,
                   const FT_TS_String*  module_name,
                   const FT_TS_String*  property_name,
                   void*             value )
  {
    return ft_property_do( library,
                           module_name,
                           property_name,
                           value,
                           FALSE,
                           FALSE );
  }


#ifdef FT_TS_CONFIG_OPTION_ENVIRONMENT_PROPERTIES

  /* this variant is used for handling the FREETYPE_PROPERTIES */
  /* environment variable                                      */

  FT_TS_BASE_DEF( FT_TS_Error )
  ft_property_string_set( FT_TS_Library        library,
                          const FT_TS_String*  module_name,
                          const FT_TS_String*  property_name,
                          FT_TS_String*        value )
  {
    return ft_property_do( library,
                           module_name,
                           property_name,
                           (void*)value,
                           TRUE,
                           TRUE );
  }

#endif


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                         L I B R A R Y                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Reference_Library( FT_TS_Library  library )
  {
    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    library->refcount++;

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_New_Library( FT_TS_Memory    memory,
                  FT_TS_Library  *alibrary )
  {
    FT_TS_Library  library = NULL;
    FT_TS_Error    error;


    if ( !memory || !alibrary )
      return FT_TS_THROW( Invalid_Argument );

#ifndef FT_TS_DEBUG_LOGGING
#ifdef FT_TS_DEBUG_LEVEL_ERROR
    /* init debugging support */
    ft_debug_init();
#endif /* FT_TS_DEBUG_LEVEL_ERROR */
#endif /* !FT_TS_DEBUG_LOGGING */

    /* first of all, allocate the library object */
    if ( FT_TS_NEW( library ) )
      return error;

    library->memory = memory;

    library->version_major = FREETYPE_MAJOR;
    library->version_minor = FREETYPE_MINOR;
    library->version_patch = FREETYPE_PATCH;

    library->refcount = 1;

    /* That's ok now */
    *alibrary = library;

    return FT_TS_Err_Ok;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Library_Version( FT_TS_Library   library,
                      FT_TS_Int      *amajor,
                      FT_TS_Int      *aminor,
                      FT_TS_Int      *apatch )
  {
    FT_TS_Int  major = 0;
    FT_TS_Int  minor = 0;
    FT_TS_Int  patch = 0;


    if ( library )
    {
      major = library->version_major;
      minor = library->version_minor;
      patch = library->version_patch;
    }

    if ( amajor )
      *amajor = major;

    if ( aminor )
      *aminor = minor;

    if ( apatch )
      *apatch = patch;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Done_Library( FT_TS_Library  library )
  {
    FT_TS_Memory  memory;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    library->refcount--;
    if ( library->refcount > 0 )
      goto Exit;

    memory = library->memory;

    /*
     * Close all faces in the library.  If we don't do this, we can have
     * some subtle memory leaks.
     *
     * Example:
     *
     * - the cff font driver uses the pshinter module in cff_size_done
     * - if the pshinter module is destroyed before the cff font driver,
     *   opened FT_TS_Face objects managed by the driver are not properly
     *   destroyed, resulting in a memory leak
     *
     * Some faces are dependent on other faces, like Type42 faces that
     * depend on TrueType faces synthesized internally.
     *
     * The order of drivers should be specified in driver_name[].
     */
    {
      FT_TS_UInt      m, n;
      const char*  driver_name[] = { "type42", NULL };


      for ( m = 0;
            m < sizeof ( driver_name ) / sizeof ( driver_name[0] );
            m++ )
      {
        for ( n = 0; n < library->num_modules; n++ )
        {
          FT_TS_Module    module      = library->modules[n];
          const char*  module_name = module->clazz->module_name;
          FT_TS_List      faces;


          if ( driver_name[m]                                &&
               ft_strcmp( module_name, driver_name[m] ) != 0 )
            continue;

          if ( ( module->clazz->module_flags & FT_TS_MODULE_FONT_DRIVER ) == 0 )
            continue;

          FT_TS_TRACE7(( "FT_TS_Done_Library: close faces for %s\n", module_name ));

          faces = &FT_TS_DRIVER( module )->faces_list;
          while ( faces->head )
          {
            FT_TS_Done_Face( FT_TS_FACE( faces->head->data ) );
            if ( faces->head )
              FT_TS_TRACE0(( "FT_TS_Done_Library: failed to free some faces\n" ));
          }
        }
      }
    }

    /* Close all other modules in the library */
#if 1
    /* XXX Modules are removed in the reversed order so that  */
    /* type42 module is removed before truetype module.  This */
    /* avoids double free in some occasions.  It is a hack.   */
    while ( library->num_modules > 0 )
      FT_TS_Remove_Module( library,
                        library->modules[library->num_modules - 1] );
#else
    {
      FT_TS_UInt  n;


      for ( n = 0; n < library->num_modules; n++ )
      {
        FT_TS_Module  module = library->modules[n];


        if ( module )
        {
          Destroy_Module( module );
          library->modules[n] = NULL;
        }
      }
    }
#endif

    FT_TS_FREE( library );

  Exit:
    return FT_TS_Err_Ok;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_Set_Debug_Hook( FT_TS_Library         library,
                     FT_TS_UInt            hook_index,
                     FT_TS_DebugHook_Func  debug_hook )
  {
    if ( library && debug_hook &&
         hook_index <
           ( sizeof ( library->debug_hooks ) / sizeof ( void* ) ) )
      library->debug_hooks[hook_index] = debug_hook;
  }


  /* documentation is in ftmodapi.h */

  FT_TS_EXPORT_DEF( FT_TS_TrueTypeEngineType )
  FT_TS_Get_TrueType_Engine_Type( FT_TS_Library  library )
  {
    FT_TS_TrueTypeEngineType  result = FT_TS_TRUETYPE_ENGINE_TYPE_NONE;


    if ( library )
    {
      FT_TS_Module  module = FT_TS_Get_Module( library, "truetype" );


      if ( module )
      {
        FT_TS_Service_TrueTypeEngine  service;


        service = (FT_TS_Service_TrueTypeEngine)
                    ft_module_get_service( module,
                                           FT_TS_SERVICE_ID_TRUETYPE_ENGINE,
                                           0 );
        if ( service )
          result = service->engine_type;
      }
    }

    return result;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_SubGlyph_Info( FT_TS_GlyphSlot  glyph,
                        FT_TS_UInt       sub_index,
                        FT_TS_Int       *p_index,
                        FT_TS_UInt      *p_flags,
                        FT_TS_Int       *p_arg1,
                        FT_TS_Int       *p_arg2,
                        FT_TS_Matrix    *p_transform )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );


    if ( glyph                                      &&
         glyph->subglyphs                           &&
         glyph->format == FT_TS_GLYPH_FORMAT_COMPOSITE &&
         sub_index < glyph->num_subglyphs           )
    {
      FT_TS_SubGlyph  subg = glyph->subglyphs + sub_index;


      *p_index     = subg->index;
      *p_flags     = subg->flags;
      *p_arg1      = subg->arg1;
      *p_arg2      = subg->arg2;
      *p_transform = subg->transform;

      error = FT_TS_Err_Ok;
    }

    return error;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Color_Glyph_Layer( FT_TS_Face            face,
                            FT_TS_UInt            base_glyph,
                            FT_TS_UInt           *aglyph_index,
                            FT_TS_UInt           *acolor_index,
                            FT_TS_LayerIterator*  iterator )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face                                   ||
         !aglyph_index                           ||
         !acolor_index                           ||
         !iterator                               ||
         base_glyph >= (FT_TS_UInt)face->num_glyphs )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_colr_layer )
      return sfnt->get_colr_layer( ttface,
                                   base_glyph,
                                   aglyph_index,
                                   acolor_index,
                                   iterator );
    else
      return 0;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Color_Glyph_Paint( FT_TS_Face                  face,
                            FT_TS_UInt                  base_glyph,
                            FT_TS_Color_Root_Transform  root_transform,
                            FT_TS_OpaquePaint*          paint )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face || !paint )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_colr_layer )
      return sfnt->get_colr_glyph_paint( ttface,
                                         base_glyph,
                                         root_transform,
                                         paint );
    else
      return 0;
  }


  /* documentation is in ftcolor.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Color_Glyph_ClipBox( FT_TS_Face      face,
                              FT_TS_UInt      base_glyph,
                              FT_TS_ClipBox*  clip_box )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face || !clip_box )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_color_glyph_clipbox )
      return sfnt->get_color_glyph_clipbox( ttface,
                                            base_glyph,
                                            clip_box );
    else
      return 0;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Paint_Layers( FT_TS_Face            face,
                       FT_TS_LayerIterator*  layer_iterator,
                       FT_TS_OpaquePaint*    paint )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face || !paint || !layer_iterator )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_paint_layers )
      return sfnt->get_paint_layers( ttface, layer_iterator, paint );
    else
      return 0;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Paint( FT_TS_Face face,
                FT_TS_OpaquePaint  opaque_paint,
                FT_TS_COLR_Paint*  paint )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face || !paint )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_paint )
      return sfnt->get_paint( ttface, opaque_paint, paint );
    else
      return 0;
  }


  /* documentation is in freetype.h */

  FT_TS_EXPORT_DEF( FT_TS_Bool )
  FT_TS_Get_Colorline_Stops ( FT_TS_Face                face,
                           FT_TS_ColorStop *         color_stop,
                           FT_TS_ColorStopIterator  *iterator )
  {
    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face || !color_stop || !iterator )
      return 0;

    if ( !FT_TS_IS_SFNT( face ) )
      return 0;

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    if ( sfnt->get_colorline_stops )
      return sfnt->get_colorline_stops ( ttface, color_stop, iterator );
    else
      return 0;
  }


/* END */
