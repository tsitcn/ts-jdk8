/****************************************************************************
 *
 * ttmtx.h
 *
 *   Load the metrics tables common to TTF and OTF fonts (specification).
 *
 * Copyright (C) 2006-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef TTMTX_H_
#define TTMTX_H_


#include <freetype/internal/ftstream.h>
#include <freetype/internal/tttypes.h>


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_hhea( TT_Face    face,
                     FT_TS_Stream  stream,
                     FT_TS_Bool    vertical );


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_hmtx( TT_Face    face,
                     FT_TS_Stream  stream,
                     FT_TS_Bool    vertical );


  FT_TS_LOCAL( void )
  tt_face_get_metrics( TT_Face     face,
                       FT_TS_Bool     vertical,
                       FT_TS_UInt     gindex,
                       FT_TS_Short*   abearing,
                       FT_TS_UShort*  aadvance );

FT_TS_END_HEADER

#endif /* TTMTX_H_ */


/* END */
