/****************************************************************************
 *
 * cffload.h
 *
 *   OpenType & CFF data/program tables loader (specification).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef CFFLOAD_H_
#define CFFLOAD_H_


#include <freetype/internal/cfftypes.h>
#include "cffparse.h"
#include <freetype/internal/cffotypes.h>  /* for CFF_Face */


FT_TS_BEGIN_HEADER

  FT_TS_LOCAL( FT_TS_UShort )
  cff_get_standard_encoding( FT_TS_UInt  charcode );


  FT_TS_LOCAL( FT_TS_String* )
  cff_index_get_string( CFF_Font  font,
                        FT_TS_UInt   element );

  FT_TS_LOCAL( FT_TS_String* )
  cff_index_get_sid_string( CFF_Font  font,
                            FT_TS_UInt   sid );


  FT_TS_LOCAL( FT_TS_Error )
  cff_index_access_element( CFF_Index  idx,
                            FT_TS_UInt    element,
                            FT_TS_Byte**  pbytes,
                            FT_TS_ULong*  pbyte_len );

  FT_TS_LOCAL( void )
  cff_index_forget_element( CFF_Index  idx,
                            FT_TS_Byte**  pbytes );

  FT_TS_LOCAL( FT_TS_String* )
  cff_index_get_name( CFF_Font  font,
                      FT_TS_UInt   element );


  FT_TS_LOCAL( FT_TS_UInt )
  cff_charset_cid_to_gindex( CFF_Charset  charset,
                             FT_TS_UInt      cid );


  FT_TS_LOCAL( FT_TS_Error )
  cff_font_load( FT_TS_Library  library,
                 FT_TS_Stream   stream,
                 FT_TS_Int      face_index,
                 CFF_Font    font,
                 CFF_Face    face,
                 FT_TS_Bool     pure_cff,
                 FT_TS_Bool     cff2 );

  FT_TS_LOCAL( void )
  cff_font_done( CFF_Font  font );


  FT_TS_LOCAL( FT_TS_Error )
  cff_load_private_dict( CFF_Font     font,
                         CFF_SubFont  subfont,
                         FT_TS_UInt      lenNDV,
                         FT_TS_Fixed*    NDV );

  FT_TS_LOCAL( FT_TS_Byte )
  cff_fd_select_get( CFF_FDSelect  fdselect,
                     FT_TS_UInt       glyph_index );

  FT_TS_LOCAL( FT_TS_Bool )
  cff_blend_check_vector( CFF_Blend  blend,
                          FT_TS_UInt    vsindex,
                          FT_TS_UInt    lenNDV,
                          FT_TS_Fixed*  NDV );

  FT_TS_LOCAL( FT_TS_Error )
  cff_blend_build_vector( CFF_Blend  blend,
                          FT_TS_UInt    vsindex,
                          FT_TS_UInt    lenNDV,
                          FT_TS_Fixed*  NDV );

  FT_TS_LOCAL( void )
  cff_blend_clear( CFF_SubFont  subFont );

  FT_TS_LOCAL( FT_TS_Error )
  cff_blend_doBlend( CFF_SubFont  subfont,
                     CFF_Parser   parser,
                     FT_TS_UInt      numBlends );

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
  FT_TS_LOCAL( FT_TS_Error )
  cff_get_var_blend( CFF_Face     face,
                     FT_TS_UInt     *num_coords,
                     FT_TS_Fixed*   *coords,
                     FT_TS_Fixed*   *normalizedcoords,
                     FT_TS_MM_Var*  *mm_var );

  FT_TS_LOCAL( void )
  cff_done_blend( CFF_Face  face );
#endif


FT_TS_END_HEADER

#endif /* CFFLOAD_H_ */


/* END */
