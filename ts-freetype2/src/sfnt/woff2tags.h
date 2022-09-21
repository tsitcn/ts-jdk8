/****************************************************************************
 *
 * woff2tags.h
 *
 *   WOFF2 Font table tags (specification).
 *
 * Copyright (C) 2019-2022 by
 * Nikhil Ramakrishnan, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef WOFF2TAGS_H
#define WOFF2TAGS_H


#include <freetype/internal/ftobjs.h>
#include <freetype/internal/compiler-macros.h>


FT_TS_BEGIN_HEADER

#ifdef FT_TS_CONFIG_OPTION_USE_BROTLI

  FT_TS_LOCAL( FT_TS_Tag )
  woff2_known_tags( FT_TS_Byte  index );

#endif

FT_TS_END_HEADER

#endif /* WOFF2TAGS_H */


/* END */
