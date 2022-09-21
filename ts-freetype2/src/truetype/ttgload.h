/****************************************************************************
 *
 * ttgload.h
 *
 *   TrueType Glyph Loader (specification).
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


#ifndef TTGLOAD_H_
#define TTGLOAD_H_


#include "ttobjs.h"

#ifdef TT_USE_BYTECODE_INTERPRETER
#include "ttinterp.h"
#endif


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( void )
  TT_Init_Glyph_Loading( TT_Face  face );

  FT_TS_LOCAL( void )
  TT_Get_HMetrics( TT_Face     face,
                   FT_TS_UInt     idx,
                   FT_TS_Short*   lsb,
                   FT_TS_UShort*  aw );

  FT_TS_LOCAL( void )
  TT_Get_VMetrics( TT_Face     face,
                   FT_TS_UInt     idx,
                   FT_TS_Pos      yMax,
                   FT_TS_Short*   tsb,
                   FT_TS_UShort*  ah );

  FT_TS_LOCAL( FT_TS_Error )
  TT_Load_Glyph( TT_Size       size,
                 TT_GlyphSlot  glyph,
                 FT_TS_UInt       glyph_index,
                 FT_TS_Int32      load_flags );


FT_TS_END_HEADER

#endif /* TTGLOAD_H_ */


/* END */
