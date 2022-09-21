/****************************************************************************
 *
 * cidload.h
 *
 *   CID-keyed Type1 font loader (specification).
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


#ifndef CIDLOAD_H_
#define CIDLOAD_H_


#include <freetype/internal/ftstream.h>
#include "cidparse.h"


FT_TS_BEGIN_HEADER


  typedef struct  CID_Loader_
  {
    CID_Parser  parser;          /* parser used to read the stream */
    FT_TS_Int      num_chars;       /* number of characters in encoding */

  } CID_Loader;


  FT_TS_LOCAL( FT_TS_ULong )
  cid_get_offset( FT_TS_Byte**  start,
                  FT_TS_UInt    offsize );

  FT_TS_LOCAL( FT_TS_Error )
  cid_face_open( CID_Face  face,
                 FT_TS_Int    face_index );


FT_TS_END_HEADER

#endif /* CIDLOAD_H_ */


/* END */
