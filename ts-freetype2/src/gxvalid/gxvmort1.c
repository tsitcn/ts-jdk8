/****************************************************************************
 *
 * gxvmort1.c
 *
 *   TrueTypeGX/AAT mort table validation
 *   body for type1 (Contextual Substitution) subtable.
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


#include "gxvmort.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvmort


  typedef struct  GXV_mort_subtable_type1_StateOptRec_
  {
    FT_TS_UShort  substitutionTable;
    FT_TS_UShort  substitutionTable_length;

  }  GXV_mort_subtable_type1_StateOptRec,
    *GXV_mort_subtable_type1_StateOptRecData;

#define GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE \
          ( GXV_STATETABLE_HEADER_SIZE + 2 )


  static void
  gxv_mort_subtable_type1_substitutionTable_load( FT_TS_Bytes       table,
                                                  FT_TS_Bytes       limit,
                                                  GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_mort_subtable_type1_StateOptRecData  optdata =
      (GXV_mort_subtable_type1_StateOptRecData)gxvalid->statetable.optdata;


    GXV_LIMIT_CHECK( 2 );
    optdata->substitutionTable = FT_TS_NEXT_USHORT( p );
  }


  static void
  gxv_mort_subtable_type1_subtable_setup( FT_TS_UShort      table_size,
                                          FT_TS_UShort      classTable,
                                          FT_TS_UShort      stateArray,
                                          FT_TS_UShort      entryTable,
                                          FT_TS_UShort*     classTable_length_p,
                                          FT_TS_UShort*     stateArray_length_p,
                                          FT_TS_UShort*     entryTable_length_p,
                                          GXV_Validator  gxvalid )
  {
    FT_TS_UShort  o[4];
    FT_TS_UShort  *l[4];
    FT_TS_UShort  buff[5];

    GXV_mort_subtable_type1_StateOptRecData  optdata =
      (GXV_mort_subtable_type1_StateOptRecData)gxvalid->statetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->substitutionTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &( optdata->substitutionTable_length );

    gxv_set_length_by_ushort_offset( o, l, buff, 4, table_size, gxvalid );
  }


  static void
  gxv_mort_subtable_type1_offset_to_subst_validate(
    FT_TS_Short          wordOffset,
    const FT_TS_String*  tag,
    FT_TS_Byte           state,
    GXV_Validator     gxvalid )
  {
    FT_TS_UShort  substTable;
    FT_TS_UShort  substTable_limit;

    FT_TS_UNUSED( tag );
    FT_TS_UNUSED( state );


    substTable =
      ((GXV_mort_subtable_type1_StateOptRec *)
       (gxvalid->statetable.optdata))->substitutionTable;
    substTable_limit =
      (FT_TS_UShort)( substTable +
                   ((GXV_mort_subtable_type1_StateOptRec *)
                    (gxvalid->statetable.optdata))->substitutionTable_length );

    gxvalid->min_gid = (FT_TS_UShort)( ( substTable       - wordOffset * 2 ) / 2 );
    gxvalid->max_gid = (FT_TS_UShort)( ( substTable_limit - wordOffset * 2 ) / 2 );
    gxvalid->max_gid = (FT_TS_UShort)( FT_TS_MAX( gxvalid->max_gid,
                                            gxvalid->face->num_glyphs ) );

    /* XXX: check range? */

    /* TODO: min_gid & max_gid comparison with ClassTable contents */
  }


  static void
  gxv_mort_subtable_type1_entry_validate(
    FT_TS_Byte                         state,
    FT_TS_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_TS_Bytes                        table,
    FT_TS_Bytes                        limit,
    GXV_Validator                   gxvalid )
  {
#ifdef GXV_LOAD_UNUSED_VARS
    FT_TS_UShort  setMark;
    FT_TS_UShort  dontAdvance;
#endif
    FT_TS_UShort  reserved;
    FT_TS_Short   markOffset;
    FT_TS_Short   currentOffset;

    FT_TS_UNUSED( table );
    FT_TS_UNUSED( limit );


#ifdef GXV_LOAD_UNUSED_VARS
    setMark       = (FT_TS_UShort)(   flags >> 15            );
    dontAdvance   = (FT_TS_UShort)( ( flags >> 14 ) & 1      );
#endif
    reserved      = (FT_TS_UShort)(    flags        & 0x3FFF );

    markOffset    = (FT_TS_Short)( glyphOffset_p->ul >> 16 );
    currentOffset = (FT_TS_Short)( glyphOffset_p->ul       );

    if ( 0 < reserved )
    {
      GXV_TRACE(( " non-zero bits found in reserved range\n" ));
      GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_DATA );
    }

    gxv_mort_subtable_type1_offset_to_subst_validate( markOffset,
                                                      "markOffset",
                                                      state,
                                                      gxvalid );

    gxv_mort_subtable_type1_offset_to_subst_validate( currentOffset,
                                                      "currentOffset",
                                                      state,
                                                      gxvalid );
  }


  static void
  gxv_mort_subtable_type1_substTable_validate( FT_TS_Bytes       table,
                                               FT_TS_Bytes       limit,
                                               GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  num_gids = (FT_TS_UShort)(
                 ((GXV_mort_subtable_type1_StateOptRec *)
                  (gxvalid->statetable.optdata))->substitutionTable_length / 2 );
    FT_TS_UShort  i;


    GXV_NAME_ENTER( "validating contents of substitutionTable" );
    for ( i = 0; i < num_gids; i++ )
    {
      FT_TS_UShort  dst_gid;


      GXV_LIMIT_CHECK( 2 );
      dst_gid = FT_TS_NEXT_USHORT( p );

      if ( dst_gid >= 0xFFFFU )
        continue;

      if ( dst_gid < gxvalid->min_gid || gxvalid->max_gid < dst_gid )
      {
        GXV_TRACE(( "substTable include a strange gid[%d]=%d >"
                    " out of define range (%d..%d)\n",
                    i, dst_gid, gxvalid->min_gid, gxvalid->max_gid ));
        GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_GLYPH_ID );
      }
    }

    GXV_EXIT;
  }


  /*
   * subtable for Contextual glyph substitution is a modified StateTable.
   * In addition to classTable, stateArray, and entryTable, the field
   * `substitutionTable' is added.
   */
  FT_TS_LOCAL_DEF( void )
  gxv_mort_subtable_type1_validate( FT_TS_Bytes       table,
                                    FT_TS_Bytes       limit,
                                    GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_mort_subtable_type1_StateOptRec  st_rec;


    GXV_NAME_ENTER( "mort chain subtable type1 (Contextual Glyph Subst)" );

    GXV_LIMIT_CHECK( GXV_MORT_SUBTABLE_TYPE1_HEADER_SIZE );

    gxvalid->statetable.optdata =
      &st_rec;
    gxvalid->statetable.optdata_load_func =
      gxv_mort_subtable_type1_substitutionTable_load;
    gxvalid->statetable.subtable_setup_func =
      gxv_mort_subtable_type1_subtable_setup;
    gxvalid->statetable.entry_glyphoffset_fmt =
      GXV_GLYPHOFFSET_ULONG;
    gxvalid->statetable.entry_validate_func =

      gxv_mort_subtable_type1_entry_validate;
    gxv_StateTable_validate( p, limit, gxvalid );

    gxv_mort_subtable_type1_substTable_validate(
      table + st_rec.substitutionTable,
      table + st_rec.substitutionTable + st_rec.substitutionTable_length,
      gxvalid );

    GXV_EXIT;
  }


/* END */
