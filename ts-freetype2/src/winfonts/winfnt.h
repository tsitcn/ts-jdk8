/****************************************************************************
 *
 * winfnt.h
 *
 *   FreeType font driver for Windows FNT/FON files
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 * Copyright 2007 Dmitry Timoshkov for Codeweavers
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef WINFNT_H_
#define WINFNT_H_


#include <freetype/ftwinfnt.h>
#include <freetype/internal/ftdrv.h>


FT_TS_BEGIN_HEADER


  typedef struct  WinMZ_HeaderRec_
  {
    FT_TS_UShort  magic;
    /* skipped content */
    FT_TS_UShort  lfanew;

  } WinMZ_HeaderRec;


  typedef struct  WinNE_HeaderRec_
  {
    FT_TS_UShort  magic;
    /* skipped content */
    FT_TS_UShort  resource_tab_offset;
    FT_TS_UShort  rname_tab_offset;

  } WinNE_HeaderRec;


  typedef struct  WinPE32_HeaderRec_
  {
    FT_TS_ULong   magic;
    FT_TS_UShort  machine;
    FT_TS_UShort  number_of_sections;
    /* skipped content */
    FT_TS_UShort  size_of_optional_header;
    /* skipped content */
    FT_TS_UShort  magic32;
    /* skipped content */
    FT_TS_ULong   rsrc_virtual_address;
    FT_TS_ULong   rsrc_size;
    /* skipped content */

  } WinPE32_HeaderRec;


  typedef struct  WinPE32_SectionRec_
  {
    FT_TS_Byte   name[8];
    /* skipped content */
    FT_TS_ULong  virtual_address;
    FT_TS_ULong  size_of_raw_data;
    FT_TS_ULong  pointer_to_raw_data;
    /* skipped content */

  } WinPE32_SectionRec;


  typedef struct  WinPE_RsrcDirRec_
  {
    FT_TS_ULong   characteristics;
    FT_TS_ULong   time_date_stamp;
    FT_TS_UShort  major_version;
    FT_TS_UShort  minor_version;
    FT_TS_UShort  number_of_named_entries;
    FT_TS_UShort  number_of_id_entries;

  } WinPE_RsrcDirRec;


  typedef struct  WinPE_RsrcDirEntryRec_
  {
    FT_TS_ULong  name;
    FT_TS_ULong  offset;

  } WinPE_RsrcDirEntryRec;


  typedef struct  WinPE_RsrcDataEntryRec_
  {
    FT_TS_ULong  offset_to_data;
    FT_TS_ULong  size;
    FT_TS_ULong  code_page;
    FT_TS_ULong  reserved;

  } WinPE_RsrcDataEntryRec;


  typedef struct  WinNameInfoRec_
  {
    FT_TS_UShort  offset;
    FT_TS_UShort  length;
    FT_TS_UShort  flags;
    FT_TS_UShort  id;
    FT_TS_UShort  handle;
    FT_TS_UShort  usage;

  } WinNameInfoRec;


  typedef struct  WinResourceInfoRec_
  {
    FT_TS_UShort  type_id;
    FT_TS_UShort  count;

  } WinResourceInfoRec;


#define WINFNT_MZ_MAGIC  0x5A4D
#define WINFNT_NE_MAGIC  0x454E
#define WINFNT_PE_MAGIC  0x4550


  typedef struct  FNT_FontRec_
  {
    FT_TS_ULong             offset;

    FT_TS_WinFNT_HeaderRec  header;

    FT_TS_Byte*             fnt_frame;
    FT_TS_ULong             fnt_size;
    FT_TS_String*           family_name;

  } FNT_FontRec, *FNT_Font;


  typedef struct  FNT_FaceRec_
  {
    FT_TS_FaceRec     root;
    FNT_Font       font;

  } FNT_FaceRec, *FNT_Face;


  FT_TS_EXPORT_VAR( const FT_TS_Driver_ClassRec )  winfnt_driver_class;


FT_TS_END_HEADER


#endif /* WINFNT_H_ */


/* END */
