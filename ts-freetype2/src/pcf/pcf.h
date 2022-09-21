/*  pcf.h

  FreeType font driver for pcf fonts

  Copyright (C) 2000, 2001, 2002, 2003, 2006, 2010 by
  Francesco Zappa Nardelli

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#ifndef PCF_H_
#define PCF_H_


#include <freetype/internal/ftdrv.h>
#include <freetype/internal/ftstream.h>


FT_TS_BEGIN_HEADER

  typedef struct  PCF_TableRec_
  {
    FT_TS_ULong  type;
    FT_TS_ULong  format;
    FT_TS_ULong  size;
    FT_TS_ULong  offset;

  } PCF_TableRec, *PCF_Table;


  typedef struct  PCF_TocRec_
  {
    FT_TS_ULong   version;
    FT_TS_ULong   count;
    PCF_Table  tables;

  } PCF_TocRec, *PCF_Toc;


  typedef struct  PCF_ParsePropertyRec_
  {
    FT_TS_Long  name;
    FT_TS_Byte  isString;
    FT_TS_Long  value;

  } PCF_ParsePropertyRec, *PCF_ParseProperty;


  typedef struct  PCF_PropertyRec_
  {
    FT_TS_String*  name;
    FT_TS_Byte     isString;

    union
    {
      FT_TS_String*  atom;
      FT_TS_Long     l;
      FT_TS_ULong    ul;

    } value;

  } PCF_PropertyRec, *PCF_Property;


  typedef struct  PCF_Compressed_MetricRec_
  {
    FT_TS_Byte  leftSideBearing;
    FT_TS_Byte  rightSideBearing;
    FT_TS_Byte  characterWidth;
    FT_TS_Byte  ascent;
    FT_TS_Byte  descent;

  } PCF_Compressed_MetricRec, *PCF_Compressed_Metric;


  typedef struct  PCF_MetricRec_
  {
    FT_TS_Short  leftSideBearing;
    FT_TS_Short  rightSideBearing;
    FT_TS_Short  characterWidth;
    FT_TS_Short  ascent;
    FT_TS_Short  descent;
    FT_TS_Short  attributes;

    FT_TS_ULong  bits;  /* offset into the PCF_BITMAPS table */

  } PCF_MetricRec, *PCF_Metric;


  typedef struct  PCF_EncRec_
  {
    FT_TS_UShort   firstCol;
    FT_TS_UShort   lastCol;
    FT_TS_UShort   firstRow;
    FT_TS_UShort   lastRow;
    FT_TS_UShort   defaultChar;

    FT_TS_UShort*  offset;

  } PCF_EncRec, *PCF_Enc;


  typedef struct  PCF_AccelRec_
  {
    FT_TS_Byte        noOverlap;
    FT_TS_Byte        constantMetrics;
    FT_TS_Byte        terminalFont;
    FT_TS_Byte        constantWidth;
    FT_TS_Byte        inkInside;
    FT_TS_Byte        inkMetrics;
    FT_TS_Byte        drawDirection;
    FT_TS_Long        fontAscent;
    FT_TS_Long        fontDescent;
    FT_TS_Long        maxOverlap;
    PCF_MetricRec  minbounds;
    PCF_MetricRec  maxbounds;
    PCF_MetricRec  ink_minbounds;
    PCF_MetricRec  ink_maxbounds;

  } PCF_AccelRec, *PCF_Accel;


  /*
   * This file uses X11 terminology for PCF data; an `encoding' in X11 speak
   * is the same as a `character code' in FreeType speak.
   */
  typedef struct  PCF_FaceRec_
  {
    FT_TS_FaceRec    root;

    FT_TS_StreamRec  comp_stream;
    FT_TS_Stream     comp_source;

    char*         charset_encoding;
    char*         charset_registry;

    PCF_TocRec    toc;
    PCF_AccelRec  accel;

    int           nprops;
    PCF_Property  properties;

    FT_TS_ULong      nmetrics;
    PCF_Metric    metrics;

    PCF_EncRec    enc;

    FT_TS_ULong      bitmapsFormat;

  } PCF_FaceRec, *PCF_Face;


  typedef struct  PCF_DriverRec_
  {
    FT_TS_DriverRec  root;

    FT_TS_Bool  no_long_family_names;

  } PCF_DriverRec, *PCF_Driver;


  /* macros for pcf font format */

#define LSBFirst  0
#define MSBFirst  1

#define PCF_FILE_VERSION        ( ( 'p' << 24 ) | \
                                  ( 'c' << 16 ) | \
                                  ( 'f' <<  8 ) | 1 )
#define PCF_FORMAT_MASK         0xFFFFFF00UL

#define PCF_DEFAULT_FORMAT      0x00000000UL
#define PCF_INKBOUNDS           0x00000200UL
#define PCF_ACCEL_W_INKBOUNDS   0x00000100UL
#define PCF_COMPRESSED_METRICS  0x00000100UL

#define PCF_FORMAT_MATCH( a, b ) \
          ( ( (a) & PCF_FORMAT_MASK ) == ( (b) & PCF_FORMAT_MASK ) )

#define PCF_GLYPH_PAD_MASK  ( 3 << 0 )
#define PCF_BYTE_MASK       ( 1 << 2 )
#define PCF_BIT_MASK        ( 1 << 3 )
#define PCF_SCAN_UNIT_MASK  ( 3 << 4 )

#define PCF_BYTE_ORDER( f ) \
          ( ( (f) & PCF_BYTE_MASK ) ? MSBFirst : LSBFirst )
#define PCF_BIT_ORDER( f ) \
          ( ( (f) & PCF_BIT_MASK ) ? MSBFirst : LSBFirst )
#define PCF_GLYPH_PAD_INDEX( f ) \
          ( (f) & PCF_GLYPH_PAD_MASK )
#define PCF_GLYPH_PAD( f ) \
          ( 1 << PCF_GLYPH_PAD_INDEX( f ) )
#define PCF_SCAN_UNIT_INDEX( f ) \
          ( ( (f) & PCF_SCAN_UNIT_MASK ) >> 4 )
#define PCF_SCAN_UNIT( f ) \
          ( 1 << PCF_SCAN_UNIT_INDEX( f ) )
#define PCF_FORMAT_BITS( f )             \
          ( (f) & ( PCF_GLYPH_PAD_MASK | \
                    PCF_BYTE_MASK      | \
                    PCF_BIT_MASK       | \
                    PCF_SCAN_UNIT_MASK ) )

#define PCF_SIZE_TO_INDEX( s )  ( (s) == 4 ? 2 : (s) == 2 ? 1 : 0 )
#define PCF_INDEX_TO_SIZE( b )  ( 1 << b )

#define PCF_FORMAT( bit, byte, glyph, scan )          \
          ( ( PCF_SIZE_TO_INDEX( scan )      << 4 ) | \
            ( ( (bit)  == MSBFirst ? 1 : 0 ) << 3 ) | \
            ( ( (byte) == MSBFirst ? 1 : 0 ) << 2 ) | \
            ( PCF_SIZE_TO_INDEX( glyph )     << 0 ) )

#define PCF_PROPERTIES        ( 1 << 0 )
#define PCF_ACCELERATORS      ( 1 << 1 )
#define PCF_METRICS           ( 1 << 2 )
#define PCF_BITMAPS           ( 1 << 3 )
#define PCF_INK_METRICS       ( 1 << 4 )
#define PCF_BDF_ENCODINGS     ( 1 << 5 )
#define PCF_SWIDTHS           ( 1 << 6 )
#define PCF_GLYPH_NAMES       ( 1 << 7 )
#define PCF_BDF_ACCELERATORS  ( 1 << 8 )

#define GLYPHPADOPTIONS  4 /* I'm not sure about this */

  FT_TS_LOCAL( FT_TS_Error )
  pcf_load_font( FT_TS_Stream  stream,
                 PCF_Face   face,
                 FT_TS_Long    face_index );

FT_TS_END_HEADER

#endif /* PCF_H_ */


/* END */
