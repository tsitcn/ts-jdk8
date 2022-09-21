/****************************************************************************
 *
 * svprop.h
 *
 *   The FreeType property service (specification).
 *
 * Copyright (C) 2012-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef SVPROP_H_
#define SVPROP_H_


FT_TS_BEGIN_HEADER


#define FT_TS_SERVICE_ID_PROPERTIES  "properties"


  typedef FT_TS_Error
  (*FT_TS_Properties_SetFunc)( FT_TS_Module    module,
                            const char*  property_name,
                            const void*  value,
                            FT_TS_Bool      value_is_string );

  typedef FT_TS_Error
  (*FT_TS_Properties_GetFunc)( FT_TS_Module    module,
                            const char*  property_name,
                            void*        value );


  FT_TS_DEFINE_SERVICE( Properties )
  {
    FT_TS_Properties_SetFunc  set_property;
    FT_TS_Properties_GetFunc  get_property;
  };


#define FT_TS_DEFINE_SERVICE_PROPERTIESREC( class_,          \
                                         set_property_,   \
                                         get_property_ )  \
  static const FT_TS_Service_PropertiesRec  class_ =         \
  {                                                       \
    set_property_,                                        \
    get_property_                                         \
  };

  /* */


FT_TS_END_HEADER


#endif /* SVPROP_H_ */


/* END */
