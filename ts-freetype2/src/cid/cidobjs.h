/****************************************************************************
 *
 * cidobjs.h
 *
 *   CID objects manager (specification).
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


#ifndef CIDOBJS_H_
#define CIDOBJS_H_


#include <ft2build.h>
#include <freetype/internal/ftobjs.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/internal/t1types.h>


FT_TS_BEGIN_HEADER


  /* The following structures must be defined by the hinter */
  typedef struct CID_Size_Hints_   CID_Size_Hints;
  typedef struct CID_Glyph_Hints_  CID_Glyph_Hints;


  /**************************************************************************
   *
   * @Type:
   *   CID_Driver
   *
   * @Description:
   *   A handle to a Type 1 driver object.
   */
  typedef struct CID_DriverRec_*  CID_Driver;


  /**************************************************************************
   *
   * @Type:
   *   CID_Size
   *
   * @Description:
   *   A handle to a Type 1 size object.
   */
  typedef struct CID_SizeRec_*  CID_Size;


  /**************************************************************************
   *
   * @Type:
   *   CID_GlyphSlot
   *
   * @Description:
   *   A handle to a Type 1 glyph slot object.
   */
  typedef struct CID_GlyphSlotRec_*  CID_GlyphSlot;


  /**************************************************************************
   *
   * @Type:
   *   CID_CharMap
   *
   * @Description:
   *   A handle to a Type 1 character mapping object.
   *
   * @Note:
   *   The Type 1 format doesn't use a charmap but an encoding table.
   *   The driver is responsible for making up charmap objects
   *   corresponding to these tables.
   */
  typedef struct CID_CharMapRec_*  CID_CharMap;


  /**************************************************************************
   *
   * HERE BEGINS THE TYPE 1 SPECIFIC STUFF
   *
   */


  typedef struct  CID_SizeRec_
  {
    FT_TS_SizeRec  root;
    FT_TS_Bool     valid;

  } CID_SizeRec;


  typedef struct  CID_GlyphSlotRec_
  {
    FT_TS_GlyphSlotRec  root;

    FT_TS_Bool          hint;
    FT_TS_Bool          scaled;

    FT_TS_Fixed         x_scale;
    FT_TS_Fixed         y_scale;

  } CID_GlyphSlotRec;


  FT_TS_LOCAL( void )
  cid_slot_done( FT_TS_GlyphSlot  slot );

  FT_TS_LOCAL( FT_TS_Error )
  cid_slot_init( FT_TS_GlyphSlot  slot );


  FT_TS_LOCAL( void )
  cid_size_done( FT_TS_Size  size );       /* CID_Size */

  FT_TS_LOCAL( FT_TS_Error )
  cid_size_init( FT_TS_Size  size );       /* CID_Size */

  FT_TS_LOCAL( FT_TS_Error )
  cid_size_request( FT_TS_Size          size,      /* CID_Size */
                    FT_TS_Size_Request  req );

  FT_TS_LOCAL( FT_TS_Error )
  cid_face_init( FT_TS_Stream      stream,
                 FT_TS_Face        face,           /* CID_Face */
                 FT_TS_Int         face_index,
                 FT_TS_Int         num_params,
                 FT_TS_Parameter*  params );

  FT_TS_LOCAL( void )
  cid_face_done( FT_TS_Face  face );               /* CID_Face */


  FT_TS_LOCAL( FT_TS_Error )
  cid_driver_init( FT_TS_Module  driver );

  FT_TS_LOCAL( void )
  cid_driver_done( FT_TS_Module  driver );


FT_TS_END_HEADER

#endif /* CIDOBJS_H_ */


/* END */
