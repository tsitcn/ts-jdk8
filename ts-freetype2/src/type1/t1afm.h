/****************************************************************************
 *
 * t1afm.h
 *
 *   AFM support for Type 1 fonts (specification).
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


#ifndef T1AFM_H_
#define T1AFM_H_

#include "t1objs.h"
#include <freetype/internal/t1types.h>

FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  T1_Read_Metrics( FT_TS_Face    face,
                   FT_TS_Stream  stream );

  FT_TS_LOCAL( void )
  T1_Done_Metrics( FT_TS_Memory     memory,
                   AFM_FontInfo  fi );

  FT_TS_LOCAL( void )
  T1_Get_Kerning( AFM_FontInfo  fi,
                  FT_TS_UInt       glyph1,
                  FT_TS_UInt       glyph2,
                  FT_TS_Vector*    kerning );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Get_Track_Kerning( FT_TS_Face    face,
                        FT_TS_Fixed   ptsize,
                        FT_TS_Int     degree,
                        FT_TS_Fixed*  kerning );

FT_TS_END_HEADER

#endif /* T1AFM_H_ */


/* END */
