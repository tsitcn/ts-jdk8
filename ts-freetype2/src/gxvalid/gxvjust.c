/****************************************************************************
 *
 * gxvjust.c
 *
 *   TrueTypeGX/AAT just table validation (body).
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


#include "gxvalid.h"
#include "gxvcommn.h"

#include <freetype/ftsnames.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvjust

  /*
   * referred `just' table format specification:
   * https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6just.html
   * last updated 2000.
   * ----------------------------------------------
   * [JUST HEADER]: GXV_JUST_HEADER_SIZE
   * version     (fixed:  32bit) = 0x00010000
   * format      (uint16: 16bit) = 0 is only defined (2000)
   * horizOffset (uint16: 16bit)
   * vertOffset  (uint16: 16bit)
   * ----------------------------------------------
   */

  typedef struct  GXV_just_DataRec_
  {
    FT_TS_UShort  wdc_offset_max;
    FT_TS_UShort  wdc_offset_min;
    FT_TS_UShort  pc_offset_max;
    FT_TS_UShort  pc_offset_min;

  } GXV_just_DataRec, *GXV_just_Data;


#define  GXV_JUST_DATA( a )  GXV_TABLE_DATA( just, a )


  /* GX just table does not define their subset of GID */
  static void
  gxv_just_check_max_gid( FT_TS_UShort         gid,
                          const FT_TS_String*  msg_tag,
                          GXV_Validator     gxvalid )
  {
    FT_TS_UNUSED( msg_tag );

    if ( gid < gxvalid->face->num_glyphs )
      return;

    GXV_TRACE(( "just table includes too large %s"
                " GID=%d > %ld (in maxp)\n",
                msg_tag, gid, gxvalid->face->num_glyphs ));
    GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_GLYPH_ID );
  }


  static void
  gxv_just_wdp_entry_validate( FT_TS_Bytes       table,
                               FT_TS_Bytes       limit,
                               GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_ULong   justClass;
#ifdef GXV_LOAD_UNUSED_VARS
    FT_TS_Fixed   beforeGrowLimit;
    FT_TS_Fixed   beforeShrinkGrowLimit;
    FT_TS_Fixed   afterGrowLimit;
    FT_TS_Fixed   afterShrinkGrowLimit;
    FT_TS_UShort  growFlags;
    FT_TS_UShort  shrinkFlags;
#endif


    GXV_LIMIT_CHECK( 4 + 4 + 4 + 4 + 4 + 2 + 2 );
    justClass             = FT_TS_NEXT_ULONG( p );
#ifndef GXV_LOAD_UNUSED_VARS
    p += 4 + 4 + 4 + 4 + 2 + 2;
#else
    beforeGrowLimit       = FT_TS_NEXT_ULONG( p );
    beforeShrinkGrowLimit = FT_TS_NEXT_ULONG( p );
    afterGrowLimit        = FT_TS_NEXT_ULONG( p );
    afterShrinkGrowLimit  = FT_TS_NEXT_ULONG( p );
    growFlags             = FT_TS_NEXT_USHORT( p );
    shrinkFlags           = FT_TS_NEXT_USHORT( p );
#endif

    /* According to Apple spec, only 7bits in justClass is used */
    if ( ( justClass & 0xFFFFFF80UL ) != 0 )
    {
      GXV_TRACE(( "just table includes non-zero value"
                  " in unused justClass higher bits"
                  " of WidthDeltaPair" ));
      GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_DATA );
    }

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  static void
  gxv_just_wdc_entry_validate( FT_TS_Bytes       table,
                               FT_TS_Bytes       limit,
                               GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;
    FT_TS_ULong  count, i;


    GXV_LIMIT_CHECK( 4 );
    count = FT_TS_NEXT_ULONG( p );
    for ( i = 0; i < count; i++ )
    {
      GXV_TRACE(( "validating wdc pair %lu/%lu\n", i + 1, count ));
      gxv_just_wdp_entry_validate( p, limit, gxvalid );
      p += gxvalid->subtable_length;
    }

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  static void
  gxv_just_widthDeltaClusters_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p         = table;
    FT_TS_Bytes  wdc_end   = table + GXV_JUST_DATA( wdc_offset_max );
    FT_TS_UInt   i;


    GXV_NAME_ENTER( "just justDeltaClusters" );

    if ( limit <= wdc_end )
      FT_TS_INVALID_OFFSET;

    for ( i = 0; p <= wdc_end; i++ )
    {
      gxv_just_wdc_entry_validate( p, limit, gxvalid );
      p += gxvalid->subtable_length;
    }

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );

    GXV_EXIT;
  }


  static void
  gxv_just_actSubrecord_type0_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;

    FT_TS_Fixed   lowerLimit;
    FT_TS_Fixed   upperLimit;
#ifdef GXV_LOAD_UNUSED_VARS
    FT_TS_UShort  order;
#endif
    FT_TS_UShort  decomposedCount;

    FT_TS_UInt    i;


    GXV_LIMIT_CHECK( 4 + 4 + 2 + 2 );
    lowerLimit      = FT_TS_NEXT_LONG( p );
    upperLimit      = FT_TS_NEXT_LONG( p );
#ifdef GXV_LOAD_UNUSED_VARS
    order           = FT_TS_NEXT_USHORT( p );
#else
    p += 2;
#endif
    decomposedCount = FT_TS_NEXT_USHORT( p );

    if ( lowerLimit >= upperLimit )
    {
      GXV_TRACE(( "just table includes invalid range spec:"
                  " lowerLimit(%ld) > upperLimit(%ld)\n",
                  lowerLimit, upperLimit ));
      GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_DATA );
    }

    for ( i = 0; i < decomposedCount; i++ )
    {
      FT_TS_UShort glyphs;


      GXV_LIMIT_CHECK( 2 );
      glyphs = FT_TS_NEXT_USHORT( p );
      gxv_just_check_max_gid( glyphs, "type0:glyphs", gxvalid );
    }

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  static void
  gxv_just_actSubrecord_type1_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  addGlyph;


    GXV_LIMIT_CHECK( 2 );
    addGlyph = FT_TS_NEXT_USHORT( p );

    gxv_just_check_max_gid( addGlyph, "type1:addGlyph", gxvalid );

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  static void
  gxv_just_actSubrecord_type2_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
#ifdef GXV_LOAD_UNUSED_VARS
    FT_TS_Fixed      substThreshhold; /* Apple misspelled "Threshhold" */
#endif
    FT_TS_UShort  addGlyph;
    FT_TS_UShort  substGlyph;


    GXV_LIMIT_CHECK( 4 + 2 + 2 );
#ifdef GXV_LOAD_UNUSED_VARS
    substThreshhold = FT_TS_NEXT_ULONG( p );
#else
    p += 4;
#endif
    addGlyph        = FT_TS_NEXT_USHORT( p );
    substGlyph      = FT_TS_NEXT_USHORT( p );

    if ( addGlyph != 0xFFFF )
      gxv_just_check_max_gid( addGlyph, "type2:addGlyph", gxvalid );

    gxv_just_check_max_gid( substGlyph, "type2:substGlyph", gxvalid );

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  static void
  gxv_just_actSubrecord_type4_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;
    FT_TS_ULong  variantsAxis;
    FT_TS_Fixed  minimumLimit;
    FT_TS_Fixed  noStretchValue;
    FT_TS_Fixed  maximumLimit;


    GXV_LIMIT_CHECK( 4 + 4 + 4 + 4 );
    variantsAxis   = FT_TS_NEXT_ULONG( p );
    minimumLimit   = FT_TS_NEXT_LONG( p );
    noStretchValue = FT_TS_NEXT_LONG( p );
    maximumLimit   = FT_TS_NEXT_LONG( p );

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );

    if ( variantsAxis != 0x64756374L ) /* 'duct' */
      GXV_TRACE(( "variantsAxis 0x%08lx is non default value",
                   variantsAxis ));

    if ( minimumLimit > noStretchValue )
      GXV_TRACE(( "type4:minimumLimit 0x%08lx > noStretchValue 0x%08lx\n",
                  minimumLimit, noStretchValue ));
    else if ( noStretchValue > maximumLimit )
      GXV_TRACE(( "type4:noStretchValue 0x%08lx > maximumLimit 0x%08lx\n",
                  noStretchValue, maximumLimit ));
    else if ( !IS_PARANOID_VALIDATION )
      return;

    FT_TS_INVALID_DATA;
  }


  static void
  gxv_just_actSubrecord_type5_validate( FT_TS_Bytes       table,
                                        FT_TS_Bytes       limit,
                                        GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  flags;
    FT_TS_UShort  glyph;


    GXV_LIMIT_CHECK( 2 + 2 );
    flags = FT_TS_NEXT_USHORT( p );
    glyph = FT_TS_NEXT_USHORT( p );

    if ( flags )
      GXV_TRACE(( "type5: nonzero value 0x%04x in unused flags\n",
                   flags ));
    gxv_just_check_max_gid( glyph, "type5:glyph", gxvalid );

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );
  }


  /* parse single actSubrecord */
  static void
  gxv_just_actSubrecord_validate( FT_TS_Bytes       table,
                                  FT_TS_Bytes       limit,
                                  GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  actionClass;
    FT_TS_UShort  actionType;
    FT_TS_ULong   actionLength;


    GXV_NAME_ENTER( "just actSubrecord" );

    GXV_LIMIT_CHECK( 2 + 2 + 4 );
    actionClass  = FT_TS_NEXT_USHORT( p );
    actionType   = FT_TS_NEXT_USHORT( p );
    actionLength = FT_TS_NEXT_ULONG( p );

    /* actionClass is related with justClass using 7bit only */
    if ( ( actionClass & 0xFF80 ) != 0 )
      GXV_SET_ERR_IF_PARANOID( FT_TS_INVALID_DATA );

    if ( actionType == 0 )
      gxv_just_actSubrecord_type0_validate( p, limit, gxvalid );
    else if ( actionType == 1 )
      gxv_just_actSubrecord_type1_validate( p, limit, gxvalid );
    else if ( actionType == 2 )
      gxv_just_actSubrecord_type2_validate( p, limit, gxvalid );
    else if ( actionType == 3 )
      ;                         /* Stretch glyph action: no actionData */
    else if ( actionType == 4 )
      gxv_just_actSubrecord_type4_validate( p, limit, gxvalid );
    else if ( actionType == 5 )
      gxv_just_actSubrecord_type5_validate( p, limit, gxvalid );
    else
      FT_TS_INVALID_DATA;

    gxvalid->subtable_length = actionLength;

    GXV_EXIT;
  }


  static void
  gxv_just_pcActionRecord_validate( FT_TS_Bytes       table,
                                    FT_TS_Bytes       limit,
                                    GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;
    FT_TS_ULong  actionCount;
    FT_TS_ULong  i;


    GXV_LIMIT_CHECK( 4 );
    actionCount = FT_TS_NEXT_ULONG( p );
    GXV_TRACE(( "actionCount = %lu\n", actionCount ));

    for ( i = 0; i < actionCount; i++ )
    {
      gxv_just_actSubrecord_validate( p, limit, gxvalid );
      p += gxvalid->subtable_length;
    }

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );

    GXV_EXIT;
  }


  static void
  gxv_just_pcTable_LookupValue_entry_validate( FT_TS_UShort            glyph,
                                               GXV_LookupValueCPtr  value_p,
                                               GXV_Validator        gxvalid )
  {
    FT_TS_UNUSED( glyph );

    if ( value_p->u > GXV_JUST_DATA( pc_offset_max ) )
      GXV_JUST_DATA( pc_offset_max ) = value_p->u;
    if ( value_p->u < GXV_JUST_DATA( pc_offset_max ) )
      GXV_JUST_DATA( pc_offset_min ) = value_p->u;
  }


  static void
  gxv_just_pcLookupTable_validate( FT_TS_Bytes       table,
                                   FT_TS_Bytes       limit,
                                   GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;


    GXV_NAME_ENTER( "just pcLookupTable" );
    GXV_JUST_DATA( pc_offset_max ) = 0x0000;
    GXV_JUST_DATA( pc_offset_min ) = 0xFFFFU;

    gxvalid->lookupval_sign = GXV_LOOKUPVALUE_UNSIGNED;
    gxvalid->lookupval_func = gxv_just_pcTable_LookupValue_entry_validate;

    gxv_LookupTable_validate( p, limit, gxvalid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  static void
  gxv_just_postcompTable_validate( FT_TS_Bytes       table,
                                   FT_TS_Bytes       limit,
                                   GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;


    GXV_NAME_ENTER( "just postcompTable" );

    gxv_just_pcLookupTable_validate( p, limit, gxvalid );
    p += gxvalid->subtable_length;

    gxv_just_pcActionRecord_validate( p, limit, gxvalid );
    p += gxvalid->subtable_length;

    gxvalid->subtable_length = (FT_TS_ULong)( p - table );

    GXV_EXIT;
  }


  static void
  gxv_just_classTable_entry_validate(
    FT_TS_Byte                         state,
    FT_TS_UShort                       flags,
    GXV_StateTable_GlyphOffsetCPtr  glyphOffset_p,
    FT_TS_Bytes                        table,
    FT_TS_Bytes                        limit,
    GXV_Validator                   gxvalid )
  {
#ifdef GXV_LOAD_UNUSED_VARS
    /* TODO: validate markClass & currentClass */
    FT_TS_UShort  setMark;
    FT_TS_UShort  dontAdvance;
    FT_TS_UShort  markClass;
    FT_TS_UShort  currentClass;
#endif

    FT_TS_UNUSED( state );
    FT_TS_UNUSED( glyphOffset_p );
    FT_TS_UNUSED( table );
    FT_TS_UNUSED( limit );
    FT_TS_UNUSED( gxvalid );

#ifndef GXV_LOAD_UNUSED_VARS
    FT_TS_UNUSED( flags );
#else
    setMark      = (FT_TS_UShort)( ( flags >> 15 ) & 1    );
    dontAdvance  = (FT_TS_UShort)( ( flags >> 14 ) & 1    );
    markClass    = (FT_TS_UShort)( ( flags >> 7  ) & 0x7F );
    currentClass = (FT_TS_UShort)(   flags         & 0x7F );
#endif
  }


  static void
  gxv_just_justClassTable_validate ( FT_TS_Bytes       table,
                                     FT_TS_Bytes       limit,
                                     GXV_Validator  gxvalid )
  {
    FT_TS_Bytes   p = table;
    FT_TS_UShort  length;
    FT_TS_UShort  coverage;
    FT_TS_ULong   subFeatureFlags;


    GXV_NAME_ENTER( "just justClassTable" );

    GXV_LIMIT_CHECK( 2 + 2 + 4 );
    length          = FT_TS_NEXT_USHORT( p );
    coverage        = FT_TS_NEXT_USHORT( p );
    subFeatureFlags = FT_TS_NEXT_ULONG( p );

    GXV_TRACE(( "  justClassTable: coverage = 0x%04x ", coverage ));
    if ( ( coverage & 0x4000 ) == 0  )
      GXV_TRACE(( "ascending\n" ));
    else
      GXV_TRACE(( "descending\n" ));

    if ( subFeatureFlags )
      GXV_TRACE(( "  justClassTable: nonzero value (0x%08lx)"
                  " in unused subFeatureFlags\n", subFeatureFlags ));

    gxvalid->statetable.optdata               = NULL;
    gxvalid->statetable.optdata_load_func     = NULL;
    gxvalid->statetable.subtable_setup_func   = NULL;
    gxvalid->statetable.entry_glyphoffset_fmt = GXV_GLYPHOFFSET_NONE;
    gxvalid->statetable.entry_validate_func   =
      gxv_just_classTable_entry_validate;

    gxv_StateTable_validate( p, table + length, gxvalid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  static void
  gxv_just_wdcTable_LookupValue_validate( FT_TS_UShort            glyph,
                                          GXV_LookupValueCPtr  value_p,
                                          GXV_Validator        gxvalid )
  {
    FT_TS_UNUSED( glyph );

    if ( value_p->u > GXV_JUST_DATA( wdc_offset_max ) )
      GXV_JUST_DATA( wdc_offset_max ) = value_p->u;
    if ( value_p->u < GXV_JUST_DATA( wdc_offset_min ) )
      GXV_JUST_DATA( wdc_offset_min ) = value_p->u;
  }


  static void
  gxv_just_justData_lookuptable_validate( FT_TS_Bytes       table,
                                          FT_TS_Bytes       limit,
                                          GXV_Validator  gxvalid )
  {
    FT_TS_Bytes  p = table;


    GXV_JUST_DATA( wdc_offset_max ) = 0x0000;
    GXV_JUST_DATA( wdc_offset_min ) = 0xFFFFU;

    gxvalid->lookupval_sign = GXV_LOOKUPVALUE_UNSIGNED;
    gxvalid->lookupval_func = gxv_just_wdcTable_LookupValue_validate;

    gxv_LookupTable_validate( p, limit, gxvalid );

    /* subtable_length is set by gxv_LookupTable_validate() */

    GXV_EXIT;
  }


  /*
   * gxv_just_justData_validate() parses and validates horizData, vertData.
   */
  static void
  gxv_just_justData_validate( FT_TS_Bytes       table,
                              FT_TS_Bytes       limit,
                              GXV_Validator  gxvalid )
  {
    /*
     * following 3 offsets are measured from the start of `just'
     * (which table points to), not justData
     */
    FT_TS_UShort  justClassTableOffset;
    FT_TS_UShort  wdcTableOffset;
    FT_TS_UShort  pcTableOffset;
    FT_TS_Bytes   p = table;

    GXV_ODTECT( 4, odtect );


    GXV_NAME_ENTER( "just justData" );

    GXV_ODTECT_INIT( odtect );
    GXV_LIMIT_CHECK( 2 + 2 + 2 );
    justClassTableOffset = FT_TS_NEXT_USHORT( p );
    wdcTableOffset       = FT_TS_NEXT_USHORT( p );
    pcTableOffset        = FT_TS_NEXT_USHORT( p );

    GXV_TRACE(( " (justClassTableOffset = 0x%04x)\n", justClassTableOffset ));
    GXV_TRACE(( " (wdcTableOffset = 0x%04x)\n", wdcTableOffset ));
    GXV_TRACE(( " (pcTableOffset = 0x%04x)\n", pcTableOffset ));

    gxv_just_justData_lookuptable_validate( p, limit, gxvalid );
    gxv_odtect_add_range( p, gxvalid->subtable_length,
                          "just_LookupTable", odtect );

    if ( wdcTableOffset )
    {
      gxv_just_widthDeltaClusters_validate(
        gxvalid->root->base + wdcTableOffset, limit, gxvalid );
      gxv_odtect_add_range( gxvalid->root->base + wdcTableOffset,
                            gxvalid->subtable_length, "just_wdcTable", odtect );
    }

    if ( pcTableOffset )
    {
      gxv_just_postcompTable_validate( gxvalid->root->base + pcTableOffset,
                                       limit, gxvalid );
      gxv_odtect_add_range( gxvalid->root->base + pcTableOffset,
                            gxvalid->subtable_length, "just_pcTable", odtect );
    }

    if ( justClassTableOffset )
    {
      gxv_just_justClassTable_validate(
        gxvalid->root->base + justClassTableOffset, limit, gxvalid );
      gxv_odtect_add_range( gxvalid->root->base + justClassTableOffset,
                            gxvalid->subtable_length, "just_justClassTable",
                            odtect );
    }

    gxv_odtect_validate( odtect, gxvalid );

    GXV_EXIT;
  }


  FT_TS_LOCAL_DEF( void )
  gxv_just_validate( FT_TS_Bytes      table,
                     FT_TS_Face       face,
                     FT_TS_Validator  ftvalid )
  {
    FT_TS_Bytes           p     = table;
    FT_TS_Bytes           limit = 0;

    GXV_ValidatorRec   gxvalidrec;
    GXV_Validator      gxvalid = &gxvalidrec;
    GXV_just_DataRec   justrec;
    GXV_just_Data      just = &justrec;

    FT_TS_ULong           version;
    FT_TS_UShort          format;
    FT_TS_UShort          horizOffset;
    FT_TS_UShort          vertOffset;

    GXV_ODTECT( 3, odtect );


    GXV_ODTECT_INIT( odtect );

    gxvalid->root       = ftvalid;
    gxvalid->table_data = just;
    gxvalid->face       = face;

    FT_TS_TRACE3(( "validating `just' table\n" ));
    GXV_INIT;

    limit      = gxvalid->root->limit;

    GXV_LIMIT_CHECK( 4 + 2 + 2 + 2 );
    version     = FT_TS_NEXT_ULONG( p );
    format      = FT_TS_NEXT_USHORT( p );
    horizOffset = FT_TS_NEXT_USHORT( p );
    vertOffset  = FT_TS_NEXT_USHORT( p );
    gxv_odtect_add_range( table, (FT_TS_ULong)( p - table ),
                          "just header", odtect );


    /* Version 1.0 (always:2000) */
    GXV_TRACE(( " (version = 0x%08lx)\n", version ));
    if ( version != 0x00010000UL )
      FT_TS_INVALID_FORMAT;

    /* format 0 (always:2000) */
    GXV_TRACE(( " (format = 0x%04x)\n", format ));
    if ( format != 0x0000 )
        FT_TS_INVALID_FORMAT;

    GXV_TRACE(( " (horizOffset = %d)\n", horizOffset  ));
    GXV_TRACE(( " (vertOffset = %d)\n", vertOffset  ));


    /* validate justData */
    if ( 0 < horizOffset )
    {
      gxv_just_justData_validate( table + horizOffset, limit, gxvalid );
      gxv_odtect_add_range( table + horizOffset, gxvalid->subtable_length,
                            "horizJustData", odtect );
    }

    if ( 0 < vertOffset )
    {
      gxv_just_justData_validate( table + vertOffset, limit, gxvalid );
      gxv_odtect_add_range( table + vertOffset, gxvalid->subtable_length,
                            "vertJustData", odtect );
    }

    gxv_odtect_validate( odtect, gxvalid );

    FT_TS_TRACE4(( "\n" ));
  }


/* END */
