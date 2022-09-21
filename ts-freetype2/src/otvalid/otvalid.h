/****************************************************************************
 *
 * otvalid.h
 *
 *   OpenType table validation (specification only).
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


#ifndef OTVALID_H_
#define OTVALID_H_


#include <freetype/freetype.h>

#include "otverror.h"                      /* must come before `ftvalid.h' */

#include <freetype/internal/ftvalid.h>
#include <freetype/internal/ftstream.h>


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( void )
  otv_BASE_validate( FT_TS_Bytes      table,
                     FT_TS_Validator  valid );

  /* GSUB and GPOS tables should already be validated; */
  /* if missing, set corresponding argument to 0       */
  FT_TS_LOCAL( void )
  otv_GDEF_validate( FT_TS_Bytes      table,
                     FT_TS_Bytes      gsub,
                     FT_TS_Bytes      gpos,
                     FT_TS_UInt       glyph_count,
                     FT_TS_Validator  valid );

  FT_TS_LOCAL( void )
  otv_GPOS_validate( FT_TS_Bytes      table,
                     FT_TS_UInt       glyph_count,
                     FT_TS_Validator  valid );

  FT_TS_LOCAL( void )
  otv_GSUB_validate( FT_TS_Bytes      table,
                     FT_TS_UInt       glyph_count,
                     FT_TS_Validator  valid );

  /* GSUB and GPOS tables should already be validated; */
  /* if missing, set corresponding argument to 0       */
  FT_TS_LOCAL( void )
  otv_JSTF_validate( FT_TS_Bytes      table,
                     FT_TS_Bytes      gsub,
                     FT_TS_Bytes      gpos,
                     FT_TS_UInt       glyph_count,
                     FT_TS_Validator  valid );

  FT_TS_LOCAL( void )
  otv_MATH_validate( FT_TS_Bytes      table,
                     FT_TS_UInt       glyph_count,
                     FT_TS_Validator  ftvalid );


FT_TS_END_HEADER

#endif /* OTVALID_H_ */


/* END */
