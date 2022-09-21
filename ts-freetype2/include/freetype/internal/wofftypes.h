/****************************************************************************
 *
 * wofftypes.h
 *
 *   Basic WOFF/WOFF2 type definitions and interface (specification
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


#ifndef WOFFTYPES_H_
#define WOFFTYPES_H_


#include <freetype/tttables.h>
#include <freetype/internal/ftobjs.h>


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @struct:
   *   WOFF_HeaderRec
   *
   * @description:
   *   WOFF file format header.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF/#WOFFHeader
   */
  typedef struct  WOFF_HeaderRec_
  {
    FT_TS_ULong   signature;
    FT_TS_ULong   flavor;
    FT_TS_ULong   length;
    FT_TS_UShort  num_tables;
    FT_TS_UShort  reserved;
    FT_TS_ULong   totalSfntSize;
    FT_TS_UShort  majorVersion;
    FT_TS_UShort  minorVersion;
    FT_TS_ULong   metaOffset;
    FT_TS_ULong   metaLength;
    FT_TS_ULong   metaOrigLength;
    FT_TS_ULong   privOffset;
    FT_TS_ULong   privLength;

  } WOFF_HeaderRec, *WOFF_Header;


  /**************************************************************************
   *
   * @struct:
   *   WOFF_TableRec
   *
   * @description:
   *   This structure describes a given table of a WOFF font.
   *
   * @fields:
   *   Tag ::
   *     A four-bytes tag describing the table.
   *
   *   Offset ::
   *     The offset of the table from the start of the WOFF font in its
   *     resource.
   *
   *   CompLength ::
   *     Compressed table length (in bytes).
   *
   *   OrigLength ::
   *     Uncompressed table length (in bytes).
   *
   *   CheckSum ::
   *     The table checksum.  This value can be ignored.
   *
   *   OrigOffset ::
   *     The uncompressed table file offset.  This value gets computed while
   *     constructing the (uncompressed) SFNT header.  It is not contained in
   *     the WOFF file.
   */
  typedef struct  WOFF_TableRec_
  {
    FT_TS_Tag    Tag;           /* table ID                  */
    FT_TS_ULong  Offset;        /* table file offset         */
    FT_TS_ULong  CompLength;    /* compressed table length   */
    FT_TS_ULong  OrigLength;    /* uncompressed table length */
    FT_TS_ULong  CheckSum;      /* uncompressed checksum     */

    FT_TS_ULong  OrigOffset;    /* uncompressed table file offset */
                             /* (not in the WOFF file)         */
  } WOFF_TableRec, *WOFF_Table;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_TtcFontRec
   *
   * @description:
   *   Metadata for a TTC font entry in WOFF2.
   *
   * @fields:
   *   flavor ::
   *     TTC font flavor.
   *
   *   num_tables ::
   *     Number of tables in TTC, indicating number of elements in
   *     `table_indices`.
   *
   *   table_indices ::
   *     Array of table indices for each TTC font.
   */
  typedef struct  WOFF2_TtcFontRec_
  {
    FT_TS_ULong    flavor;
    FT_TS_UShort   num_tables;
    FT_TS_UShort*  table_indices;

  } WOFF2_TtcFontRec, *WOFF2_TtcFont;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_HeaderRec
   *
   * @description:
   *   WOFF2 file format header.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF2/#woff20Header
   *
   * @note:
   *   We don't care about the fields `reserved`, `majorVersion` and
   *   `minorVersion`, so they are not included.  The `totalSfntSize` field
   *   does not necessarily represent the actual size of the uncompressed
   *   SFNT font stream, so that is used as a reference value instead.
   */
  typedef struct  WOFF2_HeaderRec_
  {
    FT_TS_ULong   signature;
    FT_TS_ULong   flavor;
    FT_TS_ULong   length;
    FT_TS_UShort  num_tables;
    FT_TS_ULong   totalSfntSize;
    FT_TS_ULong   totalCompressedSize;
    FT_TS_ULong   metaOffset;
    FT_TS_ULong   metaLength;
    FT_TS_ULong   metaOrigLength;
    FT_TS_ULong   privOffset;
    FT_TS_ULong   privLength;

    FT_TS_ULong   uncompressed_size;    /* uncompressed brotli stream size */
    FT_TS_ULong   compressed_offset;    /* compressed stream offset        */
    FT_TS_ULong   header_version;       /* version of original TTC Header  */
    FT_TS_UShort  num_fonts;            /* number of fonts in TTC          */
    FT_TS_ULong   actual_sfnt_size;     /* actual size of sfnt stream      */

    WOFF2_TtcFont  ttc_fonts;        /* metadata for fonts in a TTC     */

  } WOFF2_HeaderRec, *WOFF2_Header;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_TableRec
   *
   * @description:
   *   This structure describes a given table of a WOFF2 font.
   *
   * @fields:
   *   See
   *
   *     https://www.w3.org/TR/WOFF2/#table_dir_format
   */
  typedef struct  WOFF2_TableRec_
  {
    FT_TS_Byte   FlagByte;           /* table type and flags      */
    FT_TS_Tag    Tag;                /* table file offset         */
    FT_TS_ULong  dst_length;         /* uncompressed table length */
    FT_TS_ULong  TransformLength;    /* transformed length        */

    FT_TS_ULong  flags;              /* calculated flags          */
    FT_TS_ULong  src_offset;         /* compressed table offset   */
    FT_TS_ULong  src_length;         /* compressed table length   */
    FT_TS_ULong  dst_offset;         /* uncompressed table offset */

  } WOFF2_TableRec, *WOFF2_Table;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_InfoRec
   *
   * @description:
   *   Metadata for WOFF2 font that may be required for reconstruction of
   *   sfnt tables.
   *
   * @fields:
   *   header_checksum ::
   *     Checksum of SFNT offset table.
   *
   *   num_glyphs ::
   *     Number of glyphs in the font.
   *
   *   num_hmetrics ::
   *     `numberOfHMetrics` field in the 'hhea' table.
   *
   *   x_mins ::
   *     `xMin` values of glyph bounding box.
   *
   *   glyf_table ::
   *     A pointer to the `glyf' table record.
   *
   *   loca_table ::
   *     A pointer to the `loca' table record.
   *
   *   head_table ::
   *     A pointer to the `head' table record.
   */
  typedef struct  WOFF2_InfoRec_
  {
    FT_TS_ULong   header_checksum;
    FT_TS_UShort  num_glyphs;
    FT_TS_UShort  num_hmetrics;
    FT_TS_Short*  x_mins;

    WOFF2_Table  glyf_table;
    WOFF2_Table  loca_table;
    WOFF2_Table  head_table;

  } WOFF2_InfoRec, *WOFF2_Info;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_SubstreamRec
   *
   * @description:
   *   This structure stores information about a substream in the transformed
   *   'glyf' table in a WOFF2 stream.
   *
   * @fields:
   *   start ::
   *     Beginning of the substream relative to uncompressed table stream.
   *
   *   offset ::
   *     Offset of the substream relative to uncompressed table stream.
   *
   *   size ::
   *     Size of the substream.
   */
  typedef struct  WOFF2_SubstreamRec_
  {
    FT_TS_ULong  start;
    FT_TS_ULong  offset;
    FT_TS_ULong  size;

  } WOFF2_SubstreamRec, *WOFF2_Substream;


  /**************************************************************************
   *
   * @struct:
   *   WOFF2_PointRec
   *
   * @description:
   *   This structure stores information about a point in the transformed
   *   'glyf' table in a WOFF2 stream.
   *
   * @fields:
   *   x ::
   *     x-coordinate of point.
   *
   *   y ::
   *     y-coordinate of point.
   *
   *   on_curve ::
   *     Set if point is on-curve.
   */
  typedef struct  WOFF2_PointRec_
  {
    FT_TS_Int   x;
    FT_TS_Int   y;
    FT_TS_Bool  on_curve;

  } WOFF2_PointRec, *WOFF2_Point;


FT_TS_END_HEADER

#endif /* WOFFTYPES_H_ */


/* END */
