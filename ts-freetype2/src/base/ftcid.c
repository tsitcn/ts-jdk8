/****************************************************************************
 *
 * ftcid.c
 *
 *   FreeType API for accessing CID font information.
 *
 * Copyright (C) 2007-2022 by
 * Derek Clegg and Michael Toftdal.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/ftcid.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svcid.h>


  /* documentation is in ftcid.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_CID_Registry_Ordering_Supplement( FT_TS_Face       face,
                                           const char*  *registry,
                                           const char*  *ordering,
                                           FT_TS_Int       *supplement)
  {
    FT_TS_Error     error;
    const char*  r = NULL;
    const char*  o = NULL;
    FT_TS_Int       s = 0;


    error = FT_TS_ERR( Invalid_Argument );

    if ( face )
    {
      FT_TS_Service_CID  service;


      FT_TS_FACE_FIND_SERVICE( face, service, CID );

      if ( service && service->get_ros )
        error = service->get_ros( face, &r, &o, &s );
    }

    if ( registry )
      *registry = r;

    if ( ordering )
      *ordering = o;

    if ( supplement )
      *supplement = s;

    return error;
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_CID_Is_Internally_CID_Keyed( FT_TS_Face   face,
                                      FT_TS_Bool  *is_cid )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );
    FT_TS_Bool   ic = 0;


    if ( face )
    {
      FT_TS_Service_CID  service;


      FT_TS_FACE_FIND_SERVICE( face, service, CID );

      if ( service && service->get_is_cid )
        error = service->get_is_cid( face, &ic);
    }

    if ( is_cid )
      *is_cid = ic;

    return error;
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_CID_From_Glyph_Index( FT_TS_Face   face,
                               FT_TS_UInt   glyph_index,
                               FT_TS_UInt  *cid )
  {
    FT_TS_Error  error = FT_TS_ERR( Invalid_Argument );
    FT_TS_UInt   c = 0;


    if ( face )
    {
      FT_TS_Service_CID  service;


      FT_TS_FACE_FIND_SERVICE( face, service, CID );

      if ( service && service->get_cid_from_glyph_index )
        error = service->get_cid_from_glyph_index( face, glyph_index, &c);
    }

    if ( cid )
      *cid = c;

    return error;
  }


/* END */
