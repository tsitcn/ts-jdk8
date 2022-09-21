/****************************************************************************
 *
 * afloader.h
 *
 *   Auto-fitter glyph loading routines (specification).
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


#ifndef AFLOADER_H_
#define AFLOADER_H_

#include "afhints.h"
#include "afmodule.h"
#include "afglobal.h"


FT_TS_BEGIN_HEADER

  /*
   * The autofitter module's (global) data structure to communicate with
   * actual fonts.  If necessary, `local' data like the current face, the
   * current face's auto-hint data, or the current glyph's parameters
   * relevant to auto-hinting are `swapped in'.  Cf. functions like
   * `af_loader_reset' and `af_loader_load_g'.
   */

  typedef struct  AF_LoaderRec_
  {
    /* current face data */
    FT_TS_Face           face;
    AF_FaceGlobals    globals;

    /* current glyph data */
    AF_GlyphHints     hints;
    AF_StyleMetrics   metrics;
    FT_TS_Bool           transformed;
    FT_TS_Matrix         trans_matrix;
    FT_TS_Vector         trans_delta;
    FT_TS_Vector         pp1;
    FT_TS_Vector         pp2;
    /* we don't handle vertical phantom points */

  } AF_LoaderRec, *AF_Loader;


  FT_TS_LOCAL( void )
  af_loader_init( AF_Loader      loader,
                  AF_GlyphHints  hints );


  FT_TS_LOCAL( FT_TS_Error )
  af_loader_reset( AF_Loader  loader,
                   AF_Module  module,
                   FT_TS_Face    face );


  FT_TS_LOCAL( void )
  af_loader_done( AF_Loader  loader );


  FT_TS_LOCAL( FT_TS_Error )
  af_loader_load_glyph( AF_Loader  loader,
                        AF_Module  module,
                        FT_TS_Face    face,
                        FT_TS_UInt    gindex,
                        FT_TS_Int32   load_flags );

  FT_TS_LOCAL_DEF( FT_TS_Fixed )
  af_loader_compute_darkening( AF_Loader  loader,
                               FT_TS_Face    face,
                               FT_TS_Pos     standard_width );

/* */


FT_TS_END_HEADER

#endif /* AFLOADER_H_ */


/* END */
