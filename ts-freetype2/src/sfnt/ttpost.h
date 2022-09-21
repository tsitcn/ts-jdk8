/****************************************************************************
 *
 * ttpost.h
 *
 *   PostScript name table processing for TrueType and OpenType fonts
 *   (specification).
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


#ifndef TTPOST_H_
#define TTPOST_H_


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/internal/tttypes.h>


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_get_ps_name( TT_Face      face,
                       FT_TS_UInt      idx,
                       FT_TS_String**  PSname );

  FT_TS_LOCAL( void )
  tt_face_free_ps_names( TT_Face  face );


FT_TS_END_HEADER

#endif /* TTPOST_H_ */


/* END */
