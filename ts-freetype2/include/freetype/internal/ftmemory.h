/****************************************************************************
 *
 * ftmemory.h
 *
 *   The FreeType memory management macros (specification).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTMEMORY_H_
#define FTMEMORY_H_


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/fttypes.h>

#include "compiler-macros.h"

FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_SET_ERROR
   *
   * @description:
   *   This macro is used to set an implicit 'error' variable to a given
   *   expression's value (usually a function call), and convert it to a
   *   boolean which is set whenever the value is != 0.
   */
#undef  FT_TS_SET_ERROR
#define FT_TS_SET_ERROR( expression ) \
          ( ( error = (expression) ) != 0 )



  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****                                                                 ****/
  /****                           M E M O R Y                           ****/
  /****                                                                 ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /* The calculation `NULL + n' is undefined in C.  Even if the resulting */
  /* pointer doesn't get dereferenced, this causes warnings with          */
  /* sanitizers.                                                          */
  /*                                                                      */
  /* We thus provide a macro that should be used if `base' can be NULL.   */
#define FT_TS_OFFSET( base, count )  ( (base) ? (base) + (count) : NULL )


  /*
   * C++ refuses to handle statements like p = (void*)anything, with `p' a
   * typed pointer.  Since we don't have a `typeof' operator in standard C++,
   * we have to use a template to emulate it.
   */

#ifdef __cplusplus

extern "C++"
{
  template <typename T> inline T*
  cplusplus_typeof(        T*,
                    void  *v )
  {
    return static_cast <T*> ( v );
  }
}

#define FT_TS_ASSIGNP( p, val )  (p) = cplusplus_typeof( (p), (val) )

#else

#define FT_TS_ASSIGNP( p, val )  (p) = (val)

#endif



#ifdef FT_TS_DEBUG_MEMORY

  FT_TS_BASE( const char* )  _ft_debug_file;
  FT_TS_BASE( long )         _ft_debug_lineno;

#define FT_TS_DEBUG_INNER( exp )  ( _ft_debug_file   = __FILE__, \
                                 _ft_debug_lineno = __LINE__, \
                                 (exp) )

#define FT_TS_ASSIGNP_INNER( p, exp )  ( _ft_debug_file   = __FILE__, \
                                      _ft_debug_lineno = __LINE__, \
                                      FT_TS_ASSIGNP( p, exp ) )

#else /* !FT_TS_DEBUG_MEMORY */

#define FT_TS_DEBUG_INNER( exp )       (exp)
#define FT_TS_ASSIGNP_INNER( p, exp )  FT_TS_ASSIGNP( p, exp )

#endif /* !FT_TS_DEBUG_MEMORY */


  /*
   * The allocation functions return a pointer, and the error code is written
   * to through the `p_error' parameter.
   */

  /* The `q' variants of the functions below (`q' for `quick') don't fill */
  /* the allocated or reallocated memory with zero bytes.                 */

  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_alloc( FT_TS_Memory  memory,
                FT_TS_Long    size,
                FT_TS_Error  *p_error );

  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_qalloc( FT_TS_Memory  memory,
                 FT_TS_Long    size,
                 FT_TS_Error  *p_error );

  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_realloc( FT_TS_Memory  memory,
                  FT_TS_Long    item_size,
                  FT_TS_Long    cur_count,
                  FT_TS_Long    new_count,
                  void*      block,
                  FT_TS_Error  *p_error );

  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_qrealloc( FT_TS_Memory  memory,
                   FT_TS_Long    item_size,
                   FT_TS_Long    cur_count,
                   FT_TS_Long    new_count,
                   void*      block,
                   FT_TS_Error  *p_error );

  FT_TS_BASE( void )
  ft_mem_free( FT_TS_Memory    memory,
               const void*  P );


  /* The `Q' variants of the macros below (`Q' for `quick') don't fill */
  /* the allocated or reallocated memory with zero bytes.              */

#define FT_TS_MEM_ALLOC( ptr, size )                               \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_alloc( memory,          \
                                               (FT_TS_Long)(size), \
                                               &error ) )

#define FT_TS_MEM_FREE( ptr )                                  \
          FT_TS_BEGIN_STMNT                                    \
            FT_TS_DEBUG_INNER( ft_mem_free( memory, (ptr) ) ); \
            (ptr) = NULL;                                   \
          FT_TS_END_STMNT

#define FT_TS_MEM_NEW( ptr )                        \
          FT_TS_MEM_ALLOC( ptr, sizeof ( *(ptr) ) )

#define FT_TS_MEM_REALLOC( ptr, cursz, newsz )                        \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_realloc( memory,           \
                                                 1,                \
                                                 (FT_TS_Long)(cursz), \
                                                 (FT_TS_Long)(newsz), \
                                                 (ptr),            \
                                                 &error ) )

#define FT_TS_MEM_QALLOC( ptr, size )                               \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qalloc( memory,          \
                                                (FT_TS_Long)(size), \
                                                &error ) )

#define FT_TS_MEM_QNEW( ptr )                        \
          FT_TS_MEM_QALLOC( ptr, sizeof ( *(ptr) ) )

#define FT_TS_MEM_QREALLOC( ptr, cursz, newsz )                        \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qrealloc( memory,           \
                                                  1,                \
                                                  (FT_TS_Long)(cursz), \
                                                  (FT_TS_Long)(newsz), \
                                                  (ptr),            \
                                                  &error ) )

#define FT_TS_MEM_ALLOC_MULT( ptr, count, item_size )                     \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_realloc( memory,               \
                                                 (FT_TS_Long)(item_size), \
                                                 0,                    \
                                                 (FT_TS_Long)(count),     \
                                                 NULL,                 \
                                                 &error ) )

#define FT_TS_MEM_REALLOC_MULT( ptr, oldcnt, newcnt, itmsz )           \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_realloc( memory,            \
                                                 (FT_TS_Long)(itmsz),  \
                                                 (FT_TS_Long)(oldcnt), \
                                                 (FT_TS_Long)(newcnt), \
                                                 (ptr),             \
                                                 &error ) )

#define FT_TS_MEM_QALLOC_MULT( ptr, count, item_size )                     \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qrealloc( memory,               \
                                                  (FT_TS_Long)(item_size), \
                                                  0,                    \
                                                  (FT_TS_Long)(count),     \
                                                  NULL,                 \
                                                  &error ) )

#define FT_TS_MEM_QREALLOC_MULT( ptr, oldcnt, newcnt, itmsz )           \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qrealloc( memory,            \
                                                  (FT_TS_Long)(itmsz),  \
                                                  (FT_TS_Long)(oldcnt), \
                                                  (FT_TS_Long)(newcnt), \
                                                  (ptr),             \
                                                  &error ) )


#define FT_TS_MEM_SET_ERROR( cond )  ( (cond), error != 0 )


#define FT_TS_MEM_SET( dest, byte, count )               \
          ft_memset( dest, byte, (FT_TS_Offset)(count) )

#define FT_TS_MEM_COPY( dest, source, count )              \
          ft_memcpy( dest, source, (FT_TS_Offset)(count) )

#define FT_TS_MEM_MOVE( dest, source, count )               \
          ft_memmove( dest, source, (FT_TS_Offset)(count) )


#define FT_TS_MEM_ZERO( dest, count )  FT_TS_MEM_SET( dest, 0, count )

#define FT_TS_ZERO( p )                FT_TS_MEM_ZERO( p, sizeof ( *(p) ) )


#define FT_TS_ARRAY_ZERO( dest, count )                             \
          FT_TS_MEM_ZERO( dest,                                     \
                       (FT_TS_Offset)(count) * sizeof ( *(dest) ) )

#define FT_TS_ARRAY_COPY( dest, source, count )                     \
          FT_TS_MEM_COPY( dest,                                     \
                       source,                                   \
                       (FT_TS_Offset)(count) * sizeof ( *(dest) ) )

#define FT_TS_ARRAY_MOVE( dest, source, count )                     \
          FT_TS_MEM_MOVE( dest,                                     \
                       source,                                   \
                       (FT_TS_Offset)(count) * sizeof ( *(dest) ) )


  /*
   * Return the maximum number of addressable elements in an array.  We limit
   * ourselves to INT_MAX, rather than UINT_MAX, to avoid any problems.
   */
#define FT_TS_ARRAY_MAX( ptr )           ( FT_TS_INT_MAX / sizeof ( *(ptr) ) )

#define FT_TS_ARRAY_CHECK( ptr, count )  ( (count) <= FT_TS_ARRAY_MAX( ptr ) )


  /**************************************************************************
   *
   * The following functions macros expect that their pointer argument is
   * _typed_ in order to automatically compute array element sizes.
   */

#define FT_TS_MEM_NEW_ARRAY( ptr, count )                              \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_realloc( memory,            \
                                                 sizeof ( *(ptr) ), \
                                                 0,                 \
                                                 (FT_TS_Long)(count),  \
                                                 NULL,              \
                                                 &error ) )

#define FT_TS_MEM_RENEW_ARRAY( ptr, cursz, newsz )                     \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_realloc( memory,            \
                                                 sizeof ( *(ptr) ), \
                                                 (FT_TS_Long)(cursz),  \
                                                 (FT_TS_Long)(newsz),  \
                                                 (ptr),             \
                                                 &error ) )

#define FT_TS_MEM_QNEW_ARRAY( ptr, count )                              \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qrealloc( memory,            \
                                                  sizeof ( *(ptr) ), \
                                                  0,                 \
                                                  (FT_TS_Long)(count),  \
                                                  NULL,              \
                                                  &error ) )

#define FT_TS_MEM_QRENEW_ARRAY( ptr, cursz, newsz )                     \
          FT_TS_ASSIGNP_INNER( ptr, ft_mem_qrealloc( memory,            \
                                                  sizeof ( *(ptr) ), \
                                                  (FT_TS_Long)(cursz),  \
                                                  (FT_TS_Long)(newsz),  \
                                                  (ptr),             \
                                                  &error ) )

#define FT_TS_ALLOC( ptr, size )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_ALLOC( ptr, size ) )

#define FT_TS_REALLOC( ptr, cursz, newsz )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_REALLOC( ptr, cursz, newsz ) )

#define FT_TS_ALLOC_MULT( ptr, count, item_size )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_ALLOC_MULT( ptr, count, item_size ) )

#define FT_TS_REALLOC_MULT( ptr, oldcnt, newcnt, itmsz )              \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_REALLOC_MULT( ptr, oldcnt,      \
                                                 newcnt, itmsz ) )

#define FT_TS_QALLOC( ptr, size )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QALLOC( ptr, size ) )

#define FT_TS_QREALLOC( ptr, cursz, newsz )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QREALLOC( ptr, cursz, newsz ) )

#define FT_TS_QALLOC_MULT( ptr, count, item_size )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QALLOC_MULT( ptr, count, item_size ) )

#define FT_TS_QREALLOC_MULT( ptr, oldcnt, newcnt, itmsz )              \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QREALLOC_MULT( ptr, oldcnt,      \
                                                  newcnt, itmsz ) )

#define FT_TS_FREE( ptr )  FT_TS_MEM_FREE( ptr )

#define FT_TS_NEW( ptr )  FT_TS_MEM_SET_ERROR( FT_TS_MEM_NEW( ptr ) )

#define FT_TS_NEW_ARRAY( ptr, count )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_NEW_ARRAY( ptr, count ) )

#define FT_TS_RENEW_ARRAY( ptr, curcnt, newcnt )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_RENEW_ARRAY( ptr, curcnt, newcnt ) )

#define FT_TS_QNEW( ptr )  FT_TS_MEM_SET_ERROR( FT_TS_MEM_QNEW( ptr ) )

#define FT_TS_QNEW_ARRAY( ptr, count )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QNEW_ARRAY( ptr, count ) )

#define FT_TS_QRENEW_ARRAY( ptr, curcnt, newcnt )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_QRENEW_ARRAY( ptr, curcnt, newcnt ) )


  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_strdup( FT_TS_Memory    memory,
                 const char*  str,
                 FT_TS_Error    *p_error );

  FT_TS_BASE( FT_TS_Pointer )
  ft_mem_dup( FT_TS_Memory    memory,
              const void*  address,
              FT_TS_ULong     size,
              FT_TS_Error    *p_error );


#define FT_TS_MEM_STRDUP( dst, str )                                            \
          (dst) = (char*)ft_mem_strdup( memory, (const char*)(str), &error )

#define FT_TS_STRDUP( dst, str )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_STRDUP( dst, str ) )

#define FT_TS_MEM_DUP( dst, address, size )                                    \
          (dst) = ft_mem_dup( memory, (address), (FT_TS_ULong)(size), &error )

#define FT_TS_DUP( dst, address, size )                           \
          FT_TS_MEM_SET_ERROR( FT_TS_MEM_DUP( dst, address, size ) )


  /* Return >= 1 if a truncation occurs.            */
  /* Return 0 if the source string fits the buffer. */
  /* This is *not* the same as strlcpy().           */
  FT_TS_BASE( FT_TS_Int )
  ft_mem_strcpyn( char*        dst,
                  const char*  src,
                  FT_TS_ULong     size );

#define FT_TS_STRCPYN( dst, src, size )                                         \
          ft_mem_strcpyn( (char*)dst, (const char*)(src), (FT_TS_ULong)(size) )


FT_TS_END_HEADER

#endif /* FTMEMORY_H_ */


/* END */
