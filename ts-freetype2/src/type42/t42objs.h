/****************************************************************************
 *
 * t42objs.h
 *
 *   Type 42 objects manager (specification).
 *
 * Copyright (C) 2002-2022 by
 * Roberto Alameda.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef T42OBJS_H_
#define T42OBJS_H_

#include <freetype/freetype.h>
#include <freetype/t1tables.h>
#include <freetype/internal/t1types.h>
#include "t42types.h"
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/ftdrv.h>
#include <freetype/internal/services/svpscmap.h>
#include <freetype/internal/pshints.h>


FT_TS_BEGIN_HEADER


  /* Type42 size */
  typedef struct  T42_SizeRec_
  {
    FT_TS_SizeRec  root;
    FT_TS_Size     ttsize;

  } T42_SizeRec, *T42_Size;


  /* Type42 slot */
  typedef struct  T42_GlyphSlotRec_
  {
    FT_TS_GlyphSlotRec  root;
    FT_TS_GlyphSlot     ttslot;

  } T42_GlyphSlotRec, *T42_GlyphSlot;


  /* Type 42 driver */
  typedef struct  T42_DriverRec_
  {
    FT_TS_DriverRec     root;
    FT_TS_Driver_Class  ttclazz;

  } T42_DriverRec, *T42_Driver;


  /* */


  FT_TS_LOCAL( FT_TS_Error )
  T42_Face_Init( FT_TS_Stream      stream,
                 FT_TS_Face        face,
                 FT_TS_Int         face_index,
                 FT_TS_Int         num_params,
                 FT_TS_Parameter*  params );


  FT_TS_LOCAL( void )
  T42_Face_Done( FT_TS_Face  face );


  FT_TS_LOCAL( FT_TS_Error )
  T42_Size_Init( FT_TS_Size  size );


  FT_TS_LOCAL( FT_TS_Error )
  T42_Size_Request( FT_TS_Size          size,
                    FT_TS_Size_Request  req );


  FT_TS_LOCAL( FT_TS_Error )
  T42_Size_Select( FT_TS_Size   size,
                   FT_TS_ULong  strike_index );


  FT_TS_LOCAL( void )
  T42_Size_Done( FT_TS_Size  size );


  FT_TS_LOCAL( FT_TS_Error )
  T42_GlyphSlot_Init( FT_TS_GlyphSlot  slot );


  FT_TS_LOCAL( FT_TS_Error )
  T42_GlyphSlot_Load( FT_TS_GlyphSlot  glyph,
                      FT_TS_Size       size,
                      FT_TS_UInt       glyph_index,
                      FT_TS_Int32      load_flags );

  FT_TS_LOCAL( void )
  T42_GlyphSlot_Done( FT_TS_GlyphSlot  slot );


  FT_TS_LOCAL( FT_TS_Error )
  T42_Driver_Init( FT_TS_Module  module );

  FT_TS_LOCAL( void )
  T42_Driver_Done( FT_TS_Module  module );

 /* */

FT_TS_END_HEADER


#endif /* T42OBJS_H_ */


/* END */
