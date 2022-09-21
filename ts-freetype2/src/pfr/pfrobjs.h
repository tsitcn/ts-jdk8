/****************************************************************************
 *
 * pfrobjs.h
 *
 *   FreeType PFR object methods (specification).
 *
 * Copyright (C) 2002-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef PFROBJS_H_
#define PFROBJS_H_

#include "pfrtypes.h"


FT_TS_BEGIN_HEADER

  typedef struct PFR_FaceRec_*  PFR_Face;

  typedef struct PFR_SizeRec_*  PFR_Size;

  typedef struct PFR_SlotRec_*  PFR_Slot;


  typedef struct  PFR_FaceRec_
  {
    FT_TS_FaceRec      root;
    PFR_HeaderRec   header;
    PFR_LogFontRec  log_font;
    PFR_PhyFontRec  phy_font;

  } PFR_FaceRec;


  typedef struct  PFR_SizeRec_
  {
    FT_TS_SizeRec  root;

  } PFR_SizeRec;


  typedef struct  PFR_SlotRec_
  {
    FT_TS_GlyphSlotRec  root;
    PFR_GlyphRec     glyph;

  } PFR_SlotRec;


  FT_TS_LOCAL( FT_TS_Error )
  pfr_face_init( FT_TS_Stream      stream,
                 FT_TS_Face        face,           /* PFR_Face */
                 FT_TS_Int         face_index,
                 FT_TS_Int         num_params,
                 FT_TS_Parameter*  params );

  FT_TS_LOCAL( void )
  pfr_face_done( FT_TS_Face  face );               /* PFR_Face */


  FT_TS_LOCAL( FT_TS_Error )
  pfr_face_get_kerning( FT_TS_Face     face,       /* PFR_Face */
                        FT_TS_UInt     glyph1,
                        FT_TS_UInt     glyph2,
                        FT_TS_Vector*  kerning );


  FT_TS_LOCAL( FT_TS_Error )
  pfr_slot_init( FT_TS_GlyphSlot  slot );          /* PFR_Slot */

  FT_TS_LOCAL( void )
  pfr_slot_done( FT_TS_GlyphSlot  slot );          /* PFR_Slot */


  FT_TS_LOCAL( FT_TS_Error )
  pfr_slot_load( FT_TS_GlyphSlot  slot,            /* PFR_Slot */
                 FT_TS_Size       size,            /* PFR_Size */
                 FT_TS_UInt       gindex,
                 FT_TS_Int32      load_flags );


FT_TS_END_HEADER

#endif /* PFROBJS_H_ */


/* END */
