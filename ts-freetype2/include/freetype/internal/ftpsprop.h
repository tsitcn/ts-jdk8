/****************************************************************************
 *
 * ftpsprop.h
 *
 *   Get and set properties of PostScript drivers (specification).
 *
 * Copyright (C) 2017-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTPSPROP_H_
#define FTPSPROP_H_


#include <freetype/freetype.h>


FT_TS_BEGIN_HEADER


  FT_TS_BASE_CALLBACK( FT_TS_Error )
  ps_property_set( FT_TS_Module    module,         /* PS_Driver */
                   const char*  property_name,
                   const void*  value,
                   FT_TS_Bool      value_is_string );

  FT_TS_BASE_CALLBACK( FT_TS_Error )
  ps_property_get( FT_TS_Module    module,         /* PS_Driver */
                   const char*  property_name,
                   void*        value );


FT_TS_END_HEADER


#endif /* FTPSPROP_H_ */


/* END */
