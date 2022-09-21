/****************************************************************************
 *
 * otvcommn.h
 *
 *   OpenType common tables validation (specification).
 *
 * Copyright (C) 2004-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef OTVCOMMN_H_
#define OTVCOMMN_H_


#include "otvalid.h"
#include <freetype/internal/ftdebug.h>


FT_TS_BEGIN_HEADER


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                         VALIDATION                            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  typedef struct OTV_ValidatorRec_*  OTV_Validator;

  typedef void  (*OTV_Validate_Func)( FT_TS_Bytes       table,
                                      OTV_Validator  otvalid );

  typedef struct  OTV_ValidatorRec_
  {
    FT_TS_Validator        root;
    FT_TS_UInt             type_count;
    OTV_Validate_Func*  type_funcs;

    FT_TS_UInt             lookup_count;
    FT_TS_UInt             glyph_count;

    FT_TS_UInt             nesting_level;

    OTV_Validate_Func   func[3];

    FT_TS_UInt             extra1;     /* for passing parameters */
    FT_TS_UInt             extra2;
    FT_TS_Bytes            extra3;

#ifdef FT_TS_DEBUG_LEVEL_TRACE
    FT_TS_UInt             debug_indent;
    const FT_TS_String*    debug_function_name[3];
#endif

  } OTV_ValidatorRec;


#undef  FT_TS_INVALID_
#define FT_TS_INVALID_( _error )                                     \
          ft_validator_error( otvalid->root, FT_TS_THROW( _error ) )

#define OTV_OPTIONAL_TABLE( _table )  FT_TS_UShort  _table;      \
                                      FT_TS_Bytes   _table ## _p

#define OTV_OPTIONAL_TABLE32( _table )  FT_TS_ULong  _table;       \
                                        FT_TS_Bytes   _table ## _p

#define OTV_OPTIONAL_OFFSET( _offset )           \
          FT_TS_BEGIN_STMNT                         \
            _offset ## _p = p;                   \
            _offset       = FT_TS_NEXT_USHORT( p ); \
          FT_TS_END_STMNT

#define OTV_OPTIONAL_OFFSET32( _offset )        \
          FT_TS_BEGIN_STMNT                        \
            _offset ## _p = p;                  \
            _offset       = FT_TS_NEXT_ULONG( p ); \
          FT_TS_END_STMNT

#define OTV_LIMIT_CHECK( _count )                      \
          FT_TS_BEGIN_STMNT                               \
            if ( p + (_count) > otvalid->root->limit ) \
              FT_TS_INVALID_TOO_SHORT;                    \
          FT_TS_END_STMNT

#define OTV_SIZE_CHECK( _size )                                     \
          FT_TS_BEGIN_STMNT                                            \
            if ( _size > 0 && _size < table_size )                  \
            {                                                       \
              if ( otvalid->root->level == FT_TS_VALIDATE_PARANOID )   \
                FT_TS_INVALID_OFFSET;                                  \
              else                                                  \
              {                                                     \
                /* strip off `const' */                             \
                FT_TS_Byte*  pp = (FT_TS_Byte*)_size ## _p;               \
                                                                    \
                                                                    \
                FT_TS_TRACE3(( "\n" ));                                \
                FT_TS_TRACE3(( "Invalid offset to optional table `%s'" \
                            " set to zero.\n",                      \
                            #_size ));                              \
                FT_TS_TRACE3(( "\n" ));                                \
                                                                    \
                _size = pp[0] = pp[1] = 0;                          \
              }                                                     \
            }                                                       \
          FT_TS_END_STMNT

#define OTV_SIZE_CHECK32( _size )                                   \
          FT_TS_BEGIN_STMNT                                            \
            if ( _size > 0 && _size < table_size )                  \
            {                                                       \
              if ( otvalid->root->level == FT_TS_VALIDATE_PARANOID )   \
                FT_TS_INVALID_OFFSET;                                  \
              else                                                  \
              {                                                     \
                /* strip off `const' */                             \
                FT_TS_Byte*  pp = (FT_TS_Byte*)_size ## _p;               \
                                                                    \
                                                                    \
                FT_TS_TRACE3(( "\n" ));                                \
                FT_TS_TRACE3(( "Invalid offset to optional table `%s'" \
                            " set to zero.\n",                      \
                            #_size ));                              \
                FT_TS_TRACE3(( "\n" ));                                \
                                                                    \
                _size = pp[0] = pp[1] = pp[2] = pp[3] = 0;          \
              }                                                     \
            }                                                       \
          FT_TS_END_STMNT


#define  OTV_NAME_(x)  #x
#define  OTV_NAME(x)   OTV_NAME_(x)

#define  OTV_FUNC_(x)  x##Func
#define  OTV_FUNC(x)   OTV_FUNC_(x)

#ifdef FT_TS_DEBUG_LEVEL_TRACE

#define OTV_NEST1( x )                                       \
          FT_TS_BEGIN_STMNT                                     \
            otvalid->nesting_level          = 0;             \
            otvalid->func[0]                = OTV_FUNC( x ); \
            otvalid->debug_function_name[0] = OTV_NAME( x ); \
          FT_TS_END_STMNT

#define OTV_NEST2( x, y )                                    \
          FT_TS_BEGIN_STMNT                                     \
            otvalid->nesting_level          = 0;             \
            otvalid->func[0]                = OTV_FUNC( x ); \
            otvalid->func[1]                = OTV_FUNC( y ); \
            otvalid->debug_function_name[0] = OTV_NAME( x ); \
            otvalid->debug_function_name[1] = OTV_NAME( y ); \
          FT_TS_END_STMNT

#define OTV_NEST3( x, y, z )                                 \
          FT_TS_BEGIN_STMNT                                     \
            otvalid->nesting_level          = 0;             \
            otvalid->func[0]                = OTV_FUNC( x ); \
            otvalid->func[1]                = OTV_FUNC( y ); \
            otvalid->func[2]                = OTV_FUNC( z ); \
            otvalid->debug_function_name[0] = OTV_NAME( x ); \
            otvalid->debug_function_name[1] = OTV_NAME( y ); \
            otvalid->debug_function_name[2] = OTV_NAME( z ); \
          FT_TS_END_STMNT

#define OTV_INIT  otvalid->debug_indent = 0

#define OTV_ENTER                                                                \
          FT_TS_BEGIN_STMNT                                                         \
            otvalid->debug_indent += 2;                                          \
            FT_TS_TRACE4(( "%*.s", otvalid->debug_indent, "" ));                    \
            FT_TS_TRACE4(( "%s table\n",                                            \
                        otvalid->debug_function_name[otvalid->nesting_level] )); \
          FT_TS_END_STMNT

#define OTV_NAME_ENTER( name )                                \
          FT_TS_BEGIN_STMNT                                      \
            otvalid->debug_indent += 2;                       \
            FT_TS_TRACE4(( "%*.s", otvalid->debug_indent, "" )); \
            FT_TS_TRACE4(( "%s table\n", name ));                \
          FT_TS_END_STMNT

#define OTV_EXIT  otvalid->debug_indent -= 2

#define OTV_TRACE( s )                                        \
          FT_TS_BEGIN_STMNT                                      \
            FT_TS_TRACE4(( "%*.s", otvalid->debug_indent, "" )); \
            FT_TS_TRACE4( s );                                   \
          FT_TS_END_STMNT

#else   /* !FT_TS_DEBUG_LEVEL_TRACE */

#define OTV_NEST1( x )                              \
          FT_TS_BEGIN_STMNT                            \
            otvalid->nesting_level = 0;             \
            otvalid->func[0]       = OTV_FUNC( x ); \
          FT_TS_END_STMNT

#define OTV_NEST2( x, y )                           \
          FT_TS_BEGIN_STMNT                            \
            otvalid->nesting_level = 0;             \
            otvalid->func[0]       = OTV_FUNC( x ); \
            otvalid->func[1]       = OTV_FUNC( y ); \
          FT_TS_END_STMNT

#define OTV_NEST3( x, y, z )                        \
          FT_TS_BEGIN_STMNT                            \
            otvalid->nesting_level = 0;             \
            otvalid->func[0]       = OTV_FUNC( x ); \
            otvalid->func[1]       = OTV_FUNC( y ); \
            otvalid->func[2]       = OTV_FUNC( z ); \
          FT_TS_END_STMNT

#define OTV_INIT                do { } while ( 0 )
#define OTV_ENTER               do { } while ( 0 )
#define OTV_NAME_ENTER( name )  do { } while ( 0 )
#define OTV_EXIT                do { } while ( 0 )

#define OTV_TRACE( s )          do { } while ( 0 )

#endif  /* !FT_TS_DEBUG_LEVEL_TRACE */


#define OTV_RUN  otvalid->func[0]


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       COVERAGE TABLE                          *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_Coverage_validate( FT_TS_Bytes       table,
                         OTV_Validator  otvalid,
                         FT_TS_Int         expected_count );

  /* return first covered glyph */
  FT_TS_LOCAL( FT_TS_UInt )
  otv_Coverage_get_first( FT_TS_Bytes  table );

  /* return last covered glyph */
  FT_TS_LOCAL( FT_TS_UInt )
  otv_Coverage_get_last( FT_TS_Bytes  table );

  /* return number of covered glyphs */
  FT_TS_LOCAL( FT_TS_UInt )
  otv_Coverage_get_count( FT_TS_Bytes  table );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                  CLASS DEFINITION TABLE                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_ClassDef_validate( FT_TS_Bytes       table,
                         OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      DEVICE TABLE                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_Device_validate( FT_TS_Bytes       table,
                       OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           LOOKUPS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_Lookup_validate( FT_TS_Bytes       table,
                       OTV_Validator  otvalid );

  FT_TS_LOCAL( void )
  otv_LookupList_validate( FT_TS_Bytes       table,
                           OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                        FEATURES                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_Feature_validate( FT_TS_Bytes       table,
                        OTV_Validator  otvalid );

  /* lookups must already be validated */
  FT_TS_LOCAL( void )
  otv_FeatureList_validate( FT_TS_Bytes       table,
                            FT_TS_Bytes       lookups,
                            OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       LANGUAGE SYSTEM                         *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_LangSys_validate( FT_TS_Bytes       table,
                        OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                           SCRIPTS                             *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_LOCAL( void )
  otv_Script_validate( FT_TS_Bytes       table,
                       OTV_Validator  otvalid );

  /* features must already be validated */
  FT_TS_LOCAL( void )
  otv_ScriptList_validate( FT_TS_Bytes       table,
                           FT_TS_Bytes       features,
                           OTV_Validator  otvalid );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      UTILITY FUNCTIONS                        *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

#define ChainPosClassSetFunc  otv_x_Ox
#define ChainPosRuleSetFunc   otv_x_Ox
#define ChainSubClassSetFunc  otv_x_Ox
#define ChainSubRuleSetFunc   otv_x_Ox
#define JstfLangSysFunc       otv_x_Ox
#define JstfMaxFunc           otv_x_Ox
#define LigGlyphFunc          otv_x_Ox
#define LigatureArrayFunc     otv_x_Ox
#define LigatureSetFunc       otv_x_Ox
#define PosClassSetFunc       otv_x_Ox
#define PosRuleSetFunc        otv_x_Ox
#define SubClassSetFunc       otv_x_Ox
#define SubRuleSetFunc        otv_x_Ox

  FT_TS_LOCAL( void )
  otv_x_Ox ( FT_TS_Bytes       table,
             OTV_Validator  otvalid );

#define AlternateSubstFormat1Func     otv_u_C_x_Ox
#define ChainContextPosFormat1Func    otv_u_C_x_Ox
#define ChainContextSubstFormat1Func  otv_u_C_x_Ox
#define ContextPosFormat1Func         otv_u_C_x_Ox
#define ContextSubstFormat1Func       otv_u_C_x_Ox
#define LigatureSubstFormat1Func      otv_u_C_x_Ox
#define MultipleSubstFormat1Func      otv_u_C_x_Ox

  FT_TS_LOCAL( void )
  otv_u_C_x_Ox( FT_TS_Bytes       table,
                OTV_Validator  otvalid );

#define AlternateSetFunc     otv_x_ux
#define AttachPointFunc      otv_x_ux
#define ExtenderGlyphFunc    otv_x_ux
#define JstfGPOSModListFunc  otv_x_ux
#define JstfGSUBModListFunc  otv_x_ux
#define SequenceFunc         otv_x_ux

  FT_TS_LOCAL( void )
  otv_x_ux( FT_TS_Bytes       table,
            OTV_Validator  otvalid );

#define PosClassRuleFunc  otv_x_y_ux_sy
#define PosRuleFunc       otv_x_y_ux_sy
#define SubClassRuleFunc  otv_x_y_ux_sy
#define SubRuleFunc       otv_x_y_ux_sy

  FT_TS_LOCAL( void )
  otv_x_y_ux_sy( FT_TS_Bytes       table,
                 OTV_Validator  otvalid );

#define ChainPosClassRuleFunc  otv_x_ux_y_uy_z_uz_p_sp
#define ChainPosRuleFunc       otv_x_ux_y_uy_z_uz_p_sp
#define ChainSubClassRuleFunc  otv_x_ux_y_uy_z_uz_p_sp
#define ChainSubRuleFunc       otv_x_ux_y_uy_z_uz_p_sp

  FT_TS_LOCAL( void )
  otv_x_ux_y_uy_z_uz_p_sp( FT_TS_Bytes       table,
                           OTV_Validator  otvalid );

#define ContextPosFormat2Func    otv_u_O_O_x_Onx
#define ContextSubstFormat2Func  otv_u_O_O_x_Onx

  FT_TS_LOCAL( void )
  otv_u_O_O_x_Onx( FT_TS_Bytes       table,
                   OTV_Validator  otvalid );

#define ContextPosFormat3Func    otv_u_x_y_Ox_sy
#define ContextSubstFormat3Func  otv_u_x_y_Ox_sy

  FT_TS_LOCAL( void )
  otv_u_x_y_Ox_sy( FT_TS_Bytes       table,
                   OTV_Validator  otvalid );

#define ChainContextPosFormat2Func    otv_u_O_O_O_O_x_Onx
#define ChainContextSubstFormat2Func  otv_u_O_O_O_O_x_Onx

  FT_TS_LOCAL( void )
  otv_u_O_O_O_O_x_Onx( FT_TS_Bytes       table,
                       OTV_Validator  otvalid );

#define ChainContextPosFormat3Func    otv_u_x_Ox_y_Oy_z_Oz_p_sp
#define ChainContextSubstFormat3Func  otv_u_x_Ox_y_Oy_z_Oz_p_sp

  FT_TS_LOCAL( void )
  otv_u_x_Ox_y_Oy_z_Oz_p_sp( FT_TS_Bytes       table,
                             OTV_Validator  otvalid );


  FT_TS_LOCAL( FT_TS_UInt )
  otv_GSUBGPOS_get_Lookup_count( FT_TS_Bytes  table );

  FT_TS_LOCAL( FT_TS_UInt )
  otv_GSUBGPOS_have_MarkAttachmentType_flag( FT_TS_Bytes  table );

 /* */

FT_TS_END_HEADER

#endif /* OTVCOMMN_H_ */


/* END */
