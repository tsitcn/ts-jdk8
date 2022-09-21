/****************************************************************************
 *
 * ftdebug.h
 *
 *   Debugging and logging component (specification).
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
 *
 * IMPORTANT: A description of FreeType's debugging support can be
 *             found in 'docs/DEBUG.TXT'.  Read it if you need to use or
 *             understand this code.
 *
 */


#ifndef FTDEBUG_H_
#define FTDEBUG_H_


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/freetype.h>

#include "compiler-macros.h"

#ifdef FT_TS_DEBUG_LOGGING
#define DLG_STATIC
#include <dlg/output.h>
#include <dlg/dlg.h>

#include <freetype/ftlogging.h>
#endif /* FT_TS_DEBUG_LOGGING */


FT_TS_BEGIN_HEADER

  /* force the definition of FT_TS_DEBUG_LEVEL_TRACE if FT_TS_DEBUG_LOGGING is */
  /* already defined.                                                    */
  /*                                                                     */
#ifdef FT_TS_DEBUG_LOGGING
#undef  FT_TS_DEBUG_LEVEL_TRACE
#define FT_TS_DEBUG_LEVEL_TRACE
#endif

  /* force the definition of FT_TS_DEBUG_LEVEL_ERROR if FT_TS_DEBUG_LEVEL_TRACE */
  /* is already defined; this simplifies the following #ifdefs            */
  /*                                                                      */
#ifdef FT_TS_DEBUG_LEVEL_TRACE
#undef  FT_TS_DEBUG_LEVEL_ERROR
#define FT_TS_DEBUG_LEVEL_ERROR
#endif


  /**************************************************************************
   *
   * Define the trace enums as well as the trace levels array when they are
   * needed.
   *
   */

#ifdef FT_TS_DEBUG_LEVEL_TRACE

#define FT_TS_TRACE_DEF( x )  trace_ ## x ,

  /* defining the enumeration */
  typedef enum  FT_TS_Trace_
  {
#include <freetype/internal/fttrace.h>
    trace_count

  } FT_TS_Trace;


  /* a pointer to the array of trace levels, */
  /* provided by `src/base/ftdebug.c'        */
  extern int*  ft_trace_levels;

#undef FT_TS_TRACE_DEF

#endif /* FT_TS_DEBUG_LEVEL_TRACE */


  /**************************************************************************
   *
   * Define the FT_TS_TRACE macro
   *
   * IMPORTANT!
   *
   * Each component must define the macro FT_TS_COMPONENT to a valid FT_TS_Trace
   * value before using any TRACE macro.
   *
   * To get consistent logging output, there should be no newline character
   * (i.e., '\n') or a single trailing one in the message string of
   * `FT_TS_TRACEx` and `FT_TS_ERROR`.
   */


  /*************************************************************************
   *
   * If FT_TS_DEBUG_LOGGING is enabled, tracing messages are sent to dlg's API.
   * If FT_TS_DEBUG_LOGGING is disabled, tracing messages are sent to
   * `FT_TS_Message` (defined in ftdebug.c).
   */
#ifdef FT_TS_DEBUG_LOGGING

  /* we need two macros to convert the names of `FT_TS_COMPONENT` to a string */
#define FT_TS_LOGGING_TAG( x )   FT_TS_LOGGING_TAG_( x )
#define FT_TS_LOGGING_TAG_( x )  #x

  /* we need two macros to convert the component and the trace level */
  /* to a string that combines them                                  */
#define FT_TS_LOGGING_TAGX( x, y )   FT_TS_LOGGING_TAGX_( x, y )
#define FT_TS_LOGGING_TAGX_( x, y )  #x ":" #y


#define FT_TS_LOG( level, varformat )                                         \
          do                                                               \
          {                                                                \
            const char*  dlg_tag = FT_TS_LOGGING_TAGX( FT_TS_COMPONENT, level ); \
                                                                           \
                                                                           \
            ft_add_tag( dlg_tag );                                         \
            if ( ft_trace_levels[FT_TS_TRACE_COMP( FT_TS_COMPONENT )] >= level ) \
            {                                                              \
              if ( custom_output_handler != NULL )                         \
                FT_TS_Logging_Callback varformat;                             \
              else                                                         \
                dlg_trace varformat;                                       \
            }                                                              \
            ft_remove_tag( dlg_tag );                                      \
          } while( 0 )

#else /* !FT_TS_DEBUG_LOGGING */

#define FT_TS_LOG( level, varformat )                                         \
          do                                                               \
          {                                                                \
            if ( ft_trace_levels[FT_TS_TRACE_COMP( FT_TS_COMPONENT )] >= level ) \
              FT_TS_Message varformat;                                        \
          } while ( 0 )

#endif /* !FT_TS_DEBUG_LOGGING */


#ifdef FT_TS_DEBUG_LEVEL_TRACE

  /* we need two macros here to make cpp expand `FT_TS_COMPONENT' */
#define FT_TS_TRACE_COMP( x )   FT_TS_TRACE_COMP_( x )
#define FT_TS_TRACE_COMP_( x )  trace_ ## x

#define FT_TS_TRACE( level, varformat )  FT_TS_LOG( level, varformat )

#else /* !FT_TS_DEBUG_LEVEL_TRACE */

#define FT_TS_TRACE( level, varformat )  do { } while ( 0 )      /* nothing */

#endif /* !FT_TS_DEBUG_LEVEL_TRACE */


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Trace_Get_Count
   *
   * @description:
   *   Return the number of available trace components.
   *
   * @return:
   *   The number of trace components.  0 if FreeType 2 is not built with
   *   FT_TS_DEBUG_LEVEL_TRACE definition.
   *
   * @note:
   *   This function may be useful if you want to access elements of the
   *   internal trace levels array by an index.
   */
  FT_TS_BASE( FT_TS_Int )
  FT_TS_Trace_Get_Count( void );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Trace_Get_Name
   *
   * @description:
   *   Return the name of a trace component.
   *
   * @input:
   *   The index of the trace component.
   *
   * @return:
   *   The name of the trace component.  This is a statically allocated
   *   C~string, so do not free it after use.  `NULL` if FreeType is not
   *   built with FT_TS_DEBUG_LEVEL_TRACE definition.
   *
   * @note:
   *   Use @FT_TS_Trace_Get_Count to get the number of available trace
   *   components.
   */
  FT_TS_BASE( const char* )
  FT_TS_Trace_Get_Name( FT_TS_Int  idx );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Trace_Disable
   *
   * @description:
   *   Switch off tracing temporarily.  It can be activated again with
   *   @FT_TS_Trace_Enable.
   */
  FT_TS_BASE( void )
  FT_TS_Trace_Disable( void );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Trace_Enable
   *
   * @description:
   *   Activate tracing.  Use it after tracing has been switched off with
   *   @FT_TS_Trace_Disable.
   */
  FT_TS_BASE( void )
  FT_TS_Trace_Enable( void );


  /**************************************************************************
   *
   * You need two opening and closing parentheses!
   *
   * Example: FT_TS_TRACE0(( "Value is %i", foo ))
   *
   * Output of the FT_TS_TRACEX macros is sent to stderr.
   *
   */

#define FT_TS_TRACE0( varformat )  FT_TS_TRACE( 0, varformat )
#define FT_TS_TRACE1( varformat )  FT_TS_TRACE( 1, varformat )
#define FT_TS_TRACE2( varformat )  FT_TS_TRACE( 2, varformat )
#define FT_TS_TRACE3( varformat )  FT_TS_TRACE( 3, varformat )
#define FT_TS_TRACE4( varformat )  FT_TS_TRACE( 4, varformat )
#define FT_TS_TRACE5( varformat )  FT_TS_TRACE( 5, varformat )
#define FT_TS_TRACE6( varformat )  FT_TS_TRACE( 6, varformat )
#define FT_TS_TRACE7( varformat )  FT_TS_TRACE( 7, varformat )


  /**************************************************************************
   *
   * Define the FT_TS_ERROR macro.
   *
   * Output of this macro is sent to stderr.
   *
   */

#ifdef FT_TS_DEBUG_LEVEL_ERROR

  /**************************************************************************
   *
   * If FT_TS_DEBUG_LOGGING is enabled, error messages are sent to dlg's API.
   * If FT_TS_DEBUG_LOGGING is disabled, error messages are sent to `FT_TS_Message`
   * (defined in ftdebug.c).
   *
   */
#ifdef FT_TS_DEBUG_LOGGING

#define FT_TS_ERROR( varformat )                                      \
          do                                                       \
          {                                                        \
            const char*  dlg_tag = FT_TS_LOGGING_TAG( FT_TS_COMPONENT ); \
                                                                   \
                                                                   \
            ft_add_tag( dlg_tag );                                 \
            dlg_trace varformat;                                   \
            ft_remove_tag( dlg_tag );                              \
          } while ( 0 )

#else /* !FT_TS_DEBUG_LOGGING */

#define FT_TS_ERROR( varformat )  FT_TS_Message varformat

#endif /* !FT_TS_DEBUG_LOGGING */


#else  /* !FT_TS_DEBUG_LEVEL_ERROR */

#define FT_TS_ERROR( varformat )  do { } while ( 0 )      /* nothing */

#endif /* !FT_TS_DEBUG_LEVEL_ERROR */


  /**************************************************************************
   *
   * Define the FT_TS_ASSERT and FT_TS_THROW macros.  The call to `FT_TS_Throw` makes
   * it possible to easily set a breakpoint at this function.
   *
   */

#ifdef FT_TS_DEBUG_LEVEL_ERROR

#define FT_TS_ASSERT( condition )                                      \
          do                                                        \
          {                                                         \
            if ( !( condition ) )                                   \
              FT_TS_Panic( "assertion failed on line %d of file %s\n", \
                        __LINE__, __FILE__ );                       \
          } while ( 0 )

#define FT_TS_THROW( e )                                   \
          ( FT_TS_Throw( FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e ),   \
                      __LINE__,                         \
                      __FILE__ )                      | \
            FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e )            )

#else /* !FT_TS_DEBUG_LEVEL_ERROR */

#define FT_TS_ASSERT( condition )  do { } while ( 0 )

#define FT_TS_THROW( e )  FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e )

#endif /* !FT_TS_DEBUG_LEVEL_ERROR */


  /**************************************************************************
   *
   * Define `FT_TS_Message` and `FT_TS_Panic` when needed.
   *
   */

#ifdef FT_TS_DEBUG_LEVEL_ERROR

#include "stdio.h"  /* for vfprintf() */

  /* print a message */
  FT_TS_BASE( void )
  FT_TS_Message( const char*  fmt,
              ... );

  /* print a message and exit */
  FT_TS_BASE( void )
  FT_TS_Panic( const char*  fmt,
            ... );

  /* report file name and line number of an error */
  FT_TS_BASE( int )
  FT_TS_Throw( FT_TS_Error     error,
            int          line,
            const char*  file );

#endif /* FT_TS_DEBUG_LEVEL_ERROR */


  FT_TS_BASE( void )
  ft_debug_init( void );


#ifdef FT_TS_DEBUG_LOGGING

  /**************************************************************************
   *
   * 'dlg' uses output handlers to control how and where log messages are
   * printed.  Therefore we need to define a default output handler for
   * FreeType.
   */
  FT_TS_BASE( void )
  ft_log_handler( const struct dlg_origin*  origin,
                  const char*               string,
                  void*                     data );


  /**************************************************************************
   *
   * 1. `ft_default_log_handler` stores the function pointer that is used
   *    internally by FreeType to print logs to a file.
   *
   * 2. `custom_output_handler` stores the function pointer to the callback
   *    function provided by the user.
   *
   * It is defined in `ftdebug.c`.
   */
  extern dlg_handler            ft_default_log_handler;
  extern FT_TS_Custom_Log_Handler  custom_output_handler;


  /**************************************************************************
   *
   * If FT_TS_DEBUG_LOGGING macro is enabled, FreeType needs to initialize and
   * un-initialize `FILE*`.
   *
   * These functions are defined in `ftdebug.c`.
   */
  FT_TS_BASE( void )
  ft_logging_init( void );

  FT_TS_BASE( void )
  ft_logging_deinit( void );


  /**************************************************************************
   *
   * For printing the name of `FT_TS_COMPONENT` along with the actual log we
   * need to add a tag with the name of `FT_TS_COMPONENT`.
   *
   * These functions are defined in `ftdebug.c`.
   */
  FT_TS_BASE( void )
  ft_add_tag( const char*  tag );

  FT_TS_BASE( void )
  ft_remove_tag( const char*  tag );


  /**************************************************************************
   *
   * A function to print log data using a custom callback logging function
   * (which is set using `FT_TS_Set_Log_Handler`).
   *
   * This function is defined in `ftdebug.c`.
   */
  FT_TS_BASE( void )
  FT_TS_Logging_Callback( const char*  fmt,
                       ... );

#endif /* FT_TS_DEBUG_LOGGING */


FT_TS_END_HEADER

#endif /* FTDEBUG_H_ */


/* END */
