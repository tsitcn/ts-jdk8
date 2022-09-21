/****************************************************************************
 *
 * ftgxval.c
 *
 *   FreeType API for validating TrueTypeGX/AAT tables (body).
 *
 * Copyright (C) 2004-2022 by
 * Masatake YAMATO, Redhat K.K,
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/****************************************************************************
 *
 * gxvalid is derived from both gxlayout module and otvalid module.
 * Development of gxlayout is supported by the Information-technology
 * Promotion Agency(IPA), Japan.
 *
 */


#include <freetype/internal/ftdebug.h>

#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svgxval.h>


  /* documentation is in ftgxval.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_TrueTypeGX_Validate( FT_TS_Face   face,
                          FT_TS_UInt   validation_flags,
                          FT_TS_Bytes  tables[FT_TS_VALIDATE_GX_LENGTH],
                          FT_TS_UInt   table_length )
  {
    FT_TS_Service_GXvalidate  service;
    FT_TS_Error               error;


    if ( !face )
    {
      error = FT_TS_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( !tables )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TS_FACE_FIND_GLOBAL_SERVICE( face, service, GX_VALIDATE );

    if ( service )
      error = service->validate( face,
                                 validation_flags,
                                 tables,
                                 table_length );
    else
      error = FT_TS_THROW( Unimplemented_Feature );

  Exit:
    return error;
  }


  FT_TS_EXPORT_DEF( void )
  FT_TS_TrueTypeGX_Free( FT_TS_Face   face,
                      FT_TS_Bytes  table )
  {
    FT_TS_Memory  memory;


    if ( !face )
      return;

    memory = FT_TS_FACE_MEMORY( face );

    FT_TS_FREE( table );
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_ClassicKern_Validate( FT_TS_Face    face,
                           FT_TS_UInt    validation_flags,
                           FT_TS_Bytes  *ckern_table )
  {
    FT_TS_Service_CKERNvalidate  service;
    FT_TS_Error                  error;


    if ( !face )
    {
      error = FT_TS_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( !ckern_table )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TS_FACE_FIND_GLOBAL_SERVICE( face, service, CLASSICKERN_VALIDATE );

    if ( service )
      error = service->validate( face,
                                 validation_flags,
                                 ckern_table );
    else
      error = FT_TS_THROW( Unimplemented_Feature );

  Exit:
    return error;
  }


  FT_TS_EXPORT_DEF( void )
  FT_TS_ClassicKern_Free( FT_TS_Face   face,
                       FT_TS_Bytes  table )
  {
    FT_TS_Memory  memory;


    if ( !face )
      return;

    memory = FT_TS_FACE_MEMORY( face );


    FT_TS_FREE( table );
  }


/* END */
