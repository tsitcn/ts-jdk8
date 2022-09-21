/****************************************************************************
 *
 * cfftypes.h
 *
 *   Basic OpenType/CFF type definitions and interface (specification
 *   only).
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


#ifndef CFFTYPES_H_
#define CFFTYPES_H_


#include <freetype/freetype.h>
#include <freetype/t1tables.h>
#include <freetype/internal/ftserv.h>
#include <freetype/internal/services/svpscmap.h>
#include <freetype/internal/pshints.h>
#include <freetype/internal/t1types.h>


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   CFF_IndexRec
   *
   * @description:
   *   A structure used to model a CFF Index table.
   *
   * @fields:
   *   stream ::
   *     The source input stream.
   *
   *   start ::
   *     The position of the first index byte in the input stream.
   *
   *   count ::
   *     The number of elements in the index.
   *
   *   off_size ::
   *     The size in bytes of object offsets in index.
   *
   *   data_offset ::
   *     The position of first data byte in the index's bytes.
   *
   *   data_size ::
   *     The size of the data table in this index.
   *
   *   offsets ::
   *     A table of element offsets in the index.  Must be loaded explicitly.
   *
   *   bytes ::
   *     If the index is loaded in memory, its bytes.
   */
  typedef struct  CFF_IndexRec_
  {
    FT_TS_Stream  stream;
    FT_TS_ULong   start;
    FT_TS_UInt    hdr_size;
    FT_TS_UInt    count;
    FT_TS_Byte    off_size;
    FT_TS_ULong   data_offset;
    FT_TS_ULong   data_size;

    FT_TS_ULong*  offsets;
    FT_TS_Byte*   bytes;

  } CFF_IndexRec, *CFF_Index;


  typedef struct  CFF_EncodingRec_
  {
    FT_TS_UInt     format;
    FT_TS_ULong    offset;

    FT_TS_UInt     count;
    FT_TS_UShort   sids [256];  /* avoid dynamic allocations */
    FT_TS_UShort   codes[256];

  } CFF_EncodingRec, *CFF_Encoding;


  typedef struct  CFF_CharsetRec_
  {

    FT_TS_UInt     format;
    FT_TS_ULong    offset;

    FT_TS_UShort*  sids;
    FT_TS_UShort*  cids;       /* the inverse mapping of `sids'; only needed */
                            /* for CID-keyed fonts                        */
    FT_TS_UInt     max_cid;
    FT_TS_UInt     num_glyphs;

  } CFF_CharsetRec, *CFF_Charset;


  /* cf. similar fields in file `ttgxvar.h' from the `truetype' module */

  typedef struct  CFF_VarData_
  {
#if 0
    FT_TS_UInt  itemCount;       /* not used; always zero */
    FT_TS_UInt  shortDeltaCount; /* not used; always zero */
#endif

    FT_TS_UInt   regionIdxCount; /* number of region indexes           */
    FT_TS_UInt*  regionIndices;  /* array of `regionIdxCount' indices; */
                              /* these index `varRegionList'        */
  } CFF_VarData;


  /* contribution of one axis to a region */
  typedef struct  CFF_AxisCoords_
  {
    FT_TS_Fixed  startCoord;
    FT_TS_Fixed  peakCoord;      /* zero peak means no effect (factor = 1) */
    FT_TS_Fixed  endCoord;

  } CFF_AxisCoords;


  typedef struct  CFF_VarRegion_
  {
    CFF_AxisCoords*  axisList;      /* array of axisCount records */

  } CFF_VarRegion;


  typedef struct  CFF_VStoreRec_
  {
    FT_TS_UInt         dataCount;
    CFF_VarData*    varData;        /* array of dataCount records      */
                                    /* vsindex indexes this array      */
    FT_TS_UShort       axisCount;
    FT_TS_UInt         regionCount;    /* total number of regions defined */
    CFF_VarRegion*  varRegionList;

  } CFF_VStoreRec, *CFF_VStore;


  /* forward reference */
  typedef struct CFF_FontRec_*  CFF_Font;


  /* This object manages one cached blend vector.                  */
  /*                                                               */
  /* There is a BlendRec for Private DICT parsing in each subfont  */
  /* and a BlendRec for charstrings in CF2_Font instance data.     */
  /* A cached BV may be used across DICTs or Charstrings if inputs */
  /* have not changed.                                             */
  /*                                                               */
  /* `usedBV' is reset at the start of each parse or charstring.   */
  /* vsindex cannot be changed after a BV is used.                 */
  /*                                                               */
  /* Note: NDV is long (32/64 bit), while BV is 16.16 (FT_TS_Int32).  */
  typedef struct  CFF_BlendRec_
  {
    FT_TS_Bool    builtBV;        /* blendV has been built           */
    FT_TS_Bool    usedBV;         /* blendV has been used            */
    CFF_Font   font;           /* top level font struct           */
    FT_TS_UInt    lastVsindex;    /* last vsindex used               */
    FT_TS_UInt    lenNDV;         /* normDV length (aka numAxes)     */
    FT_TS_Fixed*  lastNDV;        /* last NDV used                   */
    FT_TS_UInt    lenBV;          /* BlendV length (aka numMasters)  */
    FT_TS_Int32*  BV;             /* current blendV (per DICT/glyph) */

  } CFF_BlendRec, *CFF_Blend;


  typedef struct  CFF_FontRecDictRec_
  {
    FT_TS_UInt    version;
    FT_TS_UInt    notice;
    FT_TS_UInt    copyright;
    FT_TS_UInt    full_name;
    FT_TS_UInt    family_name;
    FT_TS_UInt    weight;
    FT_TS_Bool    is_fixed_pitch;
    FT_TS_Fixed   italic_angle;
    FT_TS_Fixed   underline_position;
    FT_TS_Fixed   underline_thickness;
    FT_TS_Int     paint_type;
    FT_TS_Int     charstring_type;
    FT_TS_Matrix  font_matrix;
    FT_TS_Bool    has_font_matrix;
    FT_TS_ULong   units_per_em;  /* temporarily used as scaling value also */
    FT_TS_Vector  font_offset;
    FT_TS_ULong   unique_id;
    FT_TS_BBox    font_bbox;
    FT_TS_Pos     stroke_width;
    FT_TS_ULong   charset_offset;
    FT_TS_ULong   encoding_offset;
    FT_TS_ULong   charstrings_offset;
    FT_TS_ULong   private_offset;
    FT_TS_ULong   private_size;
    FT_TS_Long    synthetic_base;
    FT_TS_UInt    embedded_postscript;

    /* these should only be used for the top-level font dictionary */
    FT_TS_UInt    cid_registry;
    FT_TS_UInt    cid_ordering;
    FT_TS_Long    cid_supplement;

    FT_TS_Long    cid_font_version;
    FT_TS_Long    cid_font_revision;
    FT_TS_Long    cid_font_type;
    FT_TS_ULong   cid_count;
    FT_TS_ULong   cid_uid_base;
    FT_TS_ULong   cid_fd_array_offset;
    FT_TS_ULong   cid_fd_select_offset;
    FT_TS_UInt    cid_font_name;

    /* the next fields come from the data of the deprecated          */
    /* `MultipleMaster' operator; they are needed to parse the (also */
    /* deprecated) `blend' operator in Type 2 charstrings            */
    FT_TS_UShort  num_designs;
    FT_TS_UShort  num_axes;

    /* fields for CFF2 */
    FT_TS_ULong   vstore_offset;
    FT_TS_UInt    maxstack;

  } CFF_FontRecDictRec, *CFF_FontRecDict;


  /* forward reference */
  typedef struct CFF_SubFontRec_*  CFF_SubFont;


  typedef struct  CFF_PrivateRec_
  {
    FT_TS_Byte   num_blue_values;
    FT_TS_Byte   num_other_blues;
    FT_TS_Byte   num_family_blues;
    FT_TS_Byte   num_family_other_blues;

    FT_TS_Pos    blue_values[14];
    FT_TS_Pos    other_blues[10];
    FT_TS_Pos    family_blues[14];
    FT_TS_Pos    family_other_blues[10];

    FT_TS_Fixed  blue_scale;
    FT_TS_Pos    blue_shift;
    FT_TS_Pos    blue_fuzz;
    FT_TS_Pos    standard_width;
    FT_TS_Pos    standard_height;

    FT_TS_Byte   num_snap_widths;
    FT_TS_Byte   num_snap_heights;
    FT_TS_Pos    snap_widths[13];
    FT_TS_Pos    snap_heights[13];
    FT_TS_Bool   force_bold;
    FT_TS_Fixed  force_bold_threshold;
    FT_TS_Int    lenIV;
    FT_TS_Int    language_group;
    FT_TS_Fixed  expansion_factor;
    FT_TS_Long   initial_random_seed;
    FT_TS_ULong  local_subrs_offset;
    FT_TS_Pos    default_width;
    FT_TS_Pos    nominal_width;

    /* fields for CFF2 */
    FT_TS_UInt      vsindex;
    CFF_SubFont  subfont;

  } CFF_PrivateRec, *CFF_Private;


  typedef struct  CFF_FDSelectRec_
  {
    FT_TS_Byte   format;
    FT_TS_UInt   range_count;

    /* that's the table, taken from the file `as is' */
    FT_TS_Byte*  data;
    FT_TS_UInt   data_size;

    /* small cache for format 3 only */
    FT_TS_UInt   cache_first;
    FT_TS_UInt   cache_count;
    FT_TS_Byte   cache_fd;

  } CFF_FDSelectRec, *CFF_FDSelect;


  /* A SubFont packs a font dict and a private dict together.  They are */
  /* needed to support CID-keyed CFF fonts.                             */
  typedef struct  CFF_SubFontRec_
  {
    CFF_FontRecDictRec  font_dict;
    CFF_PrivateRec      private_dict;

    /* fields for CFF2 */
    CFF_BlendRec  blend;      /* current blend vector       */
    FT_TS_UInt       lenNDV;     /* current length NDV or zero */
    FT_TS_Fixed*     NDV;        /* ptr to current NDV or NULL */

    /* `blend_stack' is a writable buffer to hold blend results.          */
    /* This buffer is to the side of the normal cff parser stack;         */
    /* `cff_parse_blend' and `cff_blend_doBlend' push blend results here. */
    /* The normal stack then points to these values instead of the DICT   */
    /* because all other operators in Private DICT clear the stack.       */
    /* `blend_stack' could be cleared at each operator other than blend.  */
    /* Blended values are stored as 5-byte fixed point values.            */

    FT_TS_Byte*  blend_stack;    /* base of stack allocation     */
    FT_TS_Byte*  blend_top;      /* first empty slot             */
    FT_TS_UInt   blend_used;     /* number of bytes in use       */
    FT_TS_UInt   blend_alloc;    /* number of bytes allocated    */

    CFF_IndexRec  local_subrs_index;
    FT_TS_Byte**     local_subrs; /* array of pointers           */
                               /* into Local Subrs INDEX data */

    FT_TS_UInt32  random;

  } CFF_SubFontRec;


#define CFF_MAX_CID_FONTS  256


  typedef struct  CFF_FontRec_
  {
    FT_TS_Library       library;
    FT_TS_Stream        stream;
    FT_TS_Memory        memory;        /* TODO: take this from stream->memory? */
    FT_TS_ULong         base_offset;   /* offset to start of CFF */
    FT_TS_UInt          num_faces;
    FT_TS_UInt          num_glyphs;

    FT_TS_Byte          version_major;
    FT_TS_Byte          version_minor;
    FT_TS_Byte          header_size;

    FT_TS_UInt          top_dict_length;   /* cff2 only */

    FT_TS_Bool          cff2;

    CFF_IndexRec     name_index;
    CFF_IndexRec     top_dict_index;
    CFF_IndexRec     global_subrs_index;

    CFF_EncodingRec  encoding;
    CFF_CharsetRec   charset;

    CFF_IndexRec     charstrings_index;
    CFF_IndexRec     font_dict_index;
    CFF_IndexRec     private_index;
    CFF_IndexRec     local_subrs_index;

    FT_TS_String*       font_name;

    /* array of pointers into Global Subrs INDEX data */
    FT_TS_Byte**        global_subrs;

    /* array of pointers into String INDEX data stored at string_pool */
    FT_TS_UInt          num_strings;
    FT_TS_Byte**        strings;
    FT_TS_Byte*         string_pool;
    FT_TS_ULong         string_pool_size;

    CFF_SubFontRec   top_font;
    FT_TS_UInt          num_subfonts;
    CFF_SubFont      subfonts[CFF_MAX_CID_FONTS];

    CFF_FDSelectRec  fd_select;

    /* interface to PostScript hinter */
    PSHinter_Service  pshinter;

    /* interface to Postscript Names service */
    FT_TS_Service_PsCMaps  psnames;

    /* interface to CFFLoad service */
    const void*  cffload;

    /* since version 2.3.0 */
    PS_FontInfoRec*  font_info;   /* font info dictionary */

    /* since version 2.3.6 */
    FT_TS_String*       registry;
    FT_TS_String*       ordering;

    /* since version 2.4.12 */
    FT_TS_Generic       cf2_instance;

    /* since version 2.7.1 */
    CFF_VStoreRec    vstore;        /* parsed vstore structure */

    /* since version 2.9 */
    PS_FontExtraRec*  font_extra;

  } CFF_FontRec;


FT_TS_END_HEADER

#endif /* CFFTYPES_H_ */


/* END */
