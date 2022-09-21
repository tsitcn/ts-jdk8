/****************************************************************************
 *
 * svpfr.h
 *
 *   Internal PFR service functions (specification).
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


#ifndef SVPFR_H_
#define SVPFR_H_

#include <freetype/ftpfr.h>
#include <freetype/internal/ftserv.h>


FT_TS_BEGIN_HEADER


#define FT_TS_SERVICE_ID_PFR_METRICS  "pfr-metrics"


  typedef FT_TS_Error
  (*FT_TS_PFR_GetMetricsFunc)( FT_TS_Face    face,
                            FT_TS_UInt   *aoutline,
                            FT_TS_UInt   *ametrics,
                            FT_TS_Fixed  *ax_scale,
                            FT_TS_Fixed  *ay_scale );

  typedef FT_TS_Error
  (*FT_TS_PFR_GetKerningFunc)( FT_TS_Face     face,
                            FT_TS_UInt     left,
                            FT_TS_UInt     right,
                            FT_TS_Vector  *avector );

  typedef FT_TS_Error
  (*FT_TS_PFR_GetAdvanceFunc)( FT_TS_Face   face,
                            FT_TS_UInt   gindex,
                            FT_TS_Pos   *aadvance );


  FT_TS_DEFINE_SERVICE( PfrMetrics )
  {
    FT_TS_PFR_GetMetricsFunc  get_metrics;
    FT_TS_PFR_GetKerningFunc  get_kerning;
    FT_TS_PFR_GetAdvanceFunc  get_advance;

  };


FT_TS_END_HEADER

#endif /* SVPFR_H_ */


/* END */
