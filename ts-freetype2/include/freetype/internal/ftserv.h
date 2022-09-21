/****************************************************************************
 *
 * ftserv.h
 *
 *   The FreeType services (specification only).
 *
 * Copyright (C) 2003-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

  /**************************************************************************
   *
   * Each module can export one or more 'services'.  Each service is
   * identified by a constant string and modeled by a pointer; the latter
   * generally corresponds to a structure containing function pointers.
   *
   * Note that a service's data cannot be a mere function pointer because in
   * C it is possible that function pointers might be implemented differently
   * than data pointers (e.g. 48 bits instead of 32).
   *
   */


#ifndef FTSERV_H_
#define FTSERV_H_

#include "compiler-macros.h"

FT_TS_BEGIN_HEADER

  /**************************************************************************
   *
   * @macro:
   *   FT_TS_FACE_FIND_SERVICE
   *
   * @description:
   *   This macro is used to look up a service from a face's driver module.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   id ::
   *     A string describing the service as defined in the service's header
   *     files (e.g. FT_TS_SERVICE_ID_MULTI_MASTERS which expands to
   *     'multi-masters').  It is automatically prefixed with
   *     `FT_TS_SERVICE_ID_`.
   *
   * @output:
   *   ptr ::
   *     A variable that receives the service pointer.  Will be `NULL` if not
   *     found.
   */
#ifdef __cplusplus

#define FT_TS_FACE_FIND_SERVICE( face, ptr, id )                               \
  FT_TS_BEGIN_STMNT                                                            \
    FT_TS_Module    module = FT_TS_MODULE( FT_TS_FACE( face )->driver );             \
    FT_TS_Pointer   _tmp_  = NULL;                                             \
    FT_TS_Pointer*  _pptr_ = (FT_TS_Pointer*)&(ptr);                              \
                                                                            \
                                                                            \
    if ( module->clazz->get_interface )                                     \
      _tmp_ = module->clazz->get_interface( module, FT_TS_SERVICE_ID_ ## id ); \
    *_pptr_ = _tmp_;                                                        \
  FT_TS_END_STMNT

#else /* !C++ */

#define FT_TS_FACE_FIND_SERVICE( face, ptr, id )                               \
  FT_TS_BEGIN_STMNT                                                            \
    FT_TS_Module   module = FT_TS_MODULE( FT_TS_FACE( face )->driver );              \
    FT_TS_Pointer  _tmp_  = NULL;                                              \
                                                                            \
    if ( module->clazz->get_interface )                                     \
      _tmp_ = module->clazz->get_interface( module, FT_TS_SERVICE_ID_ ## id ); \
    ptr = _tmp_;                                                            \
  FT_TS_END_STMNT

#endif /* !C++ */


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_FACE_FIND_GLOBAL_SERVICE
   *
   * @description:
   *   This macro is used to look up a service from all modules.
   *
   * @input:
   *   face ::
   *     The source face handle.
   *
   *   id ::
   *     A string describing the service as defined in the service's header
   *     files (e.g. FT_TS_SERVICE_ID_MULTI_MASTERS which expands to
   *     'multi-masters').  It is automatically prefixed with
   *     `FT_TS_SERVICE_ID_`.
   *
   * @output:
   *   ptr ::
   *     A variable that receives the service pointer.  Will be `NULL` if not
   *     found.
   */
#ifdef __cplusplus

#define FT_TS_FACE_FIND_GLOBAL_SERVICE( face, ptr, id )                  \
  FT_TS_BEGIN_STMNT                                                      \
    FT_TS_Module    module = FT_TS_MODULE( FT_TS_FACE( face )->driver );       \
    FT_TS_Pointer   _tmp_;                                               \
    FT_TS_Pointer*  _pptr_ = (FT_TS_Pointer*)&(ptr);                        \
                                                                      \
                                                                      \
    _tmp_ = ft_module_get_service( module, FT_TS_SERVICE_ID_ ## id, 1 ); \
    *_pptr_ = _tmp_;                                                  \
  FT_TS_END_STMNT

#else /* !C++ */

#define FT_TS_FACE_FIND_GLOBAL_SERVICE( face, ptr, id )                  \
  FT_TS_BEGIN_STMNT                                                      \
    FT_TS_Module   module = FT_TS_MODULE( FT_TS_FACE( face )->driver );        \
    FT_TS_Pointer  _tmp_;                                                \
                                                                      \
                                                                      \
    _tmp_ = ft_module_get_service( module, FT_TS_SERVICE_ID_ ## id, 1 ); \
    ptr   = _tmp_;                                                    \
  FT_TS_END_STMNT

#endif /* !C++ */


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****         S E R V I C E   D E S C R I P T O R S                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * The following structure is used to _describe_ a given service to the
   * library.  This is useful to build simple static service lists.
   */
  typedef struct  FT_TS_ServiceDescRec_
  {
    const char*  serv_id;     /* service name         */
    const void*  serv_data;   /* service pointer/data */

  } FT_TS_ServiceDescRec;

  typedef const FT_TS_ServiceDescRec*  FT_TS_ServiceDesc;


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_DEFINE_SERVICEDESCREC1
   *   FT_TS_DEFINE_SERVICEDESCREC2
   *   FT_TS_DEFINE_SERVICEDESCREC3
   *   FT_TS_DEFINE_SERVICEDESCREC4
   *   FT_TS_DEFINE_SERVICEDESCREC5
   *   FT_TS_DEFINE_SERVICEDESCREC6
   *   FT_TS_DEFINE_SERVICEDESCREC7
   *   FT_TS_DEFINE_SERVICEDESCREC8
   *   FT_TS_DEFINE_SERVICEDESCREC9
   *   FT_TS_DEFINE_SERVICEDESCREC10
   *
   * @description:
   *   Used to initialize an array of FT_TS_ServiceDescRec structures.
   *
   *   The array will be allocated in the global scope (or the scope where
   *   the macro is used).
   */
#define FT_TS_DEFINE_SERVICEDESCREC1( class_,                                  \
                                   serv_id_1, serv_data_1 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC2( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC3( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC4( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC5( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4,                  \
                                   serv_id_5, serv_data_5 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC6( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4,                  \
                                   serv_id_5, serv_data_5,                  \
                                   serv_id_6, serv_data_6 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { serv_id_6, serv_data_6 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC7( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4,                  \
                                   serv_id_5, serv_data_5,                  \
                                   serv_id_6, serv_data_6,                  \
                                   serv_id_7, serv_data_7 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { serv_id_6, serv_data_6 },                                             \
    { serv_id_7, serv_data_7 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC8( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4,                  \
                                   serv_id_5, serv_data_5,                  \
                                   serv_id_6, serv_data_6,                  \
                                   serv_id_7, serv_data_7,                  \
                                   serv_id_8, serv_data_8 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { serv_id_6, serv_data_6 },                                             \
    { serv_id_7, serv_data_7 },                                             \
    { serv_id_8, serv_data_8 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC9( class_,                                  \
                                   serv_id_1, serv_data_1,                  \
                                   serv_id_2, serv_data_2,                  \
                                   serv_id_3, serv_data_3,                  \
                                   serv_id_4, serv_data_4,                  \
                                   serv_id_5, serv_data_5,                  \
                                   serv_id_6, serv_data_6,                  \
                                   serv_id_7, serv_data_7,                  \
                                   serv_id_8, serv_data_8,                  \
                                   serv_id_9, serv_data_9 )                 \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { serv_id_6, serv_data_6 },                                             \
    { serv_id_7, serv_data_7 },                                             \
    { serv_id_8, serv_data_8 },                                             \
    { serv_id_9, serv_data_9 },                                             \
    { NULL, NULL }                                                          \
  };

#define FT_TS_DEFINE_SERVICEDESCREC10( class_,                                 \
                                    serv_id_1, serv_data_1,                 \
                                    serv_id_2, serv_data_2,                 \
                                    serv_id_3, serv_data_3,                 \
                                    serv_id_4, serv_data_4,                 \
                                    serv_id_5, serv_data_5,                 \
                                    serv_id_6, serv_data_6,                 \
                                    serv_id_7, serv_data_7,                 \
                                    serv_id_8, serv_data_8,                 \
                                    serv_id_9, serv_data_9,                 \
                                    serv_id_10, serv_data_10 )              \
  static const FT_TS_ServiceDescRec  class_[] =                                \
  {                                                                         \
    { serv_id_1, serv_data_1 },                                             \
    { serv_id_2, serv_data_2 },                                             \
    { serv_id_3, serv_data_3 },                                             \
    { serv_id_4, serv_data_4 },                                             \
    { serv_id_5, serv_data_5 },                                             \
    { serv_id_6, serv_data_6 },                                             \
    { serv_id_7, serv_data_7 },                                             \
    { serv_id_8, serv_data_8 },                                             \
    { serv_id_9, serv_data_9 },                                             \
    { serv_id_10, serv_data_10 },                                           \
    { NULL, NULL }                                                          \
  };


  /*
   * Parse a list of FT_TS_ServiceDescRec descriptors and look for a specific
   * service by ID.  Note that the last element in the array must be { NULL,
   * NULL }, and that the function should return NULL if the service isn't
   * available.
   *
   * This function can be used by modules to implement their `get_service'
   * method.
   */
  FT_TS_BASE( FT_TS_Pointer )
  ft_service_list_lookup( FT_TS_ServiceDesc  service_descriptors,
                          const char*     service_id );


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****             S E R V I C E S   C A C H E                       *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  /*
   * This structure is used to store a cache for several frequently used
   * services.  It is the type of `face->internal->services'.  You should
   * only use FT_TS_FACE_LOOKUP_SERVICE to access it.
   *
   * All fields should have the type FT_TS_Pointer to relax compilation
   * dependencies.  We assume the developer isn't completely stupid.
   *
   * Each field must be named `service_XXXX' where `XXX' corresponds to the
   * correct FT_TS_SERVICE_ID_XXXX macro.  See the definition of
   * FT_TS_FACE_LOOKUP_SERVICE below how this is implemented.
   *
   */
  typedef struct  FT_TS_ServiceCacheRec_
  {
    FT_TS_Pointer  service_POSTSCRIPT_FONT_NAME;
    FT_TS_Pointer  service_MULTI_MASTERS;
    FT_TS_Pointer  service_METRICS_VARIATIONS;
    FT_TS_Pointer  service_GLYPH_DICT;
    FT_TS_Pointer  service_PFR_METRICS;
    FT_TS_Pointer  service_WINFNT;

  } FT_TS_ServiceCacheRec, *FT_TS_ServiceCache;


  /*
   * A magic number used within the services cache.
   */

  /* ensure that value `1' has the same width as a pointer */
#define FT_TS_SERVICE_UNAVAILABLE  ((FT_TS_Pointer)~(FT_TS_PtrDist)1)


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_FACE_LOOKUP_SERVICE
   *
   * @description:
   *   This macro is used to look up a service from a face's driver module
   *   using its cache.
   *
   * @input:
   *   face ::
   *     The source face handle containing the cache.
   *
   *   field ::
   *     The field name in the cache.
   *
   *   id ::
   *     The service ID.
   *
   * @output:
   *   ptr ::
   *     A variable receiving the service data.  `NULL` if not available.
   */
#ifdef __cplusplus

#define FT_TS_FACE_LOOKUP_SERVICE( face, ptr, id )                \
  FT_TS_BEGIN_STMNT                                               \
    FT_TS_Pointer   svc;                                          \
    FT_TS_Pointer*  Pptr = (FT_TS_Pointer*)&(ptr);                   \
                                                               \
                                                               \
    svc = FT_TS_FACE( face )->internal->services. service_ ## id; \
    if ( svc == FT_TS_SERVICE_UNAVAILABLE )                       \
      svc = NULL;                                              \
    else if ( svc == NULL )                                    \
    {                                                          \
      FT_TS_FACE_FIND_SERVICE( face, svc, id );                   \
                                                               \
      FT_TS_FACE( face )->internal->services. service_ ## id =    \
        (FT_TS_Pointer)( svc != NULL ? svc                        \
                                  : FT_TS_SERVICE_UNAVAILABLE );  \
    }                                                          \
    *Pptr = svc;                                               \
  FT_TS_END_STMNT

#else /* !C++ */

#define FT_TS_FACE_LOOKUP_SERVICE( face, ptr, id )                \
  FT_TS_BEGIN_STMNT                                               \
    FT_TS_Pointer  svc;                                           \
                                                               \
                                                               \
    svc = FT_TS_FACE( face )->internal->services. service_ ## id; \
    if ( svc == FT_TS_SERVICE_UNAVAILABLE )                       \
      svc = NULL;                                              \
    else if ( svc == NULL )                                    \
    {                                                          \
      FT_TS_FACE_FIND_SERVICE( face, svc, id );                   \
                                                               \
      FT_TS_FACE( face )->internal->services. service_ ## id =    \
        (FT_TS_Pointer)( svc != NULL ? svc                        \
                                  : FT_TS_SERVICE_UNAVAILABLE );  \
    }                                                          \
    ptr = svc;                                                 \
  FT_TS_END_STMNT

#endif /* !C++ */

  /*
   * A macro used to define new service structure types.
   */

#define FT_TS_DEFINE_SERVICE( name )            \
  typedef struct FT_TS_Service_ ## name ## Rec_ \
    FT_TS_Service_ ## name ## Rec ;             \
  typedef struct FT_TS_Service_ ## name ## Rec_ \
    const * FT_TS_Service_ ## name ;            \
  struct FT_TS_Service_ ## name ## Rec_

  /* */

FT_TS_END_HEADER

#endif /* FTSERV_H_ */


/* END */
