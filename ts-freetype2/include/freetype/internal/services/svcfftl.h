/****************************************************************************
 *
 * svcfftl.h
 *
 *   The FreeType CFF tables loader service (specification).
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


#ifndef SVCFFTL_H_
#define SVCFFTL_H_

#include <freetype/internal/ftserv.h>
#include <freetype/internal/cfftypes.h>


FT_TS_BEGIN_HEADER


#define FT_TS_SERVICE_ID_CFF_LOAD  "cff-load"


  typedef FT_TS_UShort
  (*FT_TS_Get_Standard_Encoding_Func)( FT_TS_UInt  charcode );

  typedef FT_TS_Error
  (*FT_TS_Load_Private_Dict_Func)( CFF_Font     font,
                                CFF_SubFont  subfont,
                                FT_TS_UInt      lenNDV,
                                FT_TS_Fixed*    NDV );

  typedef FT_TS_Byte
  (*FT_TS_FD_Select_Get_Func)( CFF_FDSelect  fdselect,
                            FT_TS_UInt       glyph_index );

  typedef FT_TS_Bool
  (*FT_TS_Blend_Check_Vector_Func)( CFF_Blend  blend,
                                 FT_TS_UInt    vsindex,
                                 FT_TS_UInt    lenNDV,
                                 FT_TS_Fixed*  NDV );

  typedef FT_TS_Error
  (*FT_TS_Blend_Build_Vector_Func)( CFF_Blend  blend,
                                 FT_TS_UInt    vsindex,
                                 FT_TS_UInt    lenNDV,
                                 FT_TS_Fixed*  NDV );


  FT_TS_DEFINE_SERVICE( CFFLoad )
  {
    FT_TS_Get_Standard_Encoding_Func  get_standard_encoding;
    FT_TS_Load_Private_Dict_Func      load_private_dict;
    FT_TS_FD_Select_Get_Func          fd_select_get;
    FT_TS_Blend_Check_Vector_Func     blend_check_vector;
    FT_TS_Blend_Build_Vector_Func     blend_build_vector;
  };


#define FT_TS_DEFINE_SERVICE_CFFLOADREC( class_,                  \
                                      get_standard_encoding_,  \
                                      load_private_dict_,      \
                                      fd_select_get_,          \
                                      blend_check_vector_,     \
                                      blend_build_vector_ )    \
  static const FT_TS_Service_CFFLoadRec  class_ =                 \
  {                                                            \
    get_standard_encoding_,                                    \
    load_private_dict_,                                        \
    fd_select_get_,                                            \
    blend_check_vector_,                                       \
    blend_build_vector_                                        \
  };


FT_TS_END_HEADER


#endif


/* END */
