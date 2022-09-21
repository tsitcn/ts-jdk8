/****************************************************************************
 *
 * ftcolor.c
 *
 *   FreeType's glyph color management (body).
 *
 * Copyright (C) 2018-2022 by
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
#include <freetype/internal/sfnt.h>
#include <freetype/internal/tttypes.h>
#include <freetype/ftcolor.h>


#ifdef TT_CONFIG_OPTION_COLOR_LAYERS

  static
  const FT_TS_Palette_Data  null_palette_data = { 0, NULL, NULL, 0, NULL };


  /* documentation is in ftcolor.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Data_Get( FT_TS_Face           face,
                       FT_TS_Palette_Data  *apalette_data )
  {
    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );
    if ( !apalette_data)
      return FT_TS_THROW( Invalid_Argument );

    if ( FT_TS_IS_SFNT( face ) )
      *apalette_data = ( (TT_Face)face )->palette_data;
    else
      *apalette_data = null_palette_data;

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Select( FT_TS_Face     face,
                     FT_TS_UShort   palette_index,
                     FT_TS_Color*  *apalette )
  {
    FT_TS_Error  error;

    TT_Face       ttface;
    SFNT_Service  sfnt;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !FT_TS_IS_SFNT( face ) )
    {
      if ( apalette )
        *apalette = NULL;

      return FT_TS_Err_Ok;
    }

    ttface = (TT_Face)face;
    sfnt   = (SFNT_Service)ttface->sfnt;

    error = sfnt->set_palette( ttface, palette_index );
    if ( error )
      return error;

    ttface->palette_index = palette_index;

    if ( apalette )
      *apalette = ttface->palette;

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftcolor.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Set_Foreground_Color( FT_TS_Face   face,
                                   FT_TS_Color  foreground_color )
  {
    TT_Face  ttface;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !FT_TS_IS_SFNT( face ) )
      return FT_TS_Err_Ok;

    ttface = (TT_Face)face;

    ttface->foreground_color      = foreground_color;
    ttface->have_foreground_color = 1;

    return FT_TS_Err_Ok;
  }

#else /* !TT_CONFIG_OPTION_COLOR_LAYERS */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Data_Get( FT_TS_Face           face,
                       FT_TS_Palette_Data  *apalette_data )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( apalette_data );


    return FT_TS_THROW( Unimplemented_Feature );
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Select( FT_TS_Face     face,
                     FT_TS_UShort   palette_index,
                     FT_TS_Color*  *apalette )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( palette_index );
    FT_TS_UNUSED( apalette );


    return FT_TS_THROW( Unimplemented_Feature );
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Palette_Set_Foreground_Color( FT_TS_Face   face,
                                   FT_TS_Color  foreground_color )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( foreground_color );


    return FT_TS_THROW( Unimplemented_Feature );
  }

#endif /* !TT_CONFIG_OPTION_COLOR_LAYERS */


/* END */
