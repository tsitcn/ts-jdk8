/****************************************************************************
 *
 * ftwinfnt.c
 *
 *   FreeType API for accessing Windows FNT specific info (body).
 *
 * Copyright (C) 2003-2022 by
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
#include <freetype/ftwinfnt.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svwinfnt.h>


  /* documentation is in ftwinfnt.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_WinFNT_Header( FT_TS_Face               face,
                        FT_TS_WinFNT_HeaderRec  *header )
  {
    FT_TS_Service_WinFnt  service;
    FT_TS_Error           error;


    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    if ( !header )
      return FT_TS_THROW( Invalid_Argument );

    FT_TS_FACE_LOOKUP_SERVICE( face, service, WINFNT );

    if ( service )
      error = service->get_header( face, header );
    else
      error = FT_TS_THROW( Invalid_Argument );

    return error;
  }


/* END */
