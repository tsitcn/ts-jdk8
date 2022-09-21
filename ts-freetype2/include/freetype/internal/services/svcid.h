/****************************************************************************
 *
 * svcid.h
 *
 *   The FreeType CID font services (specification).
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


#ifndef SVCID_H_
#define SVCID_H_

#include <freetype/internal/ftserv.h>


FT_TS_BEGIN_HEADER


#define FT_TS_SERVICE_ID_CID  "CID"

  typedef FT_TS_Error
  (*FT_TS_CID_GetRegistryOrderingSupplementFunc)( FT_TS_Face       face,
                                               const char*  *registry,
                                               const char*  *ordering,
                                               FT_TS_Int       *supplement );
  typedef FT_TS_Error
  (*FT_TS_CID_GetIsInternallyCIDKeyedFunc)( FT_TS_Face   face,
                                         FT_TS_Bool  *is_cid );
  typedef FT_TS_Error
  (*FT_TS_CID_GetCIDFromGlyphIndexFunc)( FT_TS_Face   face,
                                      FT_TS_UInt   glyph_index,
                                      FT_TS_UInt  *cid );

  FT_TS_DEFINE_SERVICE( CID )
  {
    FT_TS_CID_GetRegistryOrderingSupplementFunc  get_ros;
    FT_TS_CID_GetIsInternallyCIDKeyedFunc        get_is_cid;
    FT_TS_CID_GetCIDFromGlyphIndexFunc           get_cid_from_glyph_index;
  };


#define FT_TS_DEFINE_SERVICE_CIDREC( class_,                                   \
                                  get_ros_,                                 \
                                  get_is_cid_,                              \
                                  get_cid_from_glyph_index_ )               \
  static const FT_TS_Service_CIDRec class_ =                                   \
  {                                                                         \
    get_ros_, get_is_cid_, get_cid_from_glyph_index_                        \
  };

  /* */


FT_TS_END_HEADER


#endif /* SVCID_H_ */


/* END */
