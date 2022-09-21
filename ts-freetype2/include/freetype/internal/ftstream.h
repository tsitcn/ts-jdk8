/****************************************************************************
 *
 * ftstream.h
 *
 *   Stream handling (specification).
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


#ifndef FTSTREAM_H_
#define FTSTREAM_H_


#include <ft2build.h>
#include <freetype/ftsystem.h>
#include <freetype/internal/ftobjs.h>


FT_TS_BEGIN_HEADER


  /* format of an 8-bit frame_op value:           */
  /*                                              */
  /* bit  76543210                                */
  /*      xxxxxxes                                */
  /*                                              */
  /* s is set to 1 if the value is signed.        */
  /* e is set to 1 if the value is little-endian. */
  /* xxx is a command.                            */

#define FT_TS_FRAME_OP_SHIFT         2
#define FT_TS_FRAME_OP_SIGNED        1
#define FT_TS_FRAME_OP_LITTLE        2
#define FT_TS_FRAME_OP_COMMAND( x )  ( x >> FT_TS_FRAME_OP_SHIFT )

#define FT_TS_MAKE_FRAME_OP( command, little, sign ) \
          ( ( command << FT_TS_FRAME_OP_SHIFT ) | ( little << 1 ) | sign )

#define FT_TS_FRAME_OP_END    0
#define FT_TS_FRAME_OP_START  1  /* start a new frame     */
#define FT_TS_FRAME_OP_BYTE   2  /* read 1-byte value     */
#define FT_TS_FRAME_OP_SHORT  3  /* read 2-byte value     */
#define FT_TS_FRAME_OP_LONG   4  /* read 4-byte value     */
#define FT_TS_FRAME_OP_OFF3   5  /* read 3-byte value     */
#define FT_TS_FRAME_OP_BYTES  6  /* read a bytes sequence */


  typedef enum  FT_TS_Frame_Op_
  {
    ft_frame_end       = 0,
    ft_frame_start     = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_START, 0, 0 ),

    ft_frame_byte      = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_BYTE,  0, 0 ),
    ft_frame_schar     = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_BYTE,  0, 1 ),

    ft_frame_ushort_be = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_SHORT, 0, 0 ),
    ft_frame_short_be  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_SHORT, 0, 1 ),
    ft_frame_ushort_le = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_SHORT, 1, 0 ),
    ft_frame_short_le  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_SHORT, 1, 1 ),

    ft_frame_ulong_be  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_LONG, 0, 0 ),
    ft_frame_long_be   = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_LONG, 0, 1 ),
    ft_frame_ulong_le  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_LONG, 1, 0 ),
    ft_frame_long_le   = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_LONG, 1, 1 ),

    ft_frame_uoff3_be  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_OFF3, 0, 0 ),
    ft_frame_off3_be   = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_OFF3, 0, 1 ),
    ft_frame_uoff3_le  = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_OFF3, 1, 0 ),
    ft_frame_off3_le   = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_OFF3, 1, 1 ),

    ft_frame_bytes     = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_BYTES, 0, 0 ),
    ft_frame_skip      = FT_TS_MAKE_FRAME_OP( FT_TS_FRAME_OP_BYTES, 0, 1 )

  } FT_TS_Frame_Op;


  typedef struct  FT_TS_Frame_Field_
  {
    FT_TS_Byte    value;
    FT_TS_Byte    size;
    FT_TS_UShort  offset;

  } FT_TS_Frame_Field;


  /* Construct an FT_TS_Frame_Field out of a structure type and a field name. */
  /* The structure type must be set in the FT_TS_STRUCTURE macro before       */
  /* calling the FT_TS_FRAME_START() macro.                                   */
  /*                                                                       */
#define FT_TS_FIELD_SIZE( f )                          \
          (FT_TS_Byte)sizeof ( ((FT_TS_STRUCTURE*)0)->f )

#define FT_TS_FIELD_SIZE_DELTA( f )                       \
          (FT_TS_Byte)sizeof ( ((FT_TS_STRUCTURE*)0)->f[0] )

#define FT_TS_FIELD_OFFSET( f )                         \
          (FT_TS_UShort)( offsetof( FT_TS_STRUCTURE, f ) )

#define FT_TS_FRAME_FIELD( frame_op, field ) \
          {                               \
            frame_op,                     \
            FT_TS_FIELD_SIZE( field ),       \
            FT_TS_FIELD_OFFSET( field )      \
          }

#define FT_TS_MAKE_EMPTY_FIELD( frame_op )  { frame_op, 0, 0 }

#define FT_TS_FRAME_START( size )   { ft_frame_start, 0, size }
#define FT_TS_FRAME_END             { ft_frame_end, 0, 0 }

#define FT_TS_FRAME_LONG( f )       FT_TS_FRAME_FIELD( ft_frame_long_be, f )
#define FT_TS_FRAME_ULONG( f )      FT_TS_FRAME_FIELD( ft_frame_ulong_be, f )
#define FT_TS_FRAME_SHORT( f )      FT_TS_FRAME_FIELD( ft_frame_short_be, f )
#define FT_TS_FRAME_USHORT( f )     FT_TS_FRAME_FIELD( ft_frame_ushort_be, f )
#define FT_TS_FRAME_OFF3( f )       FT_TS_FRAME_FIELD( ft_frame_off3_be, f )
#define FT_TS_FRAME_UOFF3( f )      FT_TS_FRAME_FIELD( ft_frame_uoff3_be, f )
#define FT_TS_FRAME_BYTE( f )       FT_TS_FRAME_FIELD( ft_frame_byte, f )
#define FT_TS_FRAME_CHAR( f )       FT_TS_FRAME_FIELD( ft_frame_schar, f )

#define FT_TS_FRAME_LONG_LE( f )    FT_TS_FRAME_FIELD( ft_frame_long_le, f )
#define FT_TS_FRAME_ULONG_LE( f )   FT_TS_FRAME_FIELD( ft_frame_ulong_le, f )
#define FT_TS_FRAME_SHORT_LE( f )   FT_TS_FRAME_FIELD( ft_frame_short_le, f )
#define FT_TS_FRAME_USHORT_LE( f )  FT_TS_FRAME_FIELD( ft_frame_ushort_le, f )
#define FT_TS_FRAME_OFF3_LE( f )    FT_TS_FRAME_FIELD( ft_frame_off3_le, f )
#define FT_TS_FRAME_UOFF3_LE( f )   FT_TS_FRAME_FIELD( ft_frame_uoff3_le, f )

#define FT_TS_FRAME_SKIP_LONG       { ft_frame_long_be, 0, 0 }
#define FT_TS_FRAME_SKIP_SHORT      { ft_frame_short_be, 0, 0 }
#define FT_TS_FRAME_SKIP_BYTE       { ft_frame_byte, 0, 0 }

#define FT_TS_FRAME_BYTES( field, count ) \
          {                            \
            ft_frame_bytes,            \
            count,                     \
            FT_TS_FIELD_OFFSET( field )   \
          }

#define FT_TS_FRAME_SKIP_BYTES( count )  { ft_frame_skip, count, 0 }


  /**************************************************************************
   *
   * Integer extraction macros -- the 'buffer' parameter must ALWAYS be of
   * type 'char*' or equivalent (1-byte elements).
   */

#define FT_TS_BYTE_( p, i )  ( ((const FT_TS_Byte*)(p))[(i)] )

#define FT_TS_INT16( x )   ( (FT_TS_Int16)(x)  )
#define FT_TS_UINT16( x )  ( (FT_TS_UInt16)(x) )
#define FT_TS_INT32( x )   ( (FT_TS_Int32)(x)  )
#define FT_TS_UINT32( x )  ( (FT_TS_UInt32)(x) )


#define FT_TS_BYTE_U16( p, i, s )  ( FT_TS_UINT16( FT_TS_BYTE_( p, i ) ) << (s) )
#define FT_TS_BYTE_U32( p, i, s )  ( FT_TS_UINT32( FT_TS_BYTE_( p, i ) ) << (s) )


  /*
   *    function      acts on      increases  does range   for    emits
   *                                pointer    checking   frames  error
   *  -------------------------------------------------------------------
   *   FT_TS_PEEK_XXX  buffer pointer      no         no        no     no
   *   FT_TS_NEXT_XXX  buffer pointer     yes         no        no     no
   *   FT_TS_GET_XXX   stream->cursor     yes        yes       yes     no
   *   FT_TS_READ_XXX  stream->pos        yes        yes        no    yes
   */


  /*
   * `FT_TS_PEEK_XXX' are generic macros to get data from a buffer position.  No
   * safety checks are performed.
   */
#define FT_TS_PEEK_SHORT( p )  FT_TS_INT16( FT_TS_BYTE_U16( p, 0, 8 ) | \
                                      FT_TS_BYTE_U16( p, 1, 0 ) )

#define FT_TS_PEEK_USHORT( p )  FT_TS_UINT16( FT_TS_BYTE_U16( p, 0, 8 ) | \
                                        FT_TS_BYTE_U16( p, 1, 0 ) )

#define FT_TS_PEEK_LONG( p )  FT_TS_INT32( FT_TS_BYTE_U32( p, 0, 24 ) | \
                                     FT_TS_BYTE_U32( p, 1, 16 ) | \
                                     FT_TS_BYTE_U32( p, 2,  8 ) | \
                                     FT_TS_BYTE_U32( p, 3,  0 ) )

#define FT_TS_PEEK_ULONG( p )  FT_TS_UINT32( FT_TS_BYTE_U32( p, 0, 24 ) | \
                                       FT_TS_BYTE_U32( p, 1, 16 ) | \
                                       FT_TS_BYTE_U32( p, 2,  8 ) | \
                                       FT_TS_BYTE_U32( p, 3,  0 ) )

#define FT_TS_PEEK_OFF3( p )  ( FT_TS_INT32( FT_TS_BYTE_U32( p, 0, 24 ) | \
                                       FT_TS_BYTE_U32( p, 1, 16 ) | \
                                       FT_TS_BYTE_U32( p, 2,  8 ) ) >> 8 )

#define FT_TS_PEEK_UOFF3( p )  FT_TS_UINT32( FT_TS_BYTE_U32( p, 0, 16 ) | \
                                       FT_TS_BYTE_U32( p, 1,  8 ) | \
                                       FT_TS_BYTE_U32( p, 2,  0 ) )

#define FT_TS_PEEK_SHORT_LE( p )  FT_TS_INT16( FT_TS_BYTE_U16( p, 1, 8 ) | \
                                         FT_TS_BYTE_U16( p, 0, 0 ) )

#define FT_TS_PEEK_USHORT_LE( p )  FT_TS_UINT16( FT_TS_BYTE_U16( p, 1, 8 ) |  \
                                           FT_TS_BYTE_U16( p, 0, 0 ) )

#define FT_TS_PEEK_LONG_LE( p )  FT_TS_INT32( FT_TS_BYTE_U32( p, 3, 24 ) | \
                                        FT_TS_BYTE_U32( p, 2, 16 ) | \
                                        FT_TS_BYTE_U32( p, 1,  8 ) | \
                                        FT_TS_BYTE_U32( p, 0,  0 ) )

#define FT_TS_PEEK_ULONG_LE( p )  FT_TS_UINT32( FT_TS_BYTE_U32( p, 3, 24 ) | \
                                          FT_TS_BYTE_U32( p, 2, 16 ) | \
                                          FT_TS_BYTE_U32( p, 1,  8 ) | \
                                          FT_TS_BYTE_U32( p, 0,  0 ) )

#define FT_TS_PEEK_OFF3_LE( p )  ( FT_TS_INT32( FT_TS_BYTE_U32( p, 2, 24 ) | \
                                          FT_TS_BYTE_U32( p, 1, 16 ) | \
                                          FT_TS_BYTE_U32( p, 0,  8 ) ) >> 8 )

#define FT_TS_PEEK_UOFF3_LE( p )  FT_TS_UINT32( FT_TS_BYTE_U32( p, 2, 16 ) | \
                                          FT_TS_BYTE_U32( p, 1,  8 ) | \
                                          FT_TS_BYTE_U32( p, 0,  0 ) )

  /*
   * `FT_TS_NEXT_XXX' are generic macros to get data from a buffer position
   * which is then increased appropriately.  No safety checks are performed.
   */
#define FT_TS_NEXT_CHAR( buffer )       \
          ( (signed char)*buffer++ )

#define FT_TS_NEXT_BYTE( buffer )         \
          ( (unsigned char)*buffer++ )

#define FT_TS_NEXT_SHORT( buffer )                                   \
          ( (short)( buffer += 2, FT_TS_PEEK_SHORT( buffer - 2 ) ) )

#define FT_TS_NEXT_USHORT( buffer )                                            \
          ( (unsigned short)( buffer += 2, FT_TS_PEEK_USHORT( buffer - 2 ) ) )

#define FT_TS_NEXT_OFF3( buffer )                                  \
          ( (long)( buffer += 3, FT_TS_PEEK_OFF3( buffer - 3 ) ) )

#define FT_TS_NEXT_UOFF3( buffer )                                           \
          ( (unsigned long)( buffer += 3, FT_TS_PEEK_UOFF3( buffer - 3 ) ) )

#define FT_TS_NEXT_LONG( buffer )                                  \
          ( (long)( buffer += 4, FT_TS_PEEK_LONG( buffer - 4 ) ) )

#define FT_TS_NEXT_ULONG( buffer )                                           \
          ( (unsigned long)( buffer += 4, FT_TS_PEEK_ULONG( buffer - 4 ) ) )


#define FT_TS_NEXT_SHORT_LE( buffer )                                   \
          ( (short)( buffer += 2, FT_TS_PEEK_SHORT_LE( buffer - 2 ) ) )

#define FT_TS_NEXT_USHORT_LE( buffer )                                            \
          ( (unsigned short)( buffer += 2, FT_TS_PEEK_USHORT_LE( buffer - 2 ) ) )

#define FT_TS_NEXT_OFF3_LE( buffer )                                  \
          ( (long)( buffer += 3, FT_TS_PEEK_OFF3_LE( buffer - 3 ) ) )

#define FT_TS_NEXT_UOFF3_LE( buffer )                                           \
          ( (unsigned long)( buffer += 3, FT_TS_PEEK_UOFF3_LE( buffer - 3 ) ) )

#define FT_TS_NEXT_LONG_LE( buffer )                                  \
          ( (long)( buffer += 4, FT_TS_PEEK_LONG_LE( buffer - 4 ) ) )

#define FT_TS_NEXT_ULONG_LE( buffer )                                           \
          ( (unsigned long)( buffer += 4, FT_TS_PEEK_ULONG_LE( buffer - 4 ) ) )


  /**************************************************************************
   *
   * The `FT_TS_GET_XXX` macros use an implicit 'stream' variable.
   *
   * Note that a call to `FT_TS_STREAM_SEEK` or `FT_TS_STREAM_POS` has **no**
   * effect on `FT_TS_GET_XXX`!  They operate on `stream->pos`, while
   * `FT_TS_GET_XXX` use `stream->cursor`.
   */
#if 0
#define FT_TS_GET_MACRO( type )    FT_TS_NEXT_ ## type ( stream->cursor )

#define FT_TS_GET_CHAR()       FT_TS_GET_MACRO( CHAR )
#define FT_TS_GET_BYTE()       FT_TS_GET_MACRO( BYTE )
#define FT_TS_GET_SHORT()      FT_TS_GET_MACRO( SHORT )
#define FT_TS_GET_USHORT()     FT_TS_GET_MACRO( USHORT )
#define FT_TS_GET_OFF3()       FT_TS_GET_MACRO( OFF3 )
#define FT_TS_GET_UOFF3()      FT_TS_GET_MACRO( UOFF3 )
#define FT_TS_GET_LONG()       FT_TS_GET_MACRO( LONG )
#define FT_TS_GET_ULONG()      FT_TS_GET_MACRO( ULONG )
#define FT_TS_GET_TAG4()       FT_TS_GET_MACRO( ULONG )

#define FT_TS_GET_SHORT_LE()   FT_TS_GET_MACRO( SHORT_LE )
#define FT_TS_GET_USHORT_LE()  FT_TS_GET_MACRO( USHORT_LE )
#define FT_TS_GET_LONG_LE()    FT_TS_GET_MACRO( LONG_LE )
#define FT_TS_GET_ULONG_LE()   FT_TS_GET_MACRO( ULONG_LE )

#else
#define FT_TS_GET_MACRO( func, type )        ( (type)func( stream ) )

#define FT_TS_GET_CHAR()       FT_TS_GET_MACRO( FT_TS_Stream_GetByte, FT_TS_Char )
#define FT_TS_GET_BYTE()       FT_TS_GET_MACRO( FT_TS_Stream_GetByte, FT_TS_Byte )
#define FT_TS_GET_SHORT()      FT_TS_GET_MACRO( FT_TS_Stream_GetUShort, FT_TS_Short )
#define FT_TS_GET_USHORT()     FT_TS_GET_MACRO( FT_TS_Stream_GetUShort, FT_TS_UShort )
#define FT_TS_GET_UOFF3()      FT_TS_GET_MACRO( FT_TS_Stream_GetUOffset, FT_TS_ULong )
#define FT_TS_GET_LONG()       FT_TS_GET_MACRO( FT_TS_Stream_GetULong, FT_TS_Long )
#define FT_TS_GET_ULONG()      FT_TS_GET_MACRO( FT_TS_Stream_GetULong, FT_TS_ULong )
#define FT_TS_GET_TAG4()       FT_TS_GET_MACRO( FT_TS_Stream_GetULong, FT_TS_ULong )

#define FT_TS_GET_SHORT_LE()   FT_TS_GET_MACRO( FT_TS_Stream_GetUShortLE, FT_TS_Short )
#define FT_TS_GET_USHORT_LE()  FT_TS_GET_MACRO( FT_TS_Stream_GetUShortLE, FT_TS_UShort )
#define FT_TS_GET_LONG_LE()    FT_TS_GET_MACRO( FT_TS_Stream_GetULongLE, FT_TS_Long )
#define FT_TS_GET_ULONG_LE()   FT_TS_GET_MACRO( FT_TS_Stream_GetULongLE, FT_TS_ULong )
#endif


#define FT_TS_READ_MACRO( func, type, var )        \
          ( var = (type)func( stream, &error ), \
            error != FT_TS_Err_Ok )

  /*
   * The `FT_TS_READ_XXX' macros use implicit `stream' and `error' variables.
   *
   * `FT_TS_READ_XXX' can be controlled with `FT_TS_STREAM_SEEK' and
   * `FT_TS_STREAM_POS'.  They use the full machinery to check whether a read is
   * valid.
   */
#define FT_TS_READ_BYTE( var )       FT_TS_READ_MACRO( FT_TS_Stream_ReadByte, FT_TS_Byte, var )
#define FT_TS_READ_CHAR( var )       FT_TS_READ_MACRO( FT_TS_Stream_ReadByte, FT_TS_Char, var )
#define FT_TS_READ_SHORT( var )      FT_TS_READ_MACRO( FT_TS_Stream_ReadUShort, FT_TS_Short, var )
#define FT_TS_READ_USHORT( var )     FT_TS_READ_MACRO( FT_TS_Stream_ReadUShort, FT_TS_UShort, var )
#define FT_TS_READ_UOFF3( var )      FT_TS_READ_MACRO( FT_TS_Stream_ReadUOffset, FT_TS_ULong, var )
#define FT_TS_READ_LONG( var )       FT_TS_READ_MACRO( FT_TS_Stream_ReadULong, FT_TS_Long, var )
#define FT_TS_READ_ULONG( var )      FT_TS_READ_MACRO( FT_TS_Stream_ReadULong, FT_TS_ULong, var )

#define FT_TS_READ_SHORT_LE( var )   FT_TS_READ_MACRO( FT_TS_Stream_ReadUShortLE, FT_TS_Short, var )
#define FT_TS_READ_USHORT_LE( var )  FT_TS_READ_MACRO( FT_TS_Stream_ReadUShortLE, FT_TS_UShort, var )
#define FT_TS_READ_LONG_LE( var )    FT_TS_READ_MACRO( FT_TS_Stream_ReadULongLE, FT_TS_Long, var )
#define FT_TS_READ_ULONG_LE( var )   FT_TS_READ_MACRO( FT_TS_Stream_ReadULongLE, FT_TS_ULong, var )


#ifndef FT_TS_CONFIG_OPTION_NO_DEFAULT_SYSTEM

  /* initialize a stream for reading a regular system stream */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_Open( FT_TS_Stream    stream,
                  const char*  filepathname );

#endif /* FT_TS_CONFIG_OPTION_NO_DEFAULT_SYSTEM */


  /* create a new (input) stream from an FT_TS_Open_Args structure */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_New( FT_TS_Library           library,
                 const FT_TS_Open_Args*  args,
                 FT_TS_Stream           *astream );

  /* free a stream */
  FT_TS_BASE( void )
  FT_TS_Stream_Free( FT_TS_Stream  stream,
                  FT_TS_Int     external );

  /* initialize a stream for reading in-memory data */
  FT_TS_BASE( void )
  FT_TS_Stream_OpenMemory( FT_TS_Stream       stream,
                        const FT_TS_Byte*  base,
                        FT_TS_ULong        size );

  /* close a stream (does not destroy the stream structure) */
  FT_TS_BASE( void )
  FT_TS_Stream_Close( FT_TS_Stream  stream );


  /* seek within a stream. position is relative to start of stream */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_Seek( FT_TS_Stream  stream,
                  FT_TS_ULong   pos );

  /* skip bytes in a stream */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_Skip( FT_TS_Stream  stream,
                  FT_TS_Long    distance );

  /* return current stream position */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_Pos( FT_TS_Stream  stream );

  /* read bytes from a stream into a user-allocated buffer, returns an */
  /* error if not all bytes could be read.                             */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_Read( FT_TS_Stream  stream,
                  FT_TS_Byte*   buffer,
                  FT_TS_ULong   count );

  /* read bytes from a stream at a given position */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_ReadAt( FT_TS_Stream  stream,
                    FT_TS_ULong   pos,
                    FT_TS_Byte*   buffer,
                    FT_TS_ULong   count );

  /* try to read bytes at the end of a stream; return number of bytes */
  /* really available                                                 */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_TryRead( FT_TS_Stream  stream,
                     FT_TS_Byte*   buffer,
                     FT_TS_ULong   count );

  /* Enter a frame of `count' consecutive bytes in a stream.  Returns an */
  /* error if the frame could not be read/accessed.  The caller can use  */
  /* the `FT_TS_Stream_GetXXX' functions to retrieve frame data without     */
  /* error checks.                                                       */
  /*                                                                     */
  /* You must _always_ call `FT_TS_Stream_ExitFrame' once you have entered  */
  /* a stream frame!                                                     */
  /*                                                                     */
  /* Nested frames are not permitted.                                    */
  /*                                                                     */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_EnterFrame( FT_TS_Stream  stream,
                        FT_TS_ULong   count );

  /* exit a stream frame */
  FT_TS_BASE( void )
  FT_TS_Stream_ExitFrame( FT_TS_Stream  stream );


  /* Extract a stream frame.  If the stream is disk-based, a heap block */
  /* is allocated and the frame bytes are read into it.  If the stream  */
  /* is memory-based, this function simply sets a pointer to the data.  */
  /*                                                                    */
  /* Useful to optimize access to memory-based streams transparently.   */
  /*                                                                    */
  /* `FT_TS_Stream_GetXXX' functions can't be used.                        */
  /*                                                                    */
  /* An extracted frame must be `freed' with a call to the function     */
  /* `FT_TS_Stream_ReleaseFrame'.                                          */
  /*                                                                    */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_ExtractFrame( FT_TS_Stream  stream,
                          FT_TS_ULong   count,
                          FT_TS_Byte**  pbytes );

  /* release an extract frame (see `FT_TS_Stream_ExtractFrame') */
  FT_TS_BASE( void )
  FT_TS_Stream_ReleaseFrame( FT_TS_Stream  stream,
                          FT_TS_Byte**  pbytes );


  /* read a byte from an entered frame */
  FT_TS_BASE( FT_TS_Byte )
  FT_TS_Stream_GetByte( FT_TS_Stream  stream );

  /* read a 16-bit big-endian unsigned integer from an entered frame */
  FT_TS_BASE( FT_TS_UShort )
  FT_TS_Stream_GetUShort( FT_TS_Stream  stream );

  /* read a 24-bit big-endian unsigned integer from an entered frame */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_GetUOffset( FT_TS_Stream  stream );

  /* read a 32-bit big-endian unsigned integer from an entered frame */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_GetULong( FT_TS_Stream  stream );

  /* read a 16-bit little-endian unsigned integer from an entered frame */
  FT_TS_BASE( FT_TS_UShort )
  FT_TS_Stream_GetUShortLE( FT_TS_Stream  stream );

  /* read a 32-bit little-endian unsigned integer from an entered frame */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_GetULongLE( FT_TS_Stream  stream );


  /* read a byte from a stream */
  FT_TS_BASE( FT_TS_Byte )
  FT_TS_Stream_ReadByte( FT_TS_Stream  stream,
                      FT_TS_Error*  error );

  /* read a 16-bit big-endian unsigned integer from a stream */
  FT_TS_BASE( FT_TS_UShort )
  FT_TS_Stream_ReadUShort( FT_TS_Stream  stream,
                        FT_TS_Error*  error );

  /* read a 24-bit big-endian unsigned integer from a stream */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_ReadUOffset( FT_TS_Stream  stream,
                         FT_TS_Error*  error );

  /* read a 32-bit big-endian integer from a stream */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_ReadULong( FT_TS_Stream  stream,
                       FT_TS_Error*  error );

  /* read a 16-bit little-endian unsigned integer from a stream */
  FT_TS_BASE( FT_TS_UShort )
  FT_TS_Stream_ReadUShortLE( FT_TS_Stream  stream,
                          FT_TS_Error*  error );

  /* read a 32-bit little-endian unsigned integer from a stream */
  FT_TS_BASE( FT_TS_ULong )
  FT_TS_Stream_ReadULongLE( FT_TS_Stream  stream,
                         FT_TS_Error*  error );

  /* Read a structure from a stream.  The structure must be described */
  /* by an array of FT_TS_Frame_Field records.                           */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Stream_ReadFields( FT_TS_Stream              stream,
                        const FT_TS_Frame_Field*  fields,
                        void*                  structure );


#define FT_TS_STREAM_POS()           \
          FT_TS_Stream_Pos( stream )

#define FT_TS_STREAM_SEEK( position )                               \
          FT_TS_SET_ERROR( FT_TS_Stream_Seek( stream,                  \
                                        (FT_TS_ULong)(position) ) )

#define FT_TS_STREAM_SKIP( distance )                              \
          FT_TS_SET_ERROR( FT_TS_Stream_Skip( stream,                 \
                                        (FT_TS_Long)(distance) ) )

#define FT_TS_STREAM_READ( buffer, count )                       \
          FT_TS_SET_ERROR( FT_TS_Stream_Read( stream,               \
                                        (FT_TS_Byte*)(buffer),   \
                                        (FT_TS_ULong)(count) ) )

#define FT_TS_STREAM_READ_AT( position, buffer, count )            \
          FT_TS_SET_ERROR( FT_TS_Stream_ReadAt( stream,               \
                                          (FT_TS_ULong)(position), \
                                          (FT_TS_Byte*)(buffer),   \
                                          (FT_TS_ULong)(count) ) )

#define FT_TS_STREAM_READ_FIELDS( fields, object )                          \
          FT_TS_SET_ERROR( FT_TS_Stream_ReadFields( stream, fields, object ) )


#define FT_TS_FRAME_ENTER( size )                                           \
          FT_TS_SET_ERROR(                                                  \
            FT_TS_DEBUG_INNER( FT_TS_Stream_EnterFrame( stream,                \
                                                  (FT_TS_ULong)(size) ) ) )

#define FT_TS_FRAME_EXIT()                                   \
          FT_TS_DEBUG_INNER( FT_TS_Stream_ExitFrame( stream ) )

#define FT_TS_FRAME_EXTRACT( size, bytes )                                       \
          FT_TS_SET_ERROR(                                                       \
            FT_TS_DEBUG_INNER( FT_TS_Stream_ExtractFrame( stream,                   \
                                                    (FT_TS_ULong)(size),         \
                                                    (FT_TS_Byte**)&(bytes) ) ) )

#define FT_TS_FRAME_RELEASE( bytes )                                         \
          FT_TS_DEBUG_INNER( FT_TS_Stream_ReleaseFrame( stream,                 \
                                                  (FT_TS_Byte**)&(bytes) ) )


FT_TS_END_HEADER

#endif /* FTSTREAM_H_ */


/* END */
