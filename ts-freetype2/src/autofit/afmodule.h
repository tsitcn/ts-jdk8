/****************************************************************************
 *
 * afmodule.h
 *
 *   Auto-fitter module implementation (specification).
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


#ifndef AFMODULE_H_
#define AFMODULE_H_

#include <freetype/internal/ftobjs.h>
#include <freetype/ftmodapi.h>


FT_TS_BEGIN_HEADER


  /*
   * This is the `extended' FT_TS_Module structure that holds the
   * autofitter's global data.
   */

  typedef struct  AF_ModuleRec_
  {
    FT_TS_ModuleRec  root;

    FT_TS_UInt       fallback_style;
    FT_TS_UInt       default_script;
    FT_TS_Bool       no_stem_darkening;
    FT_TS_Int        darken_params[8];

  } AF_ModuleRec, *AF_Module;


FT_TS_DECLARE_AUTOHINTER_INTERFACE( af_autofitter_interface )
FT_TS_DECLARE_MODULE( autofit_module_class )


FT_TS_END_HEADER

#endif /* AFMODULE_H_ */


/* END */
