/****************************************************************************
 *
 * cffobjs.h
 *
 *   OpenType objects manager (specification).
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


#ifndef CFFOBJS_H_
#define CFFOBJS_H_




FT_TS_BEGIN_HEADER


  FT_TS_LOCAL( FT_TS_Error )
  cff_size_init( FT_TS_Size  size );           /* CFF_Size */

  FT_TS_LOCAL( void )
  cff_size_done( FT_TS_Size  size );           /* CFF_Size */

  FT_TS_LOCAL( FT_TS_Error )
  cff_size_request( FT_TS_Size          size,
                    FT_TS_Size_Request  req );

#ifdef TT_CONFIG_OPTION_EMBEDDED_BITMAPS

  FT_TS_LOCAL( FT_TS_Error )
  cff_size_select( FT_TS_Size   size,
                   FT_TS_ULong  strike_index );

#endif

  FT_TS_LOCAL( void )
  cff_slot_done( FT_TS_GlyphSlot  slot );

  FT_TS_LOCAL( FT_TS_Error )
  cff_slot_init( FT_TS_GlyphSlot  slot );


  /**************************************************************************
   *
   * Face functions
   */
  FT_TS_LOCAL( FT_TS_Error )
  cff_face_init( FT_TS_Stream      stream,
                 FT_TS_Face        face,           /* CFF_Face */
                 FT_TS_Int         face_index,
                 FT_TS_Int         num_params,
                 FT_TS_Parameter*  params );

  FT_TS_LOCAL( void )
  cff_face_done( FT_TS_Face  face );               /* CFF_Face */


  /**************************************************************************
   *
   * Driver functions
   */
  FT_TS_LOCAL( FT_TS_Error )
  cff_driver_init( FT_TS_Module  module );         /* PS_Driver */

  FT_TS_LOCAL( void )
  cff_driver_done( FT_TS_Module  module );         /* PS_Driver */


FT_TS_END_HEADER

#endif /* CFFOBJS_H_ */


/* END */
