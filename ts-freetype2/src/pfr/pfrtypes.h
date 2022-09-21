/****************************************************************************
 *
 * pfrtypes.h
 *
 *   FreeType PFR data structures (specification only).
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


#ifndef PFRTYPES_H_
#define PFRTYPES_H_

#include <freetype/internal/ftobjs.h>

FT_TS_BEGIN_HEADER

  /************************************************************************/

  /* the PFR Header structure */
  typedef struct  PFR_HeaderRec_
  {
    FT_TS_UInt32  signature;
    FT_TS_UInt    version;
    FT_TS_UInt    signature2;
    FT_TS_UInt    header_size;

    FT_TS_UInt    log_dir_size;
    FT_TS_UInt    log_dir_offset;

    FT_TS_UInt    log_font_max_size;
    FT_TS_UInt32  log_font_section_size;
    FT_TS_UInt32  log_font_section_offset;

    FT_TS_UInt32  phy_font_max_size;
    FT_TS_UInt32  phy_font_section_size;
    FT_TS_UInt32  phy_font_section_offset;

    FT_TS_UInt    gps_max_size;
    FT_TS_UInt32  gps_section_size;
    FT_TS_UInt32  gps_section_offset;

    FT_TS_UInt    max_blue_values;
    FT_TS_UInt    max_x_orus;
    FT_TS_UInt    max_y_orus;

    FT_TS_UInt    phy_font_max_size_high;
    FT_TS_UInt    color_flags;

    FT_TS_UInt32  bct_max_size;
    FT_TS_UInt32  bct_set_max_size;
    FT_TS_UInt32  phy_bct_set_max_size;

    FT_TS_UInt    num_phy_fonts;
    FT_TS_UInt    max_vert_stem_snap;
    FT_TS_UInt    max_horz_stem_snap;
    FT_TS_UInt    max_chars;

  } PFR_HeaderRec, *PFR_Header;


  /* used in `color_flags' field of the PFR_Header */
#define PFR_FLAG_BLACK_PIXEL    0x01U
#define PFR_FLAG_INVERT_BITMAP  0x02U


  /************************************************************************/

  typedef struct  PFR_LogFontRec_
  {
    FT_TS_UInt32  size;
    FT_TS_UInt32  offset;

    FT_TS_Int32   matrix[4];
    FT_TS_UInt    stroke_flags;
    FT_TS_Int     stroke_thickness;
    FT_TS_Int     bold_thickness;
    FT_TS_Int32   miter_limit;

    FT_TS_UInt32  phys_size;
    FT_TS_UInt32  phys_offset;

  } PFR_LogFontRec, *PFR_LogFont;


#define PFR_LINE_JOIN_MITER   0x00U
#define PFR_LINE_JOIN_ROUND   0x01U
#define PFR_LINE_JOIN_BEVEL   0x02U
#define PFR_LINE_JOIN_MASK    ( PFR_LINE_JOIN_ROUND | PFR_LINE_JOIN_BEVEL )

#define PFR_LOG_STROKE        0x04U
#define PFR_LOG_2BYTE_STROKE  0x08U
#define PFR_LOG_BOLD          0x10U
#define PFR_LOG_2BYTE_BOLD    0x20U
#define PFR_LOG_EXTRA_ITEMS   0x40U


  /************************************************************************/

#define PFR_BITMAP_2BYTE_CHARCODE  0x01U
#define PFR_BITMAP_2BYTE_SIZE      0x02U
#define PFR_BITMAP_3BYTE_OFFSET    0x04U

  /* not part of the specification but used for implementation */
#define PFR_BITMAP_CHARCODES_VALIDATED  0x40U
#define PFR_BITMAP_VALID_CHARCODES      0x80U


  typedef struct  PFR_BitmapCharRec_
  {
    FT_TS_UInt    char_code;
    FT_TS_UInt    gps_size;
    FT_TS_UInt32  gps_offset;

  } PFR_BitmapCharRec, *PFR_BitmapChar;


#define PFR_STRIKE_2BYTE_XPPM    0x01U
#define PFR_STRIKE_2BYTE_YPPM    0x02U
#define PFR_STRIKE_3BYTE_SIZE    0x04U
#define PFR_STRIKE_3BYTE_OFFSET  0x08U
#define PFR_STRIKE_2BYTE_COUNT   0x10U


  typedef struct  PFR_StrikeRec_
  {
    FT_TS_UInt         x_ppm;
    FT_TS_UInt         y_ppm;
    FT_TS_UInt         flags;

    FT_TS_UInt32       gps_size;
    FT_TS_UInt32       gps_offset;

    FT_TS_UInt32       bct_size;
    FT_TS_UInt32       bct_offset;

    /* optional */
    FT_TS_UInt         num_bitmaps;
    PFR_BitmapChar  bitmaps;

  } PFR_StrikeRec, *PFR_Strike;


  /************************************************************************/

  typedef struct  PFR_CharRec_
  {
    FT_TS_UInt    char_code;
    FT_TS_Int     advance;
    FT_TS_UInt    gps_size;
    FT_TS_UInt32  gps_offset;

  } PFR_CharRec, *PFR_Char;


  /************************************************************************/

  typedef struct  PFR_DimensionRec_
  {
    FT_TS_UInt  standard;
    FT_TS_UInt  num_stem_snaps;
    FT_TS_Int*  stem_snaps;

  } PFR_DimensionRec, *PFR_Dimension;

  /************************************************************************/

  typedef struct PFR_KernItemRec_*  PFR_KernItem;

  typedef struct  PFR_KernItemRec_
  {
    PFR_KernItem  next;
    FT_TS_Byte       pair_count;
    FT_TS_Byte       flags;
    FT_TS_Short      base_adj;
    FT_TS_UInt       pair_size;
    FT_TS_Offset     offset;
    FT_TS_UInt32     pair1;
    FT_TS_UInt32     pair2;

  } PFR_KernItemRec;


#define PFR_KERN_INDEX( g1, g2 )                          \
          ( ( (FT_TS_UInt32)(g1) << 16 ) | (FT_TS_UInt16)(g2) )

#define PFR_KERN_PAIR_INDEX( pair )                        \
          PFR_KERN_INDEX( (pair)->glyph1, (pair)->glyph2 )

#define PFR_NEXT_KPAIR( p )  ( p += 2,                              \
                               ( (FT_TS_UInt32)p[-2] << 16 ) | p[-1] )


  /************************************************************************/

  typedef struct  PFR_PhyFontRec_
  {
    FT_TS_Memory          memory;
    FT_TS_UInt32          offset;

    FT_TS_UInt            font_ref_number;
    FT_TS_UInt            outline_resolution;
    FT_TS_UInt            metrics_resolution;
    FT_TS_BBox            bbox;
    FT_TS_UInt            flags;
    FT_TS_Int             standard_advance;

    FT_TS_Int             ascent;   /* optional, bbox.yMax if not present */
    FT_TS_Int             descent;  /* optional, bbox.yMin if not present */
    FT_TS_Int             leading;  /* optional, 0 if not present         */

    PFR_DimensionRec   horizontal;
    PFR_DimensionRec   vertical;

    FT_TS_String*         font_id;
    FT_TS_String*         family_name;
    FT_TS_String*         style_name;

    FT_TS_UInt            num_strikes;
    FT_TS_UInt            max_strikes;
    PFR_StrikeRec*     strikes;

    FT_TS_UInt            num_blue_values;
    FT_TS_Int            *blue_values;
    FT_TS_UInt            blue_fuzz;
    FT_TS_UInt            blue_scale;

    FT_TS_UInt            num_chars;
    FT_TS_Offset          chars_offset;
    PFR_Char           chars;

    FT_TS_UInt            num_kern_pairs;
    PFR_KernItem       kern_items;
    PFR_KernItem*      kern_items_tail;

    /* not part of the spec, but used during load */
    FT_TS_ULong           bct_offset;
    FT_TS_Byte*           cursor;

  } PFR_PhyFontRec, *PFR_PhyFont;


#define PFR_PHY_VERTICAL          0x01U
#define PFR_PHY_2BYTE_CHARCODE    0x02U
#define PFR_PHY_PROPORTIONAL      0x04U
#define PFR_PHY_ASCII_CODE        0x08U
#define PFR_PHY_2BYTE_GPS_SIZE    0x10U
#define PFR_PHY_3BYTE_GPS_OFFSET  0x20U
#define PFR_PHY_EXTRA_ITEMS       0x80U


#define PFR_KERN_2BYTE_CHAR  0x01U
#define PFR_KERN_2BYTE_ADJ   0x02U


  /************************************************************************/

#define PFR_GLYPH_YCOUNT         0x01U
#define PFR_GLYPH_XCOUNT         0x02U
#define PFR_GLYPH_1BYTE_XYCOUNT  0x04U

#define PFR_GLYPH_SINGLE_EXTRA_ITEMS    0x08U
#define PFR_GLYPH_COMPOUND_EXTRA_ITEMS  0x40U

#define PFR_GLYPH_IS_COMPOUND  0x80U


  /* controlled coordinate */
  typedef struct  PFR_CoordRec_
  {
    FT_TS_UInt  org;
    FT_TS_UInt  cur;

  } PFR_CoordRec, *PFR_Coord;


  typedef struct  PFR_SubGlyphRec_
  {
    FT_TS_Fixed   x_scale;
    FT_TS_Fixed   y_scale;
    FT_TS_Int     x_delta;
    FT_TS_Int     y_delta;
    FT_TS_UInt32  gps_offset;
    FT_TS_UInt    gps_size;

  } PFR_SubGlyphRec, *PFR_SubGlyph;


#define PFR_SUBGLYPH_XSCALE        0x10U
#define PFR_SUBGLYPH_YSCALE        0x20U
#define PFR_SUBGLYPH_2BYTE_SIZE    0x40U
#define PFR_SUBGLYPH_3BYTE_OFFSET  0x80U


  typedef struct  PFR_GlyphRec_
  {
    FT_TS_Byte           format;

#if 0
    FT_TS_UInt           num_x_control;
    FT_TS_UInt           num_y_control;
#endif
    FT_TS_UInt           max_xy_control;
    FT_TS_Pos*           x_control;
    FT_TS_Pos*           y_control;


    FT_TS_UInt           num_subs;
    FT_TS_UInt           max_subs;
    PFR_SubGlyphRec*  subs;

    FT_TS_GlyphLoader    loader;
    FT_TS_Bool           path_begun;

  } PFR_GlyphRec, *PFR_Glyph;


FT_TS_END_HEADER

#endif /* PFRTYPES_H_ */


/* END */
