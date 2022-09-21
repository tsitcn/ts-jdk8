/****************************************************************************
 *
 * t1objs.h
 *
 *   Type 1 objects manager (specification).
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


#ifndef T1OBJS_H_
#define T1OBJS_H_


#include <ft2build.h>
#include <freetype/internal/ftobjs.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/internal/t1types.h>


FT_TS_BEGIN_HEADER


  /* The following structures must be defined by the hinter */
  typedef struct T1_Size_Hints_   T1_Size_Hints;
  typedef struct T1_Glyph_Hints_  T1_Glyph_Hints;


  /**************************************************************************
   *
   * @Type:
   *   T1_Size
   *
   * @Description:
   *   A handle to a Type 1 size object.
   */
  typedef struct T1_SizeRec_*  T1_Size;


  /**************************************************************************
   *
   * @Type:
   *   T1_GlyphSlot
   *
   * @Description:
   *   A handle to a Type 1 glyph slot object.
   */
  typedef struct T1_GlyphSlotRec_*  T1_GlyphSlot;


  /**************************************************************************
   *
   * @Type:
   *   T1_CharMap
   *
   * @Description:
   *   A handle to a Type 1 character mapping object.
   *
   * @Note:
   *   The Type 1 format doesn't use a charmap but an encoding table.
   *   The driver is responsible for making up charmap objects
   *   corresponding to these tables.
   */
  typedef struct T1_CharMapRec_*   T1_CharMap;


  /**************************************************************************
   *
   *                 HERE BEGINS THE TYPE1 SPECIFIC STUFF
   *
   */


  /**************************************************************************
   *
   * @Type:
   *   T1_SizeRec
   *
   * @Description:
   *   Type 1 size record.
   */
  typedef struct  T1_SizeRec_
  {
    FT_TS_SizeRec  root;

  } T1_SizeRec;


  FT_TS_LOCAL( void )
  T1_Size_Done( FT_TS_Size  size );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Size_Request( FT_TS_Size          size,
                   FT_TS_Size_Request  req );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Size_Init( FT_TS_Size  size );


  /**************************************************************************
   *
   * @Type:
   *   T1_GlyphSlotRec
   *
   * @Description:
   *   Type 1 glyph slot record.
   */
  typedef struct  T1_GlyphSlotRec_
  {
    FT_TS_GlyphSlotRec  root;

    FT_TS_Bool          hint;
    FT_TS_Bool          scaled;

    FT_TS_Fixed         x_scale;
    FT_TS_Fixed         y_scale;

    FT_TS_Int           max_points;
    FT_TS_Int           max_contours;

  } T1_GlyphSlotRec;


  FT_TS_LOCAL( FT_TS_Error )
  T1_Face_Init( FT_TS_Stream      stream,
                FT_TS_Face        face,
                FT_TS_Int         face_index,
                FT_TS_Int         num_params,
                FT_TS_Parameter*  params );

  FT_TS_LOCAL( void )
  T1_Face_Done( FT_TS_Face  face );

  FT_TS_LOCAL( FT_TS_Error )
  T1_GlyphSlot_Init( FT_TS_GlyphSlot  slot );

  FT_TS_LOCAL( void )
  T1_GlyphSlot_Done( FT_TS_GlyphSlot  slot );

  FT_TS_LOCAL( FT_TS_Error )
  T1_Driver_Init( FT_TS_Module  driver );

  FT_TS_LOCAL( void )
  T1_Driver_Done( FT_TS_Module  driver );


FT_TS_END_HEADER

#endif /* T1OBJS_H_ */


/* END */
