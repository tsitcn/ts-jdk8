/****************************************************************************
 *
 * svwinfnt.h
 *
 *   The FreeType Windows FNT/FONT service (specification).
 *
 * Copyright (C) 2003-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef SVWINFNT_H_
#define SVWINFNT_H_

#include <freetype/internal/ftserv.h>
#include <freetype/ftwinfnt.h>


FT_TS_BEGIN_HEADER


#define FT_TS_SERVICE_ID_WINFNT  "winfonts"

  typedef FT_TS_Error
  (*FT_TS_WinFnt_GetHeaderFunc)( FT_TS_Face               face,
                              FT_TS_WinFNT_HeaderRec  *aheader );


  FT_TS_DEFINE_SERVICE( WinFnt )
  {
    FT_TS_WinFnt_GetHeaderFunc  get_header;
  };

  /* */


FT_TS_END_HEADER


#endif /* SVWINFNT_H_ */


/* END */
