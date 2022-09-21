/****************************************************************************
 *
 * psconv.h
 *
 *   Some convenience conversions (specification).
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


#ifndef PSCONV_H_
#define PSCONV_H_


#include <freetype/internal/psaux.h>

FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Long )
  PS_Conv_Strtol( FT_TS_Byte**  cursor,
                  FT_TS_Byte*   limit,
                  FT_TS_Long    base );


  FT_TS_LOCAL( FT_TS_Long )
  PS_Conv_ToInt( FT_TS_Byte**  cursor,
                 FT_TS_Byte*   limit );

  FT_TS_LOCAL( FT_TS_Fixed )
  PS_Conv_ToFixed( FT_TS_Byte**  cursor,
                   FT_TS_Byte*   limit,
                   FT_TS_Long    power_ten );

#if 0
  FT_TS_LOCAL( FT_TS_UInt )
  PS_Conv_StringDecode( FT_TS_Byte**  cursor,
                        FT_TS_Byte*   limit,
                        FT_TS_Byte*   buffer,
                        FT_TS_Offset  n );
#endif

  FT_TS_LOCAL( FT_TS_UInt )
  PS_Conv_ASCIIHexDecode( FT_TS_Byte**  cursor,
                          FT_TS_Byte*   limit,
                          FT_TS_Byte*   buffer,
                          FT_TS_Offset  n );

  FT_TS_LOCAL( FT_TS_UInt )
  PS_Conv_EexecDecode( FT_TS_Byte**   cursor,
                       FT_TS_Byte*    limit,
                       FT_TS_Byte*    buffer,
                       FT_TS_Offset   n,
                       FT_TS_UShort*  seed );


FT_TS_END_HEADER

#endif /* PSCONV_H_ */


/* END */
