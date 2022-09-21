/****************************************************************************
 *
 * fterrors.h
 *
 *   FreeType error code handling (specification).
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


  /**************************************************************************
   *
   * @section:
   *   error_enumerations
   *
   * @title:
   *   Error Enumerations
   *
   * @abstract:
   *   How to handle errors and error strings.
   *
   * @description:
   *   The header file `fterrors.h` (which is automatically included by
   *   `freetype.h` defines the handling of FreeType's enumeration
   *   constants.  It can also be used to generate error message strings
   *   with a small macro trick explained below.
   *
   *   **Error Formats**
   *
   *   The configuration macro `FT_TS_CONFIG_OPTION_USE_MODULE_ERRORS` can be
   *   defined in `ftoption.h` in order to make the higher byte indicate the
   *   module where the error has happened (this is not compatible with
   *   standard builds of FreeType~2, however).  See the file `ftmoderr.h`
   *   for more details.
   *
   *   **Error Message Strings**
   *
   *   Error definitions are set up with special macros that allow client
   *   applications to build a table of error message strings.  The strings
   *   are not included in a normal build of FreeType~2 to save space (most
   *   client applications do not use them).
   *
   *   To do so, you have to define the following macros before including
   *   this file.
   *
   *   ```
   *     FT_TS_ERROR_START_LIST
   *   ```
   *
   *   This macro is called before anything else to define the start of the
   *   error list.  It is followed by several `FT_TS_ERROR_DEF` calls.
   *
   *   ```
   *     FT_TS_ERROR_DEF( e, v, s )
   *   ```
   *
   *   This macro is called to define one single error.  'e' is the error
   *   code identifier (e.g., `Invalid_Argument`), 'v' is the error's
   *   numerical value, and 's' is the corresponding error string.
   *
   *   ```
   *     FT_TS_ERROR_END_LIST
   *   ```
   *
   *   This macro ends the list.
   *
   *   Additionally, you have to undefine `FTERRORS_H_` before #including
   *   this file.
   *
   *   Here is a simple example.
   *
   *   ```
   *     #undef FTERRORS_H_
   *     #define FT_TS_ERRORDEF( e, v, s )  { e, s },
   *     #define FT_TS_ERROR_START_LIST     {
   *     #define FT_TS_ERROR_END_LIST       { 0, NULL } };
   *
   *     const struct
   *     {
   *       int          err_code;
   *       const char*  err_msg;
   *     } ft_errors[] =
   *
   *     #include <freetype/fterrors.h>
   *   ```
   *
   *   An alternative to using an array is a switch statement.
   *
   *   ```
   *     #undef FTERRORS_H_
   *     #define FT_TS_ERROR_START_LIST     switch ( error_code ) {
   *     #define FT_TS_ERRORDEF( e, v, s )    case v: return s;
   *     #define FT_TS_ERROR_END_LIST       }
   *   ```
   *
   *   If you use `FT_TS_CONFIG_OPTION_USE_MODULE_ERRORS`, `error_code` should
   *   be replaced with `FT_TS_ERROR_BASE(error_code)` in the last example.
   */

  /* */

  /* In previous FreeType versions we used `__FTERRORS_H__`.  However, */
  /* using two successive underscores in a non-system symbol name      */
  /* violates the C (and C++) standard, so it was changed to the       */
  /* current form.  In spite of this, we have to make                  */
  /*                                                                   */
  /* ```                                                               */
  /*   #undefine __FTERRORS_H__                                        */
  /* ```                                                               */
  /*                                                                   */
  /* work for backward compatibility.                                  */
  /*                                                                   */
#if !( defined( FTERRORS_H_ ) && defined ( __FTERRORS_H__ ) )
#define FTERRORS_H_
#define __FTERRORS_H__


  /* include module base error codes */
#include <freetype/ftmoderr.h>


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                       SETUP MACROS                      *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/


#undef  FT_TS_NEED_EXTERN_C


  /* FT_TS_ERR_PREFIX is used as a prefix for error identifiers. */
  /* By default, we use `FT_TS_Err_`.                            */
  /*                                                          */
#ifndef FT_TS_ERR_PREFIX
#define FT_TS_ERR_PREFIX  FT_TS_Err_
#endif


  /* FT_TS_ERR_BASE is used as the base for module-specific errors. */
  /*                                                             */
#ifdef FT_TS_CONFIG_OPTION_USE_MODULE_ERRORS

#ifndef FT_TS_ERR_BASE
#define FT_TS_ERR_BASE  FT_TS_Mod_Err_Base
#endif

#else

#undef FT_TS_ERR_BASE
#define FT_TS_ERR_BASE  0

#endif /* FT_TS_CONFIG_OPTION_USE_MODULE_ERRORS */


  /* If FT_TS_ERRORDEF is not defined, we need to define a simple */
  /* enumeration type.                                         */
  /*                                                           */
#ifndef FT_TS_ERRORDEF

#define FT_TS_INCLUDE_ERR_PROTOS

#define FT_TS_ERRORDEF( e, v, s )  e = v,
#define FT_TS_ERROR_START_LIST     enum {
#define FT_TS_ERROR_END_LIST       FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, Max ) };

#ifdef __cplusplus
#define FT_TS_NEED_EXTERN_C
  extern "C" {
#endif

#endif /* !FT_TS_ERRORDEF */


  /* this macro is used to define an error */
#define FT_TS_ERRORDEF_( e, v, s )                                             \
          FT_TS_ERRORDEF( FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e ), v + FT_TS_ERR_BASE, s )

  /* this is only used for <module>_Err_Ok, which must be 0! */
#define FT_TS_NOERRORDEF_( e, v, s )                             \
          FT_TS_ERRORDEF( FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e ), v, s )


#ifdef FT_TS_ERROR_START_LIST
  FT_TS_ERROR_START_LIST
#endif


  /* now include the error codes */
#include <freetype/fterrdef.h>


#ifdef FT_TS_ERROR_END_LIST
  FT_TS_ERROR_END_LIST
#endif


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                      SIMPLE CLEANUP                     *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#ifdef FT_TS_NEED_EXTERN_C
  }
#endif

#undef FT_TS_ERROR_START_LIST
#undef FT_TS_ERROR_END_LIST

#undef FT_TS_ERRORDEF
#undef FT_TS_ERRORDEF_
#undef FT_TS_NOERRORDEF_

#undef FT_TS_NEED_EXTERN_C
#undef FT_TS_ERR_BASE

  /* FT_TS_ERR_PREFIX is needed internally */
#ifndef FT2_BUILD_LIBRARY
#undef FT_TS_ERR_PREFIX
#endif

  /* FT_TS_INCLUDE_ERR_PROTOS: Control whether function prototypes should be */
  /*                        included with                                 */
  /*                                                                      */
  /*                          #include <freetype/fterrors.h>              */
  /*                                                                      */
  /*                        This is only true where `FT_TS_ERRORDEF` is      */
  /*                        undefined.                                    */
  /*                                                                      */
  /* FT_TS_ERR_PROTOS_DEFINED: Actual multiple-inclusion protection of       */
  /*                        `fterrors.h`.                                 */
#ifdef FT_TS_INCLUDE_ERR_PROTOS
#undef FT_TS_INCLUDE_ERR_PROTOS

#ifndef FT_TS_ERR_PROTOS_DEFINED
#define FT_TS_ERR_PROTOS_DEFINED


FT_TS_BEGIN_HEADER

  /**************************************************************************
   *
   * @function:
   *   FT_TS_Error_String
   *
   * @description:
   *   Retrieve the description of a valid FreeType error code.
   *
   * @input:
   *   error_code ::
   *     A valid FreeType error code.
   *
   * @return:
   *   A C~string or `NULL`, if any error occurred.
   *
   * @note:
   *   FreeType has to be compiled with `FT_TS_CONFIG_OPTION_ERROR_STRINGS` or
   *   `FT_TS_DEBUG_LEVEL_ERROR` to get meaningful descriptions.
   *   'error_string' will be `NULL` otherwise.
   *
   *   Module identification will be ignored:
   *
   *   ```c
   *     strcmp( FT_TS_Error_String(  FT_TS_Err_Unknown_File_Format ),
   *             FT_TS_Error_String( BDF_Err_Unknown_File_Format ) ) == 0;
   *   ```
   */
  FT_TS_EXPORT( const char* )
  FT_TS_Error_String( FT_TS_Error  error_code );

  /* */

FT_TS_END_HEADER


#endif /* FT_TS_ERR_PROTOS_DEFINED */

#endif /* FT_TS_INCLUDE_ERR_PROTOS */

#endif /* !(FTERRORS_H_ && __FTERRORS_H__) */


/* END */
