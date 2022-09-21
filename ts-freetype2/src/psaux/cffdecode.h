/****************************************************************************
 *
 * cffdecode.h
 *
 *   PostScript CFF (Type 2) decoding routines (specification).
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


#ifndef CFFDECODE_H_
#define CFFDECODE_H_


#include <freetype/internal/psaux.h>


FT_TS_BEGIN_HEADER

  FT_TS_LOCAL( void )
  cff_decoder_init( CFF_Decoder*                     decoder,
                    TT_Face                          face,
                    CFF_Size                         size,
                    CFF_GlyphSlot                    slot,
                    FT_TS_Bool                          hinting,
                    FT_TS_Render_Mode                   hint_mode,
                    CFF_Decoder_Get_Glyph_Callback   get_callback,
                    CFF_Decoder_Free_Glyph_Callback  free_callback );

  FT_TS_LOCAL( FT_TS_Error )
  cff_decoder_prepare( CFF_Decoder*  decoder,
                       CFF_Size      size,
                       FT_TS_UInt       glyph_index );


  FT_TS_LOCAL( FT_TS_Int )
  cff_lookup_glyph_by_stdcharcode( CFF_Font  cff,
                                   FT_TS_Int    charcode );


#ifdef CFF_CONFIG_OPTION_OLD_ENGINE
  FT_TS_LOCAL( FT_TS_Error )
  cff_decoder_parse_charstrings( CFF_Decoder*  decoder,
                                 FT_TS_Byte*      charstring_base,
                                 FT_TS_ULong      charstring_len,
                                 FT_TS_Bool       in_dict );
#endif


FT_TS_END_HEADER

#endif


/* END */
