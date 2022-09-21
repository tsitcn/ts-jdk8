/****************************************************************************
 *
 * fterrors.c
 *
 *   FreeType API for error code handling.
 *
 * Copyright (C) 2018-2022 by
 * Armin Hasitzka, David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#include <freetype/internal/ftdebug.h>
#include <freetype/fterrors.h>


  /* documentation is in fterrors.h */

  FT_TS_EXPORT_DEF( const char* )
  FT_TS_Error_String( FT_TS_Error  error_code )
  {
    if ( error_code <  0                                ||
         error_code >= FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, Max ) )
      return NULL;

#if defined( FT_TS_CONFIG_OPTION_ERROR_STRINGS ) || \
    defined( FT_TS_DEBUG_LEVEL_ERROR )

#undef FTERRORS_H_
#define FT_TS_ERROR_START_LIST     switch ( FT_TS_ERROR_BASE( error_code ) ) {
#define FT_TS_ERRORDEF( e, v, s )    case v: return s;
#define FT_TS_ERROR_END_LIST       }

#include <freetype/fterrors.h>

#endif /* defined( FT_TS_CONFIG_OPTION_ERROR_STRINGS ) || ... */

    return NULL;
  }
