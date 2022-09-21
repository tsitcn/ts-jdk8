/****************************************************************************
 *
 * ftmm.c
 *
 *   Multiple Master font support (body).
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


#include <freetype/internal/ftdebug.h>

#include <freetype/ftmm.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/internal/services/svmm.h>
#include <freetype/internal/services/svmetric.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  mm


  static FT_TS_Error
  ft_face_get_mm_service( FT_TS_Face                   face,
                          FT_TS_Service_MultiMasters  *aservice )
  {
    FT_TS_Error  error;


    *aservice = NULL;

    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    error = FT_TS_ERR( Invalid_Argument );

    if ( FT_TS_HAS_MULTIPLE_MASTERS( face ) )
    {
      FT_TS_FACE_LOOKUP_SERVICE( face,
                              *aservice,
                              MULTI_MASTERS );

      if ( *aservice )
        error = FT_TS_Err_Ok;
    }

    return error;
  }


  static FT_TS_Error
  ft_face_get_mvar_service( FT_TS_Face                        face,
                            FT_TS_Service_MetricsVariations  *aservice )
  {
    FT_TS_Error  error;


    *aservice = NULL;

    if ( !face )
      return FT_TS_THROW( Invalid_Face_Handle );

    error = FT_TS_ERR( Invalid_Argument );

    if ( FT_TS_HAS_MULTIPLE_MASTERS( face ) )
    {
      FT_TS_FACE_LOOKUP_SERVICE( face,
                              *aservice,
                              METRICS_VARIATIONS );

      if ( *aservice )
        error = FT_TS_Err_Ok;
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Multi_Master( FT_TS_Face           face,
                       FT_TS_Multi_Master  *amaster )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( !amaster )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_mm )
        error = service->get_mm( face, amaster );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_MM_Var( FT_TS_Face      face,
                 FT_TS_MM_Var*  *amaster )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( !amaster )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_mm_var )
        error = service->get_mm_var( face, amaster );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Done_MM_Var( FT_TS_Library  library,
                  FT_TS_MM_Var*  amaster )
  {
    FT_TS_Memory  memory;


    if ( !library )
      return FT_TS_THROW( Invalid_Library_Handle );

    memory = library->memory;
    FT_TS_FREE( amaster );

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_MM_Design_Coordinates( FT_TS_Face   face,
                                FT_TS_UInt   num_coords,
                                FT_TS_Long*  coords )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( num_coords && !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->set_mm_design )
        error = service->set_mm_design( face, num_coords, coords );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_MM_WeightVector( FT_TS_Face    face,
                          FT_TS_UInt    len,
                          FT_TS_Fixed*  weightvector )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( len && !weightvector )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->set_mm_weightvector )
        error = service->set_mm_weightvector( face, len, weightvector );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    return error;
  }


  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_MM_WeightVector( FT_TS_Face    face,
                          FT_TS_UInt*   len,
                          FT_TS_Fixed*  weightvector )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( len && !weightvector )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_mm_weightvector )
        error = service->get_mm_weightvector( face, len, weightvector );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Var_Design_Coordinates( FT_TS_Face    face,
                                 FT_TS_UInt    num_coords,
                                 FT_TS_Fixed*  coords )
  {
    FT_TS_Error                      error;
    FT_TS_Service_MultiMasters       service_mm   = NULL;
    FT_TS_Service_MetricsVariations  service_mvar = NULL;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( num_coords && !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service_mm );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service_mm->set_var_design )
        error = service_mm->set_var_design( face, num_coords, coords );

      /* internal error code -1 means `no change'; we can exit immediately */
      if ( error == -1 )
        return FT_TS_Err_Ok;
    }

    if ( !error )
    {
      (void)ft_face_get_mvar_service( face, &service_mvar );

      if ( service_mvar && service_mvar->metrics_adjust )
        service_mvar->metrics_adjust( face );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Var_Design_Coordinates( FT_TS_Face    face,
                                 FT_TS_UInt    num_coords,
                                 FT_TS_Fixed*  coords )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_var_design )
        error = service->get_var_design( face, num_coords, coords );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_MM_Blend_Coordinates( FT_TS_Face    face,
                               FT_TS_UInt    num_coords,
                               FT_TS_Fixed*  coords )
  {
    FT_TS_Error                      error;
    FT_TS_Service_MultiMasters       service_mm   = NULL;
    FT_TS_Service_MetricsVariations  service_mvar = NULL;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( num_coords && !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service_mm );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service_mm->set_mm_blend )
        error = service_mm->set_mm_blend( face, num_coords, coords );

      /* internal error code -1 means `no change'; we can exit immediately */
      if ( error == -1 )
        return FT_TS_Err_Ok;
    }

    if ( !error )
    {
      (void)ft_face_get_mvar_service( face, &service_mvar );

      if ( service_mvar && service_mvar->metrics_adjust )
        service_mvar->metrics_adjust( face );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    return error;
  }


  /* documentation is in ftmm.h */

  /* This is exactly the same as the previous function.  It exists for */
  /* orthogonality.                                                    */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Var_Blend_Coordinates( FT_TS_Face    face,
                                FT_TS_UInt    num_coords,
                                FT_TS_Fixed*  coords )
  {
    FT_TS_Error                      error;
    FT_TS_Service_MultiMasters       service_mm   = NULL;
    FT_TS_Service_MetricsVariations  service_mvar = NULL;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( num_coords && !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service_mm );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service_mm->set_mm_blend )
        error = service_mm->set_mm_blend( face, num_coords, coords );

      /* internal error code -1 means `no change'; we can exit immediately */
      if ( error == -1 )
        return FT_TS_Err_Ok;
    }

    if ( !error )
    {
      (void)ft_face_get_mvar_service( face, &service_mvar );

      if ( service_mvar && service_mvar->metrics_adjust )
        service_mvar->metrics_adjust( face );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_MM_Blend_Coordinates( FT_TS_Face    face,
                               FT_TS_UInt    num_coords,
                               FT_TS_Fixed*  coords )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_mm_blend )
        error = service->get_mm_blend( face, num_coords, coords );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  /* This is exactly the same as the previous function.  It exists for */
  /* orthogonality.                                                    */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Var_Blend_Coordinates( FT_TS_Face    face,
                                FT_TS_UInt    num_coords,
                                FT_TS_Fixed*  coords )
  {
    FT_TS_Error                 error;
    FT_TS_Service_MultiMasters  service;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    if ( !coords )
      return FT_TS_THROW( Invalid_Argument );

    error = ft_face_get_mm_service( face, &service );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service->get_mm_blend )
        error = service->get_mm_blend( face, num_coords, coords );
    }

    return error;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Get_Var_Axis_Flags( FT_TS_MM_Var*  master,
                         FT_TS_UInt     axis_index,
                         FT_TS_UInt*    flags )
  {
    FT_TS_UShort*  axis_flags;


    if ( !master || !flags )
      return FT_TS_THROW( Invalid_Argument );

    if ( axis_index >= master->num_axis )
      return FT_TS_THROW( Invalid_Argument );

    /* the axis flags array immediately follows the data of `master' */
    axis_flags = (FT_TS_UShort*)&( master[1] );
    *flags     = axis_flags[axis_index];

    return FT_TS_Err_Ok;
  }


  /* documentation is in ftmm.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_Set_Named_Instance( FT_TS_Face  face,
                         FT_TS_UInt  instance_index )
  {
    FT_TS_Error  error;

    FT_TS_Service_MultiMasters       service_mm   = NULL;
    FT_TS_Service_MetricsVariations  service_mvar = NULL;


    /* check of `face' delayed to `ft_face_get_mm_service' */

    error = ft_face_get_mm_service( face, &service_mm );
    if ( !error )
    {
      error = FT_TS_ERR( Invalid_Argument );
      if ( service_mm->set_instance )
        error = service_mm->set_instance( face, instance_index );
    }

    if ( !error )
    {
      (void)ft_face_get_mvar_service( face, &service_mvar );

      if ( service_mvar && service_mvar->metrics_adjust )
        service_mvar->metrics_adjust( face );
    }

    /* enforce recomputation of auto-hinting data */
    if ( !error && face->autohint.finalizer )
    {
      face->autohint.finalizer( face->autohint.data );
      face->autohint.data = NULL;
    }

    if ( !error )
    {
      face->face_index  = ( instance_index << 16 )        |
                          ( face->face_index & 0xFFFFL );
      face->face_flags &= ~FT_TS_FACE_FLAG_VARIATION;
    }

    return error;
  }


/* END */
