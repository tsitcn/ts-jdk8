/****************************************************************************
 *
 * pfrgload.h
 *
 *   FreeType PFR glyph loader (specification).
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


#ifndef PFRGLOAD_H_
#define PFRGLOAD_H_

#include "pfrtypes.h"

FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( void )
  pfr_glyph_init( PFR_Glyph       glyph,
                  FT_TS_GlyphLoader  loader );

  FT_TS_LOCAL( void )
  pfr_glyph_done( PFR_Glyph  glyph );


  FT_TS_LOCAL( FT_TS_Error )
  pfr_glyph_load( PFR_Glyph  glyph,
                  FT_TS_Stream  stream,
                  FT_TS_ULong   gps_offset,
                  FT_TS_ULong   offset,
                  FT_TS_ULong   size );


FT_TS_END_HEADER


#endif /* PFRGLOAD_H_ */


/* END */
