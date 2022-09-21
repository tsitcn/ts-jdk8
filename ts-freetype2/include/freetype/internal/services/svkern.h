/****************************************************************************
 *
 * svkern.h
 *
 *   The FreeType Kerning service (specification).
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


#ifndef SVKERN_H_
#define SVKERN_H_

#include <freetype/internal/ftserv.h>
#include <freetype/tttables.h>


FT_TS_BEGIN_HEADER

#define FT_TS_SERVICE_ID_KERNING  "kerning"


  typedef FT_TS_Error
  (*FT_TS_Kerning_TrackGetFunc)( FT_TS_Face    face,
                              FT_TS_Fixed   point_size,
                              FT_TS_Int     degree,
                              FT_TS_Fixed*  akerning );

  FT_TS_DEFINE_SERVICE( Kerning )
  {
    FT_TS_Kerning_TrackGetFunc  get_track;
  };

  /* */


FT_TS_END_HEADER


#endif /* SVKERN_H_ */


/* END */
