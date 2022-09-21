/****************************************************************************
 *
 * fttype1.c
 *
 *   FreeType utility file for PS names support (body).
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


#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftserv.h>
#include <freetype/internal/services/svpsinfo.h>


  /* documentation is in t1tables.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_PS_Font_Info( FT_TS_Face          face,
                       PS_FontInfoRec*  afont_info )
  {
    FT_TS_Error           error;
    FT_TS_Service_PsInfo  service;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !afont_info )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_FACE_FIND_SERVICE( face, service, POSTSCRIPT_INFO );

    if ( service && service->ps_get_font_info )
      error = service->ps_get_font_info( face, afont_info );
    else
      error = FT_TS_THROW( Invalid_Argument );

    return error;
  }


  /* documentation is in t1tables.h */

  FT_TS_EXPORT_DEF( FT_TS_Int )
  FT_TS_Has_PS_Glyph_Names( FT_TS_Face  face )
  {
    FT_TS_Int             result = 0;
    FT_TS_Service_PsInfo  service;


    if ( face )
    {
      FT_TS_FACE_FIND_SERVICE( face, service, POSTSCRIPT_INFO );

      if ( service && service->ps_has_glyph_names )
        result = service->ps_has_glyph_names( face );
    }

    return result;
  }


  /* documentation is in t1tables.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_PS_Font_Private( FT_TS_Face         face,
                          PS_PrivateRec*  afont_private )
  {
    FT_TS_Error           error;
    FT_TS_Service_PsInfo  service;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !afont_private )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_FACE_FIND_SERVICE( face, service, POSTSCRIPT_INFO );

    if ( service && service->ps_get_font_private )
      error = service->ps_get_font_private( face, afont_private );
    else
      error = FT_TS_THROW( Invalid_Argument );

    return error;
  }


  /* documentation is in t1tables.h */

  FT_TS_EXPORT_DEF( FT_TS_Long )
  FT_TS_Get_PS_Font_Value( FT_TS_Face       face,
                        PS_Dict_Keys  key,
                        FT_TS_UInt       idx,
                        void         *value,
                        FT_TS_Long       value_len )
  {
    FT_TS_Int             result  = 0;
    FT_TS_Service_PsInfo  service = NULL;


    if ( face )
    {
      FT_TS_FACE_FIND_SERVICE( face, service, POSTSCRIPT_INFO );

      if ( service && service->ps_get_font_value )
        result = service->ps_get_font_value( face, key, idx,
                                             value, value_len );
    }

    return result;
  }


/* END */
