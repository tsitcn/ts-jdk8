/****************************************************************************
 *
 * ttsbit.h
 *
 *   TrueType and OpenType embedded bitmap support (specification).
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


#ifndef TTSBIT_H_
#define TTSBIT_H_


#include "ttload.h"


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_sbit( TT_Face    face,
                     FT_TS_Stream  stream );

  FT_TS_LOCAL( void )
  tt_face_free_sbit( TT_Face  face );


  FT_TS_LOCAL( FT_TS_Error )
  tt_face_set_sbit_strike( TT_Face          face,
                           FT_TS_Size_Request  req,
                           FT_TS_ULong*        astrike_index );

  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_strike_metrics( TT_Face           face,
                               FT_TS_ULong          strike_index,
                               FT_TS_Size_Metrics*  metrics );

  FT_TS_LOCAL( FT_TS_Error )
  tt_face_load_sbit_image( TT_Face              face,
                           FT_TS_ULong             strike_index,
                           FT_TS_UInt              glyph_index,
                           FT_TS_UInt              load_flags,
                           FT_TS_Stream            stream,
                           FT_TS_Bitmap           *map,
                           TT_SBit_MetricsRec  *metrics );


FT_TS_END_HEADER

#endif /* TTSBIT_H_ */


/* END */
