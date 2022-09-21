/****************************************************************************
 *
 * tttables.h
 *
 *   Basic SFNT/TrueType tables definitions and interface
 *   (specification only).
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


#ifndef TTTABLES_H_
#define TTTABLES_H_


#include <freetype/freetype.h>

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_TS_BEGIN_HEADER

  /**************************************************************************
   *
   * @section:
   *   truetype_tables
   *
   * @title:
   *   TrueType Tables
   *
   * @abstract:
   *   TrueType-specific table types and functions.
   *
   * @description:
   *   This section contains definitions of some basic tables specific to
   *   TrueType and OpenType as well as some routines used to access and
   *   process them.
   *
   * @order:
   *   TT_Header
   *   TT_HoriHeader
   *   TT_VertHeader
   *   TT_OS2
   *   TT_Postscript
   *   TT_PCLT
   *   TT_MaxProfile
   *
   *   FT_TS_Sfnt_Tag
   *   FT_TS_Get_Sfnt_Table
   *   FT_TS_Load_Sfnt_Table
   *   FT_TS_Sfnt_Table_Info
   *
   *   FT_TS_Get_CMap_Language_ID
   *   FT_TS_Get_CMap_Format
   *
   *   FT_TS_PARAM_TAG_UNPATENTED_HINTING
   *
   */


  /**************************************************************************
   *
   * @struct:
   *   TT_Header
   *
   * @description:
   *   A structure to model a TrueType font header table.  All fields follow
   *   the OpenType specification.  The 64-bit timestamps are stored in
   *   two-element arrays `Created` and `Modified`, first the upper then
   *   the lower 32~bits.
   */
  typedef struct  TT_Header_
  {
    FT_TS_Fixed   Table_Version;
    FT_TS_Fixed   Font_Revision;

    FT_TS_Long    CheckSum_Adjust;
    FT_TS_Long    Magic_Number;

    FT_TS_UShort  Flags;
    FT_TS_UShort  Units_Per_EM;

    FT_TS_ULong   Created [2];
    FT_TS_ULong   Modified[2];

    FT_TS_Short   xMin;
    FT_TS_Short   yMin;
    FT_TS_Short   xMax;
    FT_TS_Short   yMax;

    FT_TS_UShort  Mac_Style;
    FT_TS_UShort  Lowest_Rec_PPEM;

    FT_TS_Short   Font_Direction;
    FT_TS_Short   Index_To_Loc_Format;
    FT_TS_Short   Glyph_Data_Format;

  } TT_Header;


  /**************************************************************************
   *
   * @struct:
   *   TT_HoriHeader
   *
   * @description:
   *   A structure to model a TrueType horizontal header, the 'hhea' table,
   *   as well as the corresponding horizontal metrics table, 'hmtx'.
   *
   * @fields:
   *   Version ::
   *     The table version.
   *
   *   Ascender ::
   *     The font's ascender, i.e., the distance from the baseline to the
   *     top-most of all glyph points found in the font.
   *
   *     This value is invalid in many fonts, as it is usually set by the
   *     font designer, and often reflects only a portion of the glyphs found
   *     in the font (maybe ASCII).
   *
   *     You should use the `sTypoAscender` field of the 'OS/2' table instead
   *     if you want the correct one.
   *
   *   Descender ::
   *     The font's descender, i.e., the distance from the baseline to the
   *     bottom-most of all glyph points found in the font.  It is negative.
   *
   *     This value is invalid in many fonts, as it is usually set by the
   *     font designer, and often reflects only a portion of the glyphs found
   *     in the font (maybe ASCII).
   *
   *     You should use the `sTypoDescender` field of the 'OS/2' table
   *     instead if you want the correct one.
   *
   *   Line_Gap ::
   *     The font's line gap, i.e., the distance to add to the ascender and
   *     descender to get the BTB, i.e., the baseline-to-baseline distance
   *     for the font.
   *
   *   advance_Width_Max ::
   *     This field is the maximum of all advance widths found in the font.
   *     It can be used to compute the maximum width of an arbitrary string
   *     of text.
   *
   *   min_Left_Side_Bearing ::
   *     The minimum left side bearing of all glyphs within the font.
   *
   *   min_Right_Side_Bearing ::
   *     The minimum right side bearing of all glyphs within the font.
   *
   *   xMax_Extent ::
   *     The maximum horizontal extent (i.e., the 'width' of a glyph's
   *     bounding box) for all glyphs in the font.
   *
   *   caret_Slope_Rise ::
   *     The rise coefficient of the cursor's slope of the cursor
   *     (slope=rise/run).
   *
   *   caret_Slope_Run ::
   *     The run coefficient of the cursor's slope.
   *
   *   caret_Offset ::
   *     The cursor's offset for slanted fonts.
   *
   *   Reserved ::
   *     8~reserved bytes.
   *
   *   metric_Data_Format ::
   *     Always~0.
   *
   *   number_Of_HMetrics ::
   *     Number of HMetrics entries in the 'hmtx' table -- this value can be
   *     smaller than the total number of glyphs in the font.
   *
   *   long_metrics ::
   *     A pointer into the 'hmtx' table.
   *
   *   short_metrics ::
   *     A pointer into the 'hmtx' table.
   *
   * @note:
   *   For an OpenType variation font, the values of the following fields can
   *   change after a call to @FT_TS_Set_Var_Design_Coordinates (and friends) if
   *   the font contains an 'MVAR' table: `caret_Slope_Rise`,
   *   `caret_Slope_Run`, and `caret_Offset`.
   */
  typedef struct  TT_HoriHeader_
  {
    FT_TS_Fixed   Version;
    FT_TS_Short   Ascender;
    FT_TS_Short   Descender;
    FT_TS_Short   Line_Gap;

    FT_TS_UShort  advance_Width_Max;      /* advance width maximum */

    FT_TS_Short   min_Left_Side_Bearing;  /* minimum left-sb       */
    FT_TS_Short   min_Right_Side_Bearing; /* minimum right-sb      */
    FT_TS_Short   xMax_Extent;            /* xmax extents          */
    FT_TS_Short   caret_Slope_Rise;
    FT_TS_Short   caret_Slope_Run;
    FT_TS_Short   caret_Offset;

    FT_TS_Short   Reserved[4];

    FT_TS_Short   metric_Data_Format;
    FT_TS_UShort  number_Of_HMetrics;

    /* The following fields are not defined by the OpenType specification */
    /* but they are used to connect the metrics header to the relevant    */
    /* 'hmtx' table.                                                      */

    void*      long_metrics;
    void*      short_metrics;

  } TT_HoriHeader;


  /**************************************************************************
   *
   * @struct:
   *   TT_VertHeader
   *
   * @description:
   *   A structure used to model a TrueType vertical header, the 'vhea'
   *   table, as well as the corresponding vertical metrics table, 'vmtx'.
   *
   * @fields:
   *   Version ::
   *     The table version.
   *
   *   Ascender ::
   *     The font's ascender, i.e., the distance from the baseline to the
   *     top-most of all glyph points found in the font.
   *
   *     This value is invalid in many fonts, as it is usually set by the
   *     font designer, and often reflects only a portion of the glyphs found
   *     in the font (maybe ASCII).
   *
   *     You should use the `sTypoAscender` field of the 'OS/2' table instead
   *     if you want the correct one.
   *
   *   Descender ::
   *     The font's descender, i.e., the distance from the baseline to the
   *     bottom-most of all glyph points found in the font.  It is negative.
   *
   *     This value is invalid in many fonts, as it is usually set by the
   *     font designer, and often reflects only a portion of the glyphs found
   *     in the font (maybe ASCII).
   *
   *     You should use the `sTypoDescender` field of the 'OS/2' table
   *     instead if you want the correct one.
   *
   *   Line_Gap ::
   *     The font's line gap, i.e., the distance to add to the ascender and
   *     descender to get the BTB, i.e., the baseline-to-baseline distance
   *     for the font.
   *
   *   advance_Height_Max ::
   *     This field is the maximum of all advance heights found in the font.
   *     It can be used to compute the maximum height of an arbitrary string
   *     of text.
   *
   *   min_Top_Side_Bearing ::
   *     The minimum top side bearing of all glyphs within the font.
   *
   *   min_Bottom_Side_Bearing ::
   *     The minimum bottom side bearing of all glyphs within the font.
   *
   *   yMax_Extent ::
   *     The maximum vertical extent (i.e., the 'height' of a glyph's
   *     bounding box) for all glyphs in the font.
   *
   *   caret_Slope_Rise ::
   *     The rise coefficient of the cursor's slope of the cursor
   *     (slope=rise/run).
   *
   *   caret_Slope_Run ::
   *     The run coefficient of the cursor's slope.
   *
   *   caret_Offset ::
   *     The cursor's offset for slanted fonts.
   *
   *   Reserved ::
   *     8~reserved bytes.
   *
   *   metric_Data_Format ::
   *     Always~0.
   *
   *   number_Of_VMetrics ::
   *     Number of VMetrics entries in the 'vmtx' table -- this value can be
   *     smaller than the total number of glyphs in the font.
   *
   *   long_metrics ::
   *     A pointer into the 'vmtx' table.
   *
   *   short_metrics ::
   *     A pointer into the 'vmtx' table.
   *
   * @note:
   *   For an OpenType variation font, the values of the following fields can
   *   change after a call to @FT_TS_Set_Var_Design_Coordinates (and friends) if
   *   the font contains an 'MVAR' table: `Ascender`, `Descender`,
   *   `Line_Gap`, `caret_Slope_Rise`, `caret_Slope_Run`, and `caret_Offset`.
   */
  typedef struct  TT_VertHeader_
  {
    FT_TS_Fixed   Version;
    FT_TS_Short   Ascender;
    FT_TS_Short   Descender;
    FT_TS_Short   Line_Gap;

    FT_TS_UShort  advance_Height_Max;      /* advance height maximum */

    FT_TS_Short   min_Top_Side_Bearing;    /* minimum top-sb          */
    FT_TS_Short   min_Bottom_Side_Bearing; /* minimum bottom-sb       */
    FT_TS_Short   yMax_Extent;             /* ymax extents            */
    FT_TS_Short   caret_Slope_Rise;
    FT_TS_Short   caret_Slope_Run;
    FT_TS_Short   caret_Offset;

    FT_TS_Short   Reserved[4];

    FT_TS_Short   metric_Data_Format;
    FT_TS_UShort  number_Of_VMetrics;

    /* The following fields are not defined by the OpenType specification */
    /* but they are used to connect the metrics header to the relevant    */
    /* 'vmtx' table.                                                      */

    void*      long_metrics;
    void*      short_metrics;

  } TT_VertHeader;


  /**************************************************************************
   *
   * @struct:
   *   TT_OS2
   *
   * @description:
   *   A structure to model a TrueType 'OS/2' table.  All fields comply to
   *   the OpenType specification.
   *
   *   Note that we now support old Mac fonts that do not include an 'OS/2'
   *   table.  In this case, the `version` field is always set to 0xFFFF.
   *
   * @note:
   *   For an OpenType variation font, the values of the following fields can
   *   change after a call to @FT_TS_Set_Var_Design_Coordinates (and friends) if
   *   the font contains an 'MVAR' table: `sCapHeight`, `sTypoAscender`,
   *   `sTypoDescender`, `sTypoLineGap`, `sxHeight`, `usWinAscent`,
   *   `usWinDescent`, `yStrikeoutPosition`, `yStrikeoutSize`,
   *   `ySubscriptXOffset`, `ySubScriptXSize`, `ySubscriptYOffset`,
   *   `ySubscriptYSize`, `ySuperscriptXOffset`, `ySuperscriptXSize`,
   *   `ySuperscriptYOffset`, and `ySuperscriptYSize`.
   *
   *   Possible values for bits in the `ulUnicodeRangeX` fields are given by
   *   the @TT_UCR_XXX macros.
   */

  typedef struct  TT_OS2_
  {
    FT_TS_UShort  version;                /* 0x0001 - more or 0xFFFF */
    FT_TS_Short   xAvgCharWidth;
    FT_TS_UShort  usWeightClass;
    FT_TS_UShort  usWidthClass;
    FT_TS_UShort  fsType;
    FT_TS_Short   ySubscriptXSize;
    FT_TS_Short   ySubscriptYSize;
    FT_TS_Short   ySubscriptXOffset;
    FT_TS_Short   ySubscriptYOffset;
    FT_TS_Short   ySuperscriptXSize;
    FT_TS_Short   ySuperscriptYSize;
    FT_TS_Short   ySuperscriptXOffset;
    FT_TS_Short   ySuperscriptYOffset;
    FT_TS_Short   yStrikeoutSize;
    FT_TS_Short   yStrikeoutPosition;
    FT_TS_Short   sFamilyClass;

    FT_TS_Byte    panose[10];

    FT_TS_ULong   ulUnicodeRange1;        /* Bits 0-31   */
    FT_TS_ULong   ulUnicodeRange2;        /* Bits 32-63  */
    FT_TS_ULong   ulUnicodeRange3;        /* Bits 64-95  */
    FT_TS_ULong   ulUnicodeRange4;        /* Bits 96-127 */

    FT_TS_Char    achVendID[4];

    FT_TS_UShort  fsSelection;
    FT_TS_UShort  usFirstCharIndex;
    FT_TS_UShort  usLastCharIndex;
    FT_TS_Short   sTypoAscender;
    FT_TS_Short   sTypoDescender;
    FT_TS_Short   sTypoLineGap;
    FT_TS_UShort  usWinAscent;
    FT_TS_UShort  usWinDescent;

    /* only version 1 and higher: */

    FT_TS_ULong   ulCodePageRange1;       /* Bits 0-31   */
    FT_TS_ULong   ulCodePageRange2;       /* Bits 32-63  */

    /* only version 2 and higher: */

    FT_TS_Short   sxHeight;
    FT_TS_Short   sCapHeight;
    FT_TS_UShort  usDefaultChar;
    FT_TS_UShort  usBreakChar;
    FT_TS_UShort  usMaxContext;

    /* only version 5 and higher: */

    FT_TS_UShort  usLowerOpticalPointSize;       /* in twips (1/20th points) */
    FT_TS_UShort  usUpperOpticalPointSize;       /* in twips (1/20th points) */

  } TT_OS2;


  /**************************************************************************
   *
   * @struct:
   *   TT_Postscript
   *
   * @description:
   *   A structure to model a TrueType 'post' table.  All fields comply to
   *   the OpenType specification.  This structure does not reference a
   *   font's PostScript glyph names; use @FT_TS_Get_Glyph_Name to retrieve
   *   them.
   *
   * @note:
   *   For an OpenType variation font, the values of the following fields can
   *   change after a call to @FT_TS_Set_Var_Design_Coordinates (and friends) if
   *   the font contains an 'MVAR' table: `underlinePosition` and
   *   `underlineThickness`.
   */
  typedef struct  TT_Postscript_
  {
    FT_TS_Fixed  FormatType;
    FT_TS_Fixed  italicAngle;
    FT_TS_Short  underlinePosition;
    FT_TS_Short  underlineThickness;
    FT_TS_ULong  isFixedPitch;
    FT_TS_ULong  minMemType42;
    FT_TS_ULong  maxMemType42;
    FT_TS_ULong  minMemType1;
    FT_TS_ULong  maxMemType1;

    /* Glyph names follow in the 'post' table, but we don't */
    /* load them by default.                                */

  } TT_Postscript;


  /**************************************************************************
   *
   * @struct:
   *   TT_PCLT
   *
   * @description:
   *   A structure to model a TrueType 'PCLT' table.  All fields comply to
   *   the OpenType specification.
   */
  typedef struct  TT_PCLT_
  {
    FT_TS_Fixed   Version;
    FT_TS_ULong   FontNumber;
    FT_TS_UShort  Pitch;
    FT_TS_UShort  xHeight;
    FT_TS_UShort  Style;
    FT_TS_UShort  TypeFamily;
    FT_TS_UShort  CapHeight;
    FT_TS_UShort  SymbolSet;
    FT_TS_Char    TypeFace[16];
    FT_TS_Char    CharacterComplement[8];
    FT_TS_Char    FileName[6];
    FT_TS_Char    StrokeWeight;
    FT_TS_Char    WidthType;
    FT_TS_Byte    SerifStyle;
    FT_TS_Byte    Reserved;

  } TT_PCLT;


  /**************************************************************************
   *
   * @struct:
   *   TT_MaxProfile
   *
   * @description:
   *   The maximum profile ('maxp') table contains many max values, which can
   *   be used to pre-allocate arrays for speeding up glyph loading and
   *   hinting.
   *
   * @fields:
   *   version ::
   *     The version number.
   *
   *   numGlyphs ::
   *     The number of glyphs in this TrueType font.
   *
   *   maxPoints ::
   *     The maximum number of points in a non-composite TrueType glyph.  See
   *     also `maxCompositePoints`.
   *
   *   maxContours ::
   *     The maximum number of contours in a non-composite TrueType glyph.
   *     See also `maxCompositeContours`.
   *
   *   maxCompositePoints ::
   *     The maximum number of points in a composite TrueType glyph.  See
   *     also `maxPoints`.
   *
   *   maxCompositeContours ::
   *     The maximum number of contours in a composite TrueType glyph.  See
   *     also `maxContours`.
   *
   *   maxZones ::
   *     The maximum number of zones used for glyph hinting.
   *
   *   maxTwilightPoints ::
   *     The maximum number of points in the twilight zone used for glyph
   *     hinting.
   *
   *   maxStorage ::
   *     The maximum number of elements in the storage area used for glyph
   *     hinting.
   *
   *   maxFunctionDefs ::
   *     The maximum number of function definitions in the TrueType bytecode
   *     for this font.
   *
   *   maxInstructionDefs ::
   *     The maximum number of instruction definitions in the TrueType
   *     bytecode for this font.
   *
   *   maxStackElements ::
   *     The maximum number of stack elements used during bytecode
   *     interpretation.
   *
   *   maxSizeOfInstructions ::
   *     The maximum number of TrueType opcodes used for glyph hinting.
   *
   *   maxComponentElements ::
   *     The maximum number of simple (i.e., non-composite) glyphs in a
   *     composite glyph.
   *
   *   maxComponentDepth ::
   *     The maximum nesting depth of composite glyphs.
   *
   * @note:
   *   This structure is only used during font loading.
   */
  typedef struct  TT_MaxProfile_
  {
    FT_TS_Fixed   version;
    FT_TS_UShort  numGlyphs;
    FT_TS_UShort  maxPoints;
    FT_TS_UShort  maxContours;
    FT_TS_UShort  maxCompositePoints;
    FT_TS_UShort  maxCompositeContours;
    FT_TS_UShort  maxZones;
    FT_TS_UShort  maxTwilightPoints;
    FT_TS_UShort  maxStorage;
    FT_TS_UShort  maxFunctionDefs;
    FT_TS_UShort  maxInstructionDefs;
    FT_TS_UShort  maxStackElements;
    FT_TS_UShort  maxSizeOfInstructions;
    FT_TS_UShort  maxComponentElements;
    FT_TS_UShort  maxComponentDepth;

  } TT_MaxProfile;


  /**************************************************************************
   *
   * @enum:
   *   FT_TS_Sfnt_Tag
   *
   * @description:
   *   An enumeration to specify indices of SFNT tables loaded and parsed by
   *   FreeType during initialization of an SFNT font.  Used in the
   *   @FT_TS_Get_Sfnt_Table API function.
   *
   * @values:
   *   FT_TS_SFNT_HEAD ::
   *     To access the font's @TT_Header structure.
   *
   *   FT_TS_SFNT_MAXP ::
   *     To access the font's @TT_MaxProfile structure.
   *
   *   FT_TS_SFNT_OS2 ::
   *     To access the font's @TT_OS2 structure.
   *
   *   FT_TS_SFNT_HHEA ::
   *     To access the font's @TT_HoriHeader structure.
   *
   *   FT_TS_SFNT_VHEA ::
   *     To access the font's @TT_VertHeader structure.
   *
   *   FT_TS_SFNT_POST ::
   *     To access the font's @TT_Postscript structure.
   *
   *   FT_TS_SFNT_PCLT ::
   *     To access the font's @TT_PCLT structure.
   */
  typedef enum  FT_TS_Sfnt_Tag_
  {
    FT_TS_SFNT_HEAD,
    FT_TS_SFNT_MAXP,
    FT_TS_SFNT_OS2,
    FT_TS_SFNT_HHEA,
    FT_TS_SFNT_VHEA,
    FT_TS_SFNT_POST,
    FT_TS_SFNT_PCLT,

    FT_TS_SFNT_MAX

  } FT_TS_Sfnt_Tag;

  /* these constants are deprecated; use the corresponding `FT_TS_Sfnt_Tag` */
  /* values instead                                                      */
#define ft_sfnt_head  FT_TS_SFNT_HEAD
#define ft_sfnt_maxp  FT_TS_SFNT_MAXP
#define ft_sfnt_os2   FT_TS_SFNT_OS2
#define ft_sfnt_hhea  FT_TS_SFNT_HHEA
#define ft_sfnt_vhea  FT_TS_SFNT_VHEA
#define ft_sfnt_post  FT_TS_SFNT_POST
#define ft_sfnt_pclt  FT_TS_SFNT_PCLT


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Get_Sfnt_Table
   *
   * @description:
   *   Return a pointer to a given SFNT table stored within a face.
   *
   * @input:
   *   face ::
   *     A handle to the source.
   *
   *   tag ::
   *     The index of the SFNT table.
   *
   * @return:
   *   A type-less pointer to the table.  This will be `NULL` in case of
   *   error, or if the corresponding table was not found **OR** loaded from
   *   the file.
   *
   *   Use a typecast according to `tag` to access the structure elements.
   *
   * @note:
   *   The table is owned by the face object and disappears with it.
   *
   *   This function is only useful to access SFNT tables that are loaded by
   *   the sfnt, truetype, and opentype drivers.  See @FT_TS_Sfnt_Tag for a
   *   list.
   *
   * @example:
   *   Here is an example demonstrating access to the 'vhea' table.
   *
   *   ```
   *     TT_VertHeader*  vert_header;
   *
   *
   *     vert_header =
   *       (TT_VertHeader*)FT_TS_Get_Sfnt_Table( face, FT_TS_SFNT_VHEA );
   *   ```
   */
  FT_TS_EXPORT( void* )
  FT_TS_Get_Sfnt_Table( FT_TS_Face      face,
                     FT_TS_Sfnt_Tag  tag );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Load_Sfnt_Table
   *
   * @description:
   *   Load any SFNT font table into client memory.
   *
   * @input:
   *   face ::
   *     A handle to the source face.
   *
   *   tag ::
   *     The four-byte tag of the table to load.  Use value~0 if you want to
   *     access the whole font file.  Otherwise, you can use one of the
   *     definitions found in the @FT_TS_TRUETYPE_TAGS_H file, or forge a new
   *     one with @FT_TS_MAKE_TAG.
   *
   *   offset ::
   *     The starting offset in the table (or file if tag~==~0).
   *
   * @output:
   *   buffer ::
   *     The target buffer address.  The client must ensure that the memory
   *     array is big enough to hold the data.
   *
   * @inout:
   *   length ::
   *     If the `length` parameter is `NULL`, try to load the whole table.
   *     Return an error code if it fails.
   *
   *     Else, if `*length` is~0, exit immediately while returning the
   *     table's (or file) full size in it.
   *
   *     Else the number of bytes to read from the table or file, from the
   *     starting offset.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   If you need to determine the table's length you should first call this
   *   function with `*length` set to~0, as in the following example:
   *
   *   ```
   *     FT_TS_ULong  length = 0;
   *
   *
   *     error = FT_TS_Load_Sfnt_Table( face, tag, 0, NULL, &length );
   *     if ( error ) { ... table does not exist ... }
   *
   *     buffer = malloc( length );
   *     if ( buffer == NULL ) { ... not enough memory ... }
   *
   *     error = FT_TS_Load_Sfnt_Table( face, tag, 0, buffer, &length );
   *     if ( error ) { ... could not load table ... }
   *   ```
   *
   *   Note that structures like @TT_Header or @TT_OS2 can't be used with
   *   this function; they are limited to @FT_TS_Get_Sfnt_Table.  Reason is that
   *   those structures depend on the processor architecture, with varying
   *   size (e.g. 32bit vs. 64bit) or order (big endian vs. little endian).
   *
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Load_Sfnt_Table( FT_TS_Face    face,
                      FT_TS_ULong   tag,
                      FT_TS_Long    offset,
                      FT_TS_Byte*   buffer,
                      FT_TS_ULong*  length );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Sfnt_Table_Info
   *
   * @description:
   *   Return information on an SFNT table.
   *
   * @input:
   *   face ::
   *     A handle to the source face.
   *
   *   table_index ::
   *     The index of an SFNT table.  The function returns
   *     FT_TS_Err_Table_Missing for an invalid value.
   *
   * @inout:
   *   tag ::
   *     The name tag of the SFNT table.  If the value is `NULL`,
   *     `table_index` is ignored, and `length` returns the number of SFNT
   *     tables in the font.
   *
   * @output:
   *   length ::
   *     The length of the SFNT table (or the number of SFNT tables,
   *     depending on `tag`).
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   While parsing fonts, FreeType handles SFNT tables with length zero as
   *   missing.
   *
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Sfnt_Table_Info( FT_TS_Face    face,
                      FT_TS_UInt    table_index,
                      FT_TS_ULong  *tag,
                      FT_TS_ULong  *length );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Get_CMap_Language_ID
   *
   * @description:
   *   Return cmap language ID as specified in the OpenType standard.
   *   Definitions of language ID values are in file @FT_TS_TRUETYPE_IDS_H.
   *
   * @input:
   *   charmap ::
   *     The target charmap.
   *
   * @return:
   *   The language ID of `charmap`.  If `charmap` doesn't belong to an SFNT
   *   face, just return~0 as the default value.
   *
   *   For a format~14 cmap (to access Unicode IVS), the return value is
   *   0xFFFFFFFF.
   */
  FT_TS_EXPORT( FT_TS_ULong )
  FT_TS_Get_CMap_Language_ID( FT_TS_CharMap  charmap );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Get_CMap_Format
   *
   * @description:
   *   Return the format of an SFNT 'cmap' table.
   *
   * @input:
   *   charmap ::
   *     The target charmap.
   *
   * @return:
   *   The format of `charmap`.  If `charmap` doesn't belong to an SFNT face,
   *   return -1.
   */
  FT_TS_EXPORT( FT_TS_Long )
  FT_TS_Get_CMap_Format( FT_TS_CharMap  charmap );

  /* */


FT_TS_END_HEADER

#endif /* TTTABLES_H_ */


/* END */
