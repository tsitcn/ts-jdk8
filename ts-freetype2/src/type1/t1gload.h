/****************************************************************************
 *
 * t1gload.h
 *
 *   Type 1 Glyph Loader (specification).
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


#ifndef T1GLOAD_H_
#define T1GLOAD_H_


#include "t1objs.h"


FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  T1_Compute_Max_Advance( T1_Face  face,
                          FT_TS_Pos*  max_advance );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Get_Advances( FT_TS_Face    face,
                   FT_TS_UInt    first,
                   FT_TS_UInt    count,
                   FT_TS_Int32   load_flags,
                   FT_TS_Fixed*  advances );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Load_Glyph( FT_TS_GlyphSlot  glyph,
                 FT_TS_Size       size,
                 FT_TS_UInt       glyph_index,
                 FT_TS_Int32      load_flags );


FT_TS_END_HEADER

#endif /* T1GLOAD_H_ */


/* END */
