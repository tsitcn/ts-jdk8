/****************************************************************************
 *
 * pngshim.h
 *
 *   PNG Bitmap glyph support.
 *
 * Copyright (C) 2013-2022 by
 * Google, Inc.
 * Written by Stuart Gill and Behdad Esfahbod.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef PNGSHIM_H_
#define PNGSHIM_H_


#include "ttload.h"


FT_TS_BEGIN_HEADER

#ifdef FT_TS_CONFIG_OPTION_USE_PNG

  FT_TS_LOCAL( FT_TS_Error )
  Load_SBit_Png( FT_TS_GlyphSlot     slot,
                 FT_TS_Int           x_offset,
                 FT_TS_Int           y_offset,
                 FT_TS_Int           pix_bits,
                 TT_SBit_Metrics  metrics,
                 FT_TS_Memory        memory,
                 FT_TS_Byte*         data,
                 FT_TS_UInt          png_len,
                 FT_TS_Bool          populate_map_and_metrics,
                 FT_TS_Bool          metrics_only );

#endif

FT_TS_END_HEADER

#endif /* PNGSHIM_H_ */


/* END */
