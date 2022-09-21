/****************************************************************************
 *
 * t1types.h
 *
 *   Basic Type1/Type2 type definitions and interface (specification
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


#ifndef T1TYPES_H_
#define T1TYPES_H_


#include <freetype/t1tables.h>
#include <freetype/internal/pshints.h>
#include <freetype/internal/ftserv.h>
#include <freetype/internal/fthash.h>
#include <freetype/internal/services/svpscmap.h>


FT_TS_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***              REQUIRED TYPE1/TYPE2 TABLES DEFINITIONS              ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /**************************************************************************
   *
   * @struct:
   *   T1_EncodingRec
   *
   * @description:
   *   A structure modeling a custom encoding.
   *
   * @fields:
   *   num_chars ::
   *     The number of character codes in the encoding.  Usually 256.
   *
   *   code_first ::
   *     The lowest valid character code in the encoding.
   *
   *   code_last ::
   *     The highest valid character code in the encoding + 1. When equal to
   *     code_first there are no valid character codes.
   *
   *   char_index ::
   *     An array of corresponding glyph indices.
   *
   *   char_name ::
   *     An array of corresponding glyph names.
   */
  typedef struct  T1_EncodingRecRec_
  {
    FT_TS_Int       num_chars;
    FT_TS_Int       code_first;
    FT_TS_Int       code_last;

    FT_TS_UShort*         char_index;
    const FT_TS_String**  char_name;

  } T1_EncodingRec, *T1_Encoding;


  /* used to hold extra data of PS_FontInfoRec that
   * cannot be stored in the publicly defined structure.
   *
   * Note these can't be blended with multiple-masters.
   */
  typedef struct  PS_FontExtraRec_
  {
    FT_TS_UShort  fs_type;

  } PS_FontExtraRec;


  typedef struct  T1_FontRec_
  {
    PS_FontInfoRec   font_info;         /* font info dictionary   */
    PS_FontExtraRec  font_extra;        /* font info extra fields */
    PS_PrivateRec    private_dict;      /* private dictionary     */
    FT_TS_String*       font_name;         /* top-level dictionary   */

    T1_EncodingType  encoding_type;
    T1_EncodingRec   encoding;

    FT_TS_Byte*         subrs_block;
    FT_TS_Byte*         charstrings_block;
    FT_TS_Byte*         glyph_names_block;

    FT_TS_Int           num_subrs;
    FT_TS_Byte**        subrs;
    FT_TS_UInt*         subrs_len;
    FT_TS_Hash          subrs_hash;

    FT_TS_Int           num_glyphs;
    FT_TS_String**      glyph_names;       /* array of glyph names       */
    FT_TS_Byte**        charstrings;       /* array of glyph charstrings */
    FT_TS_UInt*         charstrings_len;

    FT_TS_Byte          paint_type;
    FT_TS_Byte          font_type;
    FT_TS_Matrix        font_matrix;
    FT_TS_Vector        font_offset;
    FT_TS_BBox          font_bbox;
    FT_TS_Long          font_id;

    FT_TS_Fixed         stroke_width;

  } T1_FontRec, *T1_Font;


  typedef struct  CID_SubrsRec_
  {
    FT_TS_Int     num_subrs;
    FT_TS_Byte**  code;

  } CID_SubrsRec, *CID_Subrs;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                AFM FONT INFORMATION STRUCTURES                    ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct  AFM_TrackKernRec_
  {
    FT_TS_Int    degree;
    FT_TS_Fixed  min_ptsize;
    FT_TS_Fixed  min_kern;
    FT_TS_Fixed  max_ptsize;
    FT_TS_Fixed  max_kern;

  } AFM_TrackKernRec, *AFM_TrackKern;

  typedef struct  AFM_KernPairRec_
  {
    FT_TS_UInt  index1;
    FT_TS_UInt  index2;
    FT_TS_Int   x;
    FT_TS_Int   y;

  } AFM_KernPairRec, *AFM_KernPair;

  typedef struct  AFM_FontInfoRec_
  {
    FT_TS_Bool        IsCIDFont;
    FT_TS_BBox        FontBBox;
    FT_TS_Fixed       Ascender;
    FT_TS_Fixed       Descender;
    AFM_TrackKern  TrackKerns;   /* free if non-NULL */
    FT_TS_UInt        NumTrackKern;
    AFM_KernPair   KernPairs;    /* free if non-NULL */
    FT_TS_UInt        NumKernPair;

  } AFM_FontInfoRec, *AFM_FontInfo;


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /***                                                                   ***/
  /***                                                                   ***/
  /***                ORIGINAL T1_FACE CLASS DEFINITION                  ***/
  /***                                                                   ***/
  /***                                                                   ***/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  typedef struct T1_FaceRec_*   T1_Face;
  typedef struct CID_FaceRec_*  CID_Face;


  typedef struct  T1_FaceRec_
  {
    FT_TS_FaceRec      root;
    T1_FontRec      type1;
    const void*     psnames;
    const void*     psaux;
    const void*     afm_data;
    FT_TS_CharMapRec   charmaprecs[2];
    FT_TS_CharMap      charmaps[2];

    /* support for Multiple Masters fonts */
    PS_Blend        blend;

    /* undocumented, optional: indices of subroutines that express      */
    /* the NormalizeDesignVector and the ConvertDesignVector procedure, */
    /* respectively, as Type 2 charstrings; -1 if keywords not present  */
    FT_TS_Int           ndv_idx;
    FT_TS_Int           cdv_idx;

    /* undocumented, optional: has the same meaning as len_buildchar */
    /* for Type 2 fonts; manipulated by othersubrs 19, 24, and 25    */
    FT_TS_UInt          len_buildchar;
    FT_TS_Long*         buildchar;

    /* since version 2.1 - interface to PostScript hinter */
    const void*     pshinter;

  } T1_FaceRec;


  typedef struct  CID_FaceRec_
  {
    FT_TS_FaceRec       root;
    void*            psnames;
    void*            psaux;
    CID_FaceInfoRec  cid;
    PS_FontExtraRec  font_extra;
#if 0
    void*            afm_data;
#endif
    CID_Subrs        subrs;

    /* since version 2.1 - interface to PostScript hinter */
    void*            pshinter;

    /* since version 2.1.8, but was originally positioned after `afm_data' */
    FT_TS_Byte*         binary_data; /* used if hex data has been converted */
    FT_TS_Stream        cid_stream;

  } CID_FaceRec;


FT_TS_END_HEADER

#endif /* T1TYPES_H_ */


/* END */
