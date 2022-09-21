/****************************************************************************
 *
 * ftbase.h
 *
 *   Private functions used in the `base' module (specification).
 *
 * Copyright (C) 2008-2022 by
 * David Turner, Robert Wilhelm, Werner Lemberg, and suzuki toshiya.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTBASE_H_
#define FTBASE_H_


#include <freetype/internal/ftobjs.h>


FT_TS_BEGIN_HEADER


  FT_TS_DECLARE_GLYPH( ft_bitmap_glyph_class )
  FT_TS_DECLARE_GLYPH( ft_outline_glyph_class )
  FT_TS_DECLARE_GLYPH( ft_svg_glyph_class )


#ifdef FT_TS_CONFIG_OPTION_MAC_FONTS

  /* MacOS resource fork cannot exceed 16MB at least for Carbon code; */
  /* see https://support.microsoft.com/en-us/kb/130437                */
#define FT_TS_MAC_RFORK_MAX_LEN  0x00FFFFFFUL


  /* Assume the stream is sfnt-wrapped PS Type1 or sfnt-wrapped CID-keyed */
  /* font, and try to load a face specified by the face_index.            */
  FT_TS_LOCAL( FT_TS_Error )
  open_face_PS_from_sfnt_stream( FT_TS_Library     library,
                                 FT_TS_Stream      stream,
                                 FT_TS_Long        face_index,
                                 FT_TS_Int         num_params,
                                 FT_TS_Parameter  *params,
                                 FT_TS_Face       *aface );


  /* Create a new FT_TS_Face given a buffer and a driver name. */
  /* From ftmac.c.                                          */
  FT_TS_LOCAL( FT_TS_Error )
  open_face_from_buffer( FT_TS_Library   library,
                         FT_TS_Byte*     base,
                         FT_TS_ULong     size,
                         FT_TS_Long      face_index,
                         const char*  driver_name,
                         FT_TS_Face     *aface );


#if  defined( FT_TS_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK ) && \
    !defined( FT_TS_MACINTOSH )
  /* Mac OS X/Darwin kernel often changes recommended method to access */
  /* the resource fork and older methods makes the kernel issue the    */
  /* warning of deprecated method.  To calm it down, the methods based */
  /* on Darwin VFS should be grouped and skip the rest methods after   */
  /* the case the resource is opened but found to lack a font in it.   */
  FT_TS_LOCAL( FT_TS_Bool )
  ft_raccess_rule_by_darwin_vfs( FT_TS_Library library, FT_TS_UInt  rule_index );
#endif

#endif /* FT_TS_CONFIG_OPTION_MAC_FONTS */


FT_TS_END_HEADER

#endif /* FTBASE_H_ */


/* END */
