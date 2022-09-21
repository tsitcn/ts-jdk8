/****************************************************************************
 *
 * gxvmorx1.c
 *
 *   TrueTypeGX/AAT morx table validation
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


#include "gxvmorx.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvmorx


  typedef struct  GXV_morx_subtable_type1_StateOptRec_
  {
    FT_TS_ULong   substitutionTable;
    FT_TS_ULong   substitutionTable_length;
    FT_TS_UShort  substitutionTable_num_lookupTables;

  }  GXV_morx_subtable_type1_StateOptRec,
    *GXV_morx_subtable_type1_StateOptRecData;


#define GXV_MORX_SUBTABLE_TYPE1_HEADER_SIZE \
          ( GXV_STATETABLE_HEADER_SIZE + 2 )


  static void
  gxv_morx_subtable_type1_substitutionTable_load( FT_TS_Bytes       table,
                                                  FT_TS_Bytes       limit,
                                                  GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_morx_subtable_type1_StateOptRecData  optdata =
      (GXV_morx_subtable_type1_StateOptRecData)gxvalid->xstatetable.optdata;


    GXV_LIMIT_CHECK( 2 );
    optdata->substitutionTable = FT_TS_NEXT_USHORT( p );
  }


  static void
  gxv_morx_subtable_type1_subtable_setup( FT_TS_ULong       table_size,
                                          FT_TS_ULong       classTable,
                                          FT_TS_ULong       stateArray,
                                          FT_TS_ULong       entryTable,
                                          FT_TS_ULong*      classTable_length_p,
                                          FT_TS_ULong*      stateArray_length_p,
                                          FT_TS_ULong*      entryTable_length_p,
                                          GXV_Validator  gxvalid )
  {
    FT_TS_ULong  o[4];
    FT_TS_ULong  *l[4];
    FT_TS_ULong  buff[5];

    GXV_morx_subtable_type1_StateOptRecData  optdata =
      (GXV_morx_subtable_type1_StateOptRecData)gxvalid->xstatetable.optdata;


    o[0] = classTable;
    o[1] = stateArray;
    o[2] = entryTable;
    o[3] = optdata->substitutionTable;
    l[0] = classTable_length_p;
    l[1] = stateArray_length_p;
    l[2] = entryTable_length_p;
    l[3] = &(optdata->substitutionTable_length);

    gxv_set_length_by_ulong_offset( o, l, buff, 4, table_size, gxvalid );
  }


  static void
  gxv_morx_subtable_type1_entry_validate(
    FT_TS_UShort                       state,
    FT_TS_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_TS_Bytes                        table,
    FT_TS_Bytes                        limit,
    GXV_Validator                   gxvalid )
  {
#ifdef GXV_LOAD_TRACE_VARS
    FT_TS_UShort  setMark;
    FT_TS_UShort  dontAdvance;
#endif
    FT_TS_UShort  reserved;
    FT_TS_Short   markIndex;
    FT_TS_Short   currentIndex;

    GXV_morx_subtable_type1_StateOptRecData  optdata =
      (GXV_morx_subtable_type1_StateOptRecData)gxvalid->xstatetable.optdata;

    FT_TS_UNUSED( state );
    FT_TS_UNUSED( table );
    FT_TS_UNUSED( limit );


#ifdef GXV_LOAD_TRACE_VARS
    setMark      = (FT_TS_UShort)( ( flags >> 15 ) & 1 );
    dontAdvance  = (FT_TS_UShort)( ( flags >> 14 ) & 1 );
#endif

    reserved = (FT_TS_UShort)( flags & 0x3FFF );

    markIndex    = (FT_TS_Short)( glyphOffset_p->ul >> 16 );
    currentIndex = (FT_TS_Short)( glyphOffset_p->ul       );

    GXV_TRACE(( " setMark=%01d dontAdvance=%01d\n",
                setMark, dontAdvance ));

    if ( 0 < reserved )
    {
      GXV_TRACE(( " non-zero bits found in reserved range\n" ));
      GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_DATA );
    }

    GXV_TRACE(( "markIndex = %d, currentIndex = %d\n",
                markIndex, currentIndex ));

    if ( optdata->substitutionTable_num_lookupTables < markIndex + 1 )
      optdata->substitutionTable_num_lookupTables =
        (FT_TS_UShort)( markIndex + 1 );

    if ( optdata->substitutionTable_num_lookupTables < currentIndex + 1 )
      optdata->substitutionTable_num_lookupTables =
        (FT_TS_UShort)( currentIndex + 1 );
  }


  static void
  gxv_morx_subtable_type1_LookupValue_validate( FT_TS_UShort            glyph,
                                                GXV_LookupValueCPtr  value_p,
                                                GXV_Validator        gxvalid )
  {
    FT_TS_UNUSED( glyph ); /* for the non-debugging case */

    GXV_TRACE(( "morx subtable type1 subst.: %d -> %d\n", glyph, value_p->u ));

    if ( value_p->u > gxvalid->face->num_glyphs )
      FT_TS_INVALID_GLYPH_ID;
  }


  static GXV_LookupValueDesc
  gxv_morx_subtable_type1_LookupFmt4_transit(
    FT_TS_UShort            relative_gindex,
    GXV_LookupValueCPtr  base_value_p,
    FT_TS_Bytes             lookuptbl_limit,
    GXV_Validator        gxvalid )
  {
    FT_TS_Bytes             p;
    FT_TS_Bytes             limit;
    FT_TS_UShort            offset;
    GXV_LookupValueDesc  value;

    /* XXX: check range? */
    offset = (FT_TS_UShort)( base_value_p->u +
                          relative_gindex * sizeof ( FT_TS_UShort ) );

    p     = gxvalid->lookuptbl_head + offset;
    limit = lookuptbl_limit;

    GXV_LIMIT_CHECK ( 2 );
    value.u = FT_TS_NEXT_USHORT( p );

    return value;
  }


  /*
   * TODO: length should be limit?
   **/
  static void
  gxv_morx_subtable_type1_substitutionTable_validate( FT_TS_Bytes       table,
                                                      FT_TS_Bytes       limit,
                                                      GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  i;

    GXV_morx_subtable_type1_StateOptRecData  optdata =
      (GXV_morx_subtable_type1_StateOptRecData)gxvalid->xstatetable.optdata;


    /* TODO: calculate offset/length for each lookupTables */
    gxvalid->lookupval_sign   = GXV_LOOKUPVALUE_UNSIGNED;
    gxvalid->lookupval_func   = gxv_morx_subtable_type1_LookupValue_validate;
    gxvalid->lookupfmt4_trans = gxv_morx_subtable_type1_LookupFmt4_transit;

    for ( i = 0; i < optdata->substitutionTable_num_lookupTables; i++ )
    {
      FT_TS_ULong  offset;


      GXV_LIMIT_CHECK( 4 );
      offset = FT_TS_NEXT_ULONG( p );

      gxv_LookupTable_validate( table + offset, limit, gxvalid );
    }

    /* TODO: overlapping of lookupTables in substitutionTable */
  }


  /*
   * subtable for Contextual glyph substitution is a modified StateTable.
   * In addition to classTable, stateArray, entryTable, the field
   * `substitutionTable' is added.
   */
  FT_TS_LOCAL_DEF( void )
  gxv_morx_subtable_type1_validate( FT_TS_Bytes       table,
                                    FT_TS_Bytes       limit,
                                    GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;

    GXV_morx_subtable_type1_StateOptRec  st_rec;


    GXV_NAME_ENTER( "morx chain subtable type1 (Contextual Glyph Subst)" );

    GXV_LIMIT_CHECK( GXV_MORX_SUBTABLE_TYPE1_HEADER_SIZE );

    st_rec.substitutionTable_num_lookupTables = 0;

    gxvalid->xstatetable.optdata =
      &st_rec;
    gxvalid->xstatetable.optdata_load_func =
      gxv_morx_subtable_type1_substitutionTable_load;
    gxvalid->xstatetable.subtable_setup_func =
      gxv_morx_subtable_type1_subtable_setup;
    gxvalid->xstatetable.entry_glyphoffset_fmt =
      GXV_GLYPHOFFSET_ULONG;
    gxvalid->xstatetable.entry_validate_func =
      gxv_morx_subtable_type1_entry_validate;

    gxv_XStateTable_validate( p, limit, gxvalid );

    gxv_morx_subtable_type1_substitutionTable_validate(
      table + st_rec.substitutionTable,
      table + st_rec.substitutionTable + st_rec.substitutionTable_length,
      gxvalid );

    GXV_EXIT;
  }


/* END */
