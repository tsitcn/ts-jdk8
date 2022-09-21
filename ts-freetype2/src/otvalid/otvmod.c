/****************************************************************************
 *
 * otvmod.c
 *
 *   FreeType's OpenType validation module implementation (body).
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


#include <freetype/tttables.h>
#include <freetype/tttags.h>
#include <freetype/ftotval.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svotval.h>

#include "otvmod.h"
#include "otvalid.h"
#include "otvcommn.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  otvmodule


  static FT_TS_Error
  otv_load_table( FT_TS_Face             face,
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


  static FT_TS_Error
  otv_validate( FT_TS_Face volatile   face,
                FT_TS_UInt            ot_flags,
                FT_TS_Bytes          *ot_base,
                FT_TS_Bytes          *ot_gdef,
                FT_TS_Bytes          *ot_gpos,
                FT_TS_Bytes          *ot_gsub,
                FT_TS_Bytes          *ot_jstf )
  {
    FT_TS_Error                  error = FT_TS_Err_Ok;
    FT_TS_Byte* volatile         base;
    FT_TS_Byte* volatile         gdef;
    FT_TS_Byte* volatile         gpos;
    FT_TS_Byte* volatile         gsub;
    FT_TS_Byte* volatile         jstf;
    FT_TS_Byte* volatile         math;
    FT_TS_ULong                  len_base, len_gdef, len_gpos, len_gsub, len_jstf;
    FT_TS_ULong                  len_math;
    FT_TS_UInt                   num_glyphs = (FT_TS_UInt)face->num_glyphs;
    FT_TS_ValidatorRec volatile  valid;


    base     = gdef     = gpos     = gsub     = jstf     = math     = NULL;
    len_base = len_gdef = len_gpos = len_gsub = len_jstf = len_math = 0;

    /*
     * XXX: OpenType tables cannot handle 32-bit glyph index,
     *      although broken TrueType can have 32-bit glyph index.
     */
    if ( face->num_glyphs > 0xFFFFL )
    {
      FT_TS_TRACE1(( "otv_validate: Invalid glyphs index (0x0000FFFF - 0x%08lx) ",
                  face->num_glyphs ));
      FT_TS_TRACE1(( "are not handled by OpenType tables\n" ));
      num_glyphs = 0xFFFF;
    }

    /* load tables */

    if ( ot_flags & FT_TS_VALIDATE_BASE )
    {
      error = otv_load_table( face, TTAG_BASE, &base, &len_base );
      if ( error )
        goto Exit;
    }

    if ( ot_flags & FT_TS_VALIDATE_GDEF )
    {
      error = otv_load_table( face, TTAG_GDEF, &gdef, &len_gdef );
      if ( error )
        goto Exit;
    }

    if ( ot_flags & FT_TS_VALIDATE_GPOS )
    {
      error = otv_load_table( face, TTAG_GPOS, &gpos, &len_gpos );
      if ( error )
        goto Exit;
    }

    if ( ot_flags & FT_TS_VALIDATE_GSUB )
    {
      error = otv_load_table( face, TTAG_GSUB, &gsub, &len_gsub );
      if ( error )
        goto Exit;
    }

    if ( ot_flags & FT_TS_VALIDATE_JSTF )
    {
      error = otv_load_table( face, TTAG_JSTF, &jstf, &len_jstf );
      if ( error )
        goto Exit;
    }

    if ( ot_flags & FT_TS_VALIDATE_MATH )
    {
      error = otv_load_table( face, TTAG_MATH, &math, &len_math );
      if ( error )
        goto Exit;
    }

    /* validate tables */

    if ( base )
    {
      ft_validator_init( &valid, base, base + len_base, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_BASE_validate( base, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    if ( gpos )
    {
      ft_validator_init( &valid, gpos, gpos + len_gpos, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_GPOS_validate( gpos, num_glyphs, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    if ( gsub )
    {
      ft_validator_init( &valid, gsub, gsub + len_gsub, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_GSUB_validate( gsub, num_glyphs, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    if ( gdef )
    {
      ft_validator_init( &valid, gdef, gdef + len_gdef, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_GDEF_validate( gdef, gsub, gpos, num_glyphs, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    if ( jstf )
    {
      ft_validator_init( &valid, jstf, jstf + len_jstf, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_JSTF_validate( jstf, gsub, gpos, num_glyphs, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    if ( math )
    {
      ft_validator_init( &valid, math, math + len_math, FT_TS_VALIDATE_DEFAULT );
      if ( ft_setjmp( valid.jump_buffer ) == 0 )
        otv_MATH_validate( math, num_glyphs, &valid );
      error = valid.error;
      if ( error )
        goto Exit;
    }

    *ot_base = (FT_TS_Bytes)base;
    *ot_gdef = (FT_TS_Bytes)gdef;
    *ot_gpos = (FT_TS_Bytes)gpos;
    *ot_gsub = (FT_TS_Bytes)gsub;
    *ot_jstf = (FT_TS_Bytes)jstf;

  Exit:
    if ( error )
    {
      FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


      FT_TS_FREE( base );
      FT_TS_FREE( gdef );
      FT_TS_FREE( gpos );
      FT_TS_FREE( gsub );
      FT_TS_FREE( jstf );
    }

    {
      FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


      FT_TS_FREE( math );                 /* Can't return this as API is frozen */
    }

    return error;
  }


  static
  const FT_TS_Service_OTvalidateRec  otvalid_interface =
  {
    otv_validate        /* validate */
  };


  static
  const FT_TS_ServiceDescRec  otvalid_services[] =
  {
    { FT_TS_SERVICE_ID_OPENTYPE_VALIDATE, &otvalid_interface },
    { NULL, NULL }
  };


  static FT_TS_Pointer
  otvalid_get_service( FT_TS_Module    module,
                       const char*  service_id )
  {
    FT_TS_UNUSED( module );

    return ft_service_list_lookup( otvalid_services, service_id );
  }


  FT_TS_CALLBACK_TABLE_DEF
  const FT_TS_Module_Class  otv_module_class =
  {
    0,
    sizeof ( FT_TS_ModuleRec ),
    "otvalid",
    0x10000L,
    0x20000L,

    NULL,              /* module-specific interface */

    (FT_TS_Module_Constructor)NULL,                /* module_init   */
    (FT_TS_Module_Destructor) NULL,                /* module_done   */
    (FT_TS_Module_Requester)  otvalid_get_service  /* get_interface */
  };


/* END */
