/****************************************************************************
 *
 * t1decode.h
 *
 *   PostScript Type 1 decoding routines (specification).
 *
 * Copyright (C) 2000-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef T1DECODE_H_
#define T1DECODE_H_


#include <freetype/internal/psaux.h>
#include <freetype/internal/t1types.h>


FT_TS_BEGIN_HEADER


  FT_TS_CALLBACK_TABLE
  const T1_Decoder_FuncsRec  t1_decoder_funcs;

  FT_TS_LOCAL( FT_TS_Int )
  t1_lookup_glyph_by_stdcharcode_ps( PS_Decoder*  decoder,
                                     FT_TS_Int       charcode );

#ifdef T1_CONFIG_OPTION_OLD_ENGINE
  FT_TS_LOCAL( FT_TS_Error )
  t1_decoder_parse_glyph( T1_Decoder  decoder,
                          FT_TS_UInt     glyph_index );

  FT_TS_LOCAL( FT_TS_Error )
  t1_decoder_parse_charstrings( T1_Decoder  decoder,
                                FT_TS_Byte*    base,
                                FT_TS_UInt     len );
#else
  FT_TS_LOCAL( FT_TS_Error )
  t1_decoder_parse_metrics( T1_Decoder  decoder,
                            FT_TS_Byte*    charstring_base,
                            FT_TS_UInt     charstring_len );
#endif

  FT_TS_LOCAL( FT_TS_Error )
  t1_decoder_init( T1_Decoder           decoder,
                   FT_TS_Face              face,
                   FT_TS_Size              size,
                   FT_TS_GlyphSlot         slot,
                   FT_TS_Byte**            glyph_names,
                   PS_Blend             blend,
                   FT_TS_Bool              hinting,
                   FT_TS_Render_Mode       hint_mode,
                   T1_Decoder_Callback  parse_glyph );

  FT_TS_LOCAL( void )
  t1_decoder_done( T1_Decoder  decoder );


FT_TS_END_HEADER

#endif /* T1DECODE_H_ */


/* END */
