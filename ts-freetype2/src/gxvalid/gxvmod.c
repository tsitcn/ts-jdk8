/****************************************************************************
 *
 * gxvmod.c
 *
 *   FreeType's TrueTypeGX/AAT validation module implementation (body).
 *
 * Copyright (C) 2004-2022 by
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


#include <freetype/tttables.h>
#include <freetype/tttags.h>
#include <freetype/ftgxval.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svgxval.h>

#include "gxvmod.h"
#include "gxvalid.h"
#include "gxvcommn.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvmodule


  static FT_TS_Error
  gxv_load_table( FT_TS_Face             face,
                  FT_TS_Tag              tag,
                  FT_TS_Byte* volatile*  table,
                  FT_TS_ULong*           table_len )
  {
    FT_TS_Error   error;
    FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


    error = FT_TS_Load_Sfnt_Table( face, tag, 0, NULL, table_len );
    if ( FT_TS_ERR_EQ( error, Table_Missing ) )
      return FT_TS_Err_Ok;
    if ( error )
      goto Exit;

    if ( FT_TS_QALLOC( *table, *table_len ) )
      goto Exit;

    error = FT_TS_Load_Sfnt_Table( face, tag, 0, *table, table_len );

  Exit:
    return error;
  }


#define GXV_TABLE_DECL( _sfnt )                     \
          FT_TS_Byte* volatile  _sfnt          = NULL; \
          FT_TS_ULong            len_ ## _sfnt = 0

#define GXV_TABLE_LOAD( _sfnt )                                       \
          FT_TS_BEGIN_STMNT                                              \
            if ( ( FT_TS_VALIDATE_ ## _sfnt ## _INDEX < table_count ) && \
                 ( gx_flags & FT_TS_VALIDATE_ ## _sfnt )            )    \
            {                                                         \
              error = gxv_load_table( face, TTAG_ ## _sfnt,           \
                                      &_sfnt, &len_ ## _sfnt );       \
              if ( error )                                            \
                goto Exit;                                            \
            }                                                         \
          FT_TS_END_STMNT

#define GXV_TABLE_VALIDATE( _sfnt )                                    \
          FT_TS_BEGIN_STMNT                                               \
            if ( _sfnt )                                               \
            {                                                          \
              ft_validator_init( &valid, _sfnt, _sfnt + len_ ## _sfnt, \
                                 FT_TS_VALIDATE_DEFAULT );                \
              if ( ft_setjmp( valid.jump_buffer ) == 0 )               \
                gxv_ ## _sfnt ## _validate( _sfnt, face, &valid );     \
              error = valid.error;                                     \
              if ( error )                                             \
                goto Exit;                                             \
            }                                                          \
          FT_TS_END_STMNT

#define GXV_TABLE_SET( _sfnt )                                        \
          if ( FT_TS_VALIDATE_ ## _sfnt ## _INDEX < table_count )        \
            tables[FT_TS_VALIDATE_ ## _sfnt ## _INDEX] = (FT_TS_Bytes)_sfnt


  static FT_TS_Error
  gxv_validate( FT_TS_Face   face,
                FT_TS_UInt   gx_flags,
                FT_TS_Bytes  tables[FT_TS_VALIDATE_GX_LENGTH],
                FT_TS_UInt   table_count )
  {
    FT_TS_Memory volatile        memory = FT_TS_FACE_MEMORY( face );

    FT_TS_Error                  error = FT_TS_Err_Ok;
    FT_TS_ValidatorRec volatile  valid;

    FT_TS_UInt  i;


    GXV_TABLE_DECL( feat );
    GXV_TABLE_DECL( bsln );
    GXV_TABLE_DECL( trak );
    GXV_TABLE_DECL( just );
    GXV_TABLE_DECL( mort );
    GXV_TABLE_DECL( morx );
    GXV_TABLE_DECL( kern );
    GXV_TABLE_DECL( opbd );
    GXV_TABLE_DECL( prop );
    GXV_TABLE_DECL( lcar );

    for ( i = 0; i < table_count; i++ )
      tables[i] = 0;

    /* load tables */
    GXV_TABLE_LOAD( feat );
    GXV_TABLE_LOAD( bsln );
    GXV_TABLE_LOAD( trak );
    GXV_TABLE_LOAD( just );
    GXV_TABLE_LOAD( mort );
    GXV_TABLE_LOAD( morx );
    GXV_TABLE_LOAD( kern );
    GXV_TABLE_LOAD( opbd );
    GXV_TABLE_LOAD( prop );
    GXV_TABLE_LOAD( lcar );

    /* validate tables */
    GXV_TABLE_VALIDATE( feat );
    GXV_TABLE_VALIDATE( bsln );
    GXV_TABLE_VALIDATE( trak );
    GXV_TABLE_VALIDATE( just );
    GXV_TABLE_VALIDATE( mort );
    GXV_TABLE_VALIDATE( morx );
    GXV_TABLE_VALIDATE( kern );
    GXV_TABLE_VALIDATE( opbd );
    GXV_TABLE_VALIDATE( prop );
    GXV_TABLE_VALIDATE( lcar );

    /* Set results */
    GXV_TABLE_SET( feat );
    GXV_TABLE_SET( mort );
    GXV_TABLE_SET( morx );
    GXV_TABLE_SET( bsln );
    GXV_TABLE_SET( just );
    GXV_TABLE_SET( kern );
    GXV_TABLE_SET( opbd );
    GXV_TABLE_SET( trak );
    GXV_TABLE_SET( prop );
    GXV_TABLE_SET( lcar );

  Exit:
    if ( error )
    {
      FT_TS_FREE( feat );
      FT_TS_FREE( bsln );
      FT_TS_FREE( trak );
      FT_TS_FREE( just );
      FT_TS_FREE( mort );
      FT_TS_FREE( morx );
      FT_TS_FREE( kern );
      FT_TS_FREE( opbd );
      FT_TS_FREE( prop );
      FT_TS_FREE( lcar );
    }

    return error;
  }


  static FT_TS_Error
  classic_kern_validate( FT_TS_Face    face,
                         FT_TS_UInt    ckern_flags,
                         FT_TS_Bytes*  ckern_table )
  {
    FT_TS_Memory volatile        memory = FT_TS_FACE_MEMORY( face );

    FT_TS_Byte* volatile         ckern     = NULL;
    FT_TS_ULong                  len_ckern = 0;

    /* without volatile on `error' GCC 4.1.1. emits:                         */
    /*  warning: variable 'error' might be clobbered by 'longjmp' or 'vfork' */
    /* this warning seems spurious but ---                                   */
    FT_TS_Error volatile         error;
    FT_TS_ValidatorRec volatile  valid;


    *ckern_table = NULL;

    error = gxv_load_table( face, TTAG_kern, &ckern, &len_ckern );
    if ( error )
      goto Exit;

    if ( ckern )
    {
      ft_validator_init( &valid, ckern, ckern + len_ckern,
                         FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        gxv_kern_validate_classic( ckern, face,
                                   ckern_flags & FT_TS_VALIDATE_CKERN, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    *ckern_table = ckern;

  Exit:
    if ( error )
      FT_TS_FREE( ckern );

    return error;
  }


  static
  const FT_TS_Service_GXvalidateRec  gxvalid_interface =
  {
    gxv_validate              /* validate */
  };


  static
  const FT_TS_Service_CKERNvalidateRec  ckernvalid_interface =
  {
    classic_kern_validate     /* validate */
  };


  static
  const FT_TS_ServiceDescRec  gxvalid_services[] =
  {
    { FT_TS_SERVICE_ID_GX_VALIDATE,          &gxvalid_interface },
    { FT_TS_SERVICE_ID_CLASSICKERN_VALIDATE, &ckernvalid_interface },
    { NULL, NULL }
  };


  static FT_TS_Pointer
  gxvalid_get_service( FT_TS_Module    module,
                       const char*  service_id )
  {
    FT_TS_UNUSED( module );

    return ft_service_list_lookup( gxvalid_services, service_id );
  }


  FT_TS_CALLBACK_TABLE_DEF
  const FT_TS_Module_Class  gxv_module_class =
  {
    0,
    sizeof ( FT_TS_ModuleRec ),
    "gxvalid",
    0x10000L,
    0x20000L,

    NULL,              /* module-specific interface */

    (FT_TS_Module_Constructor)NULL,                /* module_init   */
    (FT_TS_Module_Destructor) NULL,                /* module_done   */
    (FT_TS_Module_Requester)  gxvalid_get_service  /* get_interface */
  };


/* END */
