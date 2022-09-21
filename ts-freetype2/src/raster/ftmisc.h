/****************************************************************************
 *
 * ftmisc.h
 *
 *   Miscellaneous macros for stand-alone rasterizer (specification
 *   only).
 *
 * Copyright (C) 2005-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used
 * modified and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


  /****************************************************
   *
   * This file is *not* portable!  You have to adapt
   * its definitions to your platform.
   *
   */

#ifndef FTMISC_H_
#define FTMISC_H_


  /* memset */
#include FT_TS_CONFIG_STANDARD_LIBRARY_H

#define FT_TS_BEGIN_HEADER
#define FT_TS_END_HEADER

#define FT_TS_LOCAL_DEF( x )   static x


  /* from include/freetype/fttypes.h */

  typedef unsigned char  FT_TS_Byte;
  typedef signed int     FT_TS_Int;
  typedef unsigned int   FT_TS_UInt;
  typedef signed long    FT_TS_Long;
  typedef unsigned long  FT_TS_ULong;
  typedef signed long    FT_TS_F26Dot6;
  typedef int            FT_TS_Error;


#define FT_TS_STATIC_BYTE_CAST( type, var )  (type)(FT_TS_Byte)(var)


  /* from include/freetype/ftsystem.h */

  typedef struct FT_TS_MemoryRec_*  FT_TS_Memory;

  typedef void* (*FT_TS_Alloc_Func)( FT_TS_Memory  memory,
                                  long       size );

  typedef void (*FT_TS_Free_Func)( FT_TS_Memory  memory,
                                void*      block );

  typedef void* (*FT_TS_Realloc_Func)( FT_TS_Memory  memory,
                                    long       cur_size,
                                    long       new_size,
                                    void*      block );

  typedef struct FT_TS_MemoryRec_
  {
    void*            user;

    FT_TS_Alloc_Func    alloc;
    FT_TS_Free_Func     free;
    FT_TS_Realloc_Func  realloc;

  } FT_TS_MemoryRec;


  /* from src/ftcalc.c */

#if ( defined _WIN32 || defined _WIN64 )

  typedef __int64  FT_TS_Int64;

#else

#include "inttypes.h"

  typedef int64_t  FT_TS_Int64;

#endif


  static FT_TS_Long
  FT_TS_MulDiv( FT_TS_Long  a,
             FT_TS_Long  b,
             FT_TS_Long  c )
  {
    FT_TS_Int   s;
    FT_TS_Long  d;


    s = 1;
    if ( a < 0 ) { a = -a; s = -1; }
    if ( b < 0 ) { b = -b; s = -s; }
    if ( c < 0 ) { c = -c; s = -s; }

    d = (FT_TS_Long)( c > 0 ? ( (FT_TS_Int64)a * b + ( c >> 1 ) ) / c
                         : 0x7FFFFFFFL );

    return ( s > 0 ) ? d : -d;
  }


  static FT_TS_Long
  FT_TS_MulDiv_No_Round( FT_TS_Long  a,
                      FT_TS_Long  b,
                      FT_TS_Long  c )
  {
    FT_TS_Int   s;
    FT_TS_Long  d;


    s = 1;
    if ( a < 0 ) { a = -a; s = -1; }
    if ( b < 0 ) { b = -b; s = -s; }
    if ( c < 0 ) { c = -c; s = -s; }

    d = (FT_TS_Long)( c > 0 ? (FT_TS_Int64)a * b / c
                         : 0x7FFFFFFFL );

    return ( s > 0 ) ? d : -d;
  }

#endif /* FTMISC_H_ */


/* END */
