/****************************************************************************
 *
 * ftotval.c
 *
 *   FreeType API for validating OpenType tables (body).
 *
 * Copyright (C) 2004-2022 by
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
#include <freetype/internal/services/svotval.h>
#include <freetype/ftotval.h>


  /* documentation is in ftotval.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_OpenType_Validate( FT_TS_Face    face,
                        FT_TS_UInt    validation_flags,
                        FT_TS_Bytes  *BASE_table,
                        FT_TS_Bytes  *GDEF_table,
                        FT_TS_Bytes  *GPOS_table,
                        FT_TS_Bytes  *GSUB_table,
                        FT_TS_Bytes  *JSTF_table )
  {
    FT_TS_Service_OTvalidate  service;
    FT_TS_Error               error;


    if ( !face )
    {
      error = FT_TS_THROW( Invalid_Face_Handle );
      goto Exit;
    }

    if ( !( BASE_table &&
            GDEF_table &&
            GPOS_table &&
            GSUB_table &&
            JSTF_table ) )
    {
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    FT_TS_FACE_FIND_GLOBAL_SERVICE( face, service, OPENTYPE_VALIDATE );

    if ( service )
      error = service->validate( face,
                                 validation_flags,
                                 BASE_table,
                                 GDEF_table,
                                 GPOS_table,
                                 GSUB_table,
                                 JSTF_table );
    else
      error = FT_TS_THROW( Unimplemented_Feature );

  Exit:
    return error;
  }


  FT_TS_EXPORT_DEF( void )
  FT_TS_OpenType_Free( FT_TS_Face   face,
                    FT_TS_Bytes  table )
  {
    FT_TS_Memory  memory;


    if ( !face )
      return;

    memory = FT_TS_FACE_MEMORY( face );

    FT_TS_FREE( table );
  }


/* END */
