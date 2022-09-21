/****************************************************************************
 *
 * gxvmorx5.c
 *
 *   TrueTypeGX/AAT morx table validation
 *   body for type5 (Contextual Glyph Insertion) subtable.
 *
 * Copyright (C) 2005-2022 by
 * suzuki toshiya, Masatake YAMATO, Red Hat K.K.,
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/****************************************************************************
 *
 * gxvalid is derived from both gxlayout module and otvalid module.
 * Development of gxlayout is supported by the Information-technology
 * Promotion Agency(IPA), Japan.
 *
 */


#include "gxvmorx.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvmorx


  /*
   * `morx' subtable type5 (Contextual Glyph Insertion)
   * has format of a StateTable with insertion-glyph-list
   * without name.  However, the 32bit offset from the head
   * of subtable to the i-g-l is given after `entryTable',
   * without variable name specification (the existence of
   * this offset to the table is different from mort type5).
   */


  typedef struct  GXV_morx_subtable_type5_StateOptRec_
  {
    FT_TS_ULong  insertionGlyphList;
    FT_TS_ULong  insertionGlyphList_length;

  }  GXV_morx_subtable_type5_StateOptRec,
    *GXV_morx_subtable_type5_StateOptRecData;


#define GXV_MORX_SUBTABLE_TYPE5_HEADER_SIZE \
          ( GXV_STATETABLE_HEADER_SIZE + 4 )


  static void
  gxv_morx_subtable_type5_insertionGlyphList_load( FT_TS_Bytes       table,
                                                   FT_TS_Bytes       limit,
                                                   GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_morx_subtable_type5_StateOptRecData  optdata =
      (GXV_morx_subtable_type5_StateOptRecData)gxvalid->xstatetable.optdata;


    GXV_LIMIT_CHECK( 4 );
    optdata->insertionGlyphList = FT_TS_NEXT_ULONG( p );
  }


  static void
  gxv_morx_subtable_type5_subtable_setup( FT_TS_ULong       table_size,
                                          FT_TS_ULong       classTable,
                                          FT_TS_ULong       stateArray,
                                          FT_TS_ULong       entryTable,
                                          FT_TS_ULong*      classTable_length_p,
                                          FT_TS_ULong*      stateArray_length_p,
                                          FT_TS_ULong*      entryTable_length_p,
                                          GXV_Validator  gxvalid )
  {
    FT_TS_ULong   o[4];
    FT_TS_ULong*  l[4];
    FT_TS_ULong   buff[5];

    GXV_morx_subtable_type5_StateOptRecData  optdata =
      (GXV_morx_subtable_type5_StateOptRecData)gxvalid->xstatetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->insertionGlyphList;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &(optdata->insertionGlyphList_length);

    gxv_set_length_by_ulong_offset( o, l, buff, 4, table_size, gxvalid );
  }


  static void
  gxv_morx_subtable_type5_InsertList_validate( FT_TS_UShort      table_index,
                                               FT_TS_UShort      count,
                                               FT_TS_Bytes       table,
                                               FT_TS_Bytes       limit,
                                               GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table + table_index * 2;


#ifndef GXV_LOAD_TRACE_VARS
    GXV_LIMIT_CHECK( count * 2 );
#else
    while ( p < table + count * 2 + table_index * 2 )
    {
      FT_TS_UShort  insert_glyphID;


      GXV_LIMIT_CHECK( 2 );
      insert_glyphID = FT_TS_NEXT_USHORT( p );
      GXV_TRACE(( " 0x%04x", insert_glyphID ));
    }

    GXV_TRACE(( "\n" ));
#endif
  }


  static void
  gxv_morx_subtable_type5_entry_validate(
    FT_TS_UShort                       state,
    FT_TS_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_TS_Bytes                        table,
    FT_TS_Bytes                        limit,
    GXV_Validator                   gxvalid )
  {
#ifdef GXV_LOAD_UNUSED_VARS
    FT_TS_Bool    setMark;
    FT_TS_Bool    dontAdvance;
    FT_TS_Bool    currentIsKashidaLike;
    FT_TS_Bool    markedIsKashidaLike;
    FT_TS_Bool    currentInsertBefore;
    FT_TS_Bool    markedInsertBefore;
#endif
    FT_TS_Byte    currentInsertCount;
    FT_TS_Byte    markedInsertCount;
    FT_TS_Byte    currentInsertList;
    FT_TS_UShort  markedInsertList;

    FT_TS_UNUSED( state );


#ifdef GXV_LOAD_UNUSED_VARS
    setMark              = FT_TS_BOOL( ( flags >> 15 ) & 1 );
    dontAdvance          = FT_TS_BOOL( ( flags >> 14 ) & 1 );
    currentIsKashidaLike = FT_TS_BOOL( ( flags >> 13 ) & 1 );
    markedIsKashidaLike  = FT_TS_BOOL( ( flags >> 12 ) & 1 );
    currentInsertBefore  = FT_TS_BOOL( ( flags >> 11 ) & 1 );
    markedInsertBefore   = FT_TS_BOOL( ( flags >> 10 ) & 1 );
#endif

    currentInsertCount = (FT_TS_Byte)( ( flags >> 5 ) & 0x1F   );
    markedInsertCount  = (FT_TS_Byte)(   flags        & 0x001F );

    currentInsertList = (FT_TS_Byte)  ( glyphOffset_p->ul >> 16 );
    markedInsertList  = (FT_TS_UShort)( glyphOffset_p->ul       );

    if ( currentInsertList && 0 != currentInsertCount )
      gxv_morx_subtable_type5_InsertList_validate( currentInsertList,
                                                   currentInsertCount,
                                                   table, limit,
                                                   gxvalid );

    if ( markedInsertList && 0 != markedInsertCount )
      gxv_morx_subtable_type5_InsertList_validate( markedInsertList,
                                                   markedInsertCount,
                                                   table, limit,
                                                   gxvalid );
  }


  FT_TS_LOCAL_DEF( void )
  gxv_morx_subtable_type5_validate( FT_TS_Bytes       table,
                                    FT_TS_Bytes       limit,
                                    GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_morx_subtable_type5_StateOptRec      et_rec;
    GXV_morx_subtable_type5_StateOptRecData  et = &et_rec;


    GXV_NAME_ENTER( "morx chain subtable type5 (Glyph Insertion)" );

    GXV_LIMIT_CHECK( GXV_MORX_SUBTABLE_TYPE5_HEADER_SIZE );

    gxvalid->xstatetable.optdata =
      et;
    gxvalid->xstatetable.optdata_load_func =
      gxv_morx_subtable_type5_insertionGlyphList_load;
    gxvalid->xstatetable.subtable_setup_func =
      gxv_morx_subtable_type5_subtable_setup;
    gxvalid->xstatetable.entry_glyphoffset_fmt =
      GXV_GLYPHOFFSET_ULONG;
    gxvalid->xstatetable.entry_validate_func =
      gxv_morx_subtable_type5_entry_validate;

    gxv_XStateTable_validate( p, limit, gxvalid );

    GXV_EXIT;
  }


/* END */
