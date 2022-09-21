/****************************************************************************
 *
 * ftsnames.c
 *
 *   Simple interface to access SFNT name tables (which are used
 *   to hold font names, copyright info, notices, etc.) (body).
 *
 *   This is _not_ used to retrieve glyph names!
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


#include <freetype/internal/ftdebug.h>

#include <freetype/ftsnames.h>
#include <freetype/internal/tttypes.h>
#include <freetype/internal/ftstream.h>


#ifdef TT_CONFIG_OPTION_SFNT_NAMES


  /* documentation is in ftsnames.h */

  FT_TS_EXPORT_DEF( FT_TS_UInt )
  FT_TS_Get_Sfnt_Name_Count( FT_TS_Face  face )
  {
    return ( face && FT_TS_IS_SFNT( face ) ) ? ((TT_Face)face)->num_names : 0;
  }


  /* documentation is in ftsnames.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Sfnt_Name( FT_TS_Face       face,
                    FT_TS_UInt       idx,
                    FT_TS_SfntName  *aname )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );


    if ( aname && face && FT_TS_IS_SFNT( face ) )
    {
      TT_Face  ttface = (TT_Face)face;


      if ( idx < (FT_TS_UInt)ttface->num_names )
      {
        TT_Name  entry = ttface->name_table.names + idx;


        /* load name on demand */
        if ( entry->stringLength > 0 && !entry->string )
        {
          FT_TS_Memory  memory = face->memory;
          FT_TS_Stream  stream = face->stream;


          if ( FT_TS_QNEW_ARRAY ( entry->string, entry->stringLength ) ||
               FT_TS_STREAM_SEEK( entry->stringOffset )                ||
               FT_TS_STREAM_READ( entry->string, entry->stringLength ) )
          {
            FT_TS_FREE( entry->string );
            entry->stringLength = 0;
          }
        }

        aname->platform_id = entry->platformID;
        aname->encoding_id = entry->encodingID;
        aname->language_id = entry->languageID;
        aname->name_id     = entry->nameID;
        aname->string      = (FT_TS_Byte*)entry->string;
        aname->string_len  = entry->stringLength;

        error = FT_TS_Err_Ok;
      }
    }

    return error;
  }


  /* documentation is in ftsnames.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Sfnt_LangTag( FT_TS_Face          face,
                       FT_TS_UInt          langID,
                       FT_TS_SfntLangTag  *alangTag )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );


    if ( alangTag && face && FT_TS_IS_SFNT( face ) )
    {
      TT_Face  ttface = (TT_Face)face;


      if ( ttface->name_table.format != 1 )
        return FT_TS_THROW( Invalid_Table );

      if ( langID > 0x8000U                                        &&
           langID - 0x8000U < ttface->name_table.numLangTagRecords )
      {
        TT_LangTag  entry = ttface->name_table.langTags +
                            ( langID - 0x8000U );


        /* load name on demand */
        if ( entry->stringLength > 0 && !entry->string )
        {
          FT_TS_Memory  memory = face->memory;
          FT_TS_Stream  stream = face->stream;


          if ( FT_TS_QNEW_ARRAY ( entry->string, entry->stringLength ) ||
               FT_TS_STREAM_SEEK( entry->stringOffset )                ||
               FT_TS_STREAM_READ( entry->string, entry->stringLength ) )
          {
            FT_TS_FREE( entry->string );
            entry->stringLength = 0;
          }
        }

        alangTag->string     = (FT_TS_Byte*)entry->string;
        alangTag->string_len = entry->stringLength;

        error = FT_TS_Err_Ok;
      }
    }

    return error;
  }


#else /* !TT_CONFIG_OPTION_SFNT_NAMES */


  FT_TS_EXPORT_DEF( FT_TS_UInt )
  FT_TS_Get_Sfnt_Name_Count( FT_TS_Face  face )
  {
    FT_TS_UNUSED( face );

    return 0;
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Sfnt_Name( FT_TS_Face       face,
                    FT_TS_UInt       idx,
                    FT_TS_SfntName  *aname )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( idx );
    FT_TS_UNUSED( aname );

    return FT_TS_THROW( Unimplemented_Feature );
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Sfnt_LangTag( FT_TS_Face          face,
                       FT_TS_UInt          langID,
                       FT_TS_SfntLangTag  *alangTag )
  {
    FT_TS_UNUSED( face );
    FT_TS_UNUSED( langID );
    FT_TS_UNUSED( alangTag );

    return FT_TS_THROW( Unimplemented_Feature );
  }


#endif /* !TT_CONFIG_OPTION_SFNT_NAMES */


/* END */
