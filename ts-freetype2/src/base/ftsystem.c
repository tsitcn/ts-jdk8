/****************************************************************************
 *
 * ftsystem.c
 *
 *   ANSI-specific FreeType low-level system interface (body).
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
   * This file contains the default interface used by FreeType to access
   * low-level, i.e. memory management, i/o access as well as thread
   * synchronisation.  It can be replaced by user-specific routines if
   * necessary.
   *
   */


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/internal/ftdebug.h>
#include <freetype/internal/ftstream.h>
#include <freetype/ftsystem.h>
#include <freetype/fterrors.h>
#include <freetype/fttypes.h>


  /**************************************************************************
   *
   *                      MEMORY MANAGEMENT INTERFACE
   *
   */

  /**************************************************************************
   *
   * It is not necessary to do any error checking for the
   * allocation-related functions.  This will be done by the higher level
   * routines like ft_mem_alloc() or ft_mem_realloc().
   *
   */


  /**************************************************************************
   *
   * @Function:
   *   ft_alloc
   *
   * @Description:
   *   The memory allocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   size ::
   *     The requested size in bytes.
   *
   * @Return:
   *   The address of newly allocated block.
   */
  FT_TS_CALLBACK_DEF( void* )
  ft_alloc( FT_TS_Memory  memory,
            long       size )
  {
    FT_TS_UNUSED( memory );

    return ft_smalloc( (size_t)size );
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_realloc
   *
   * @Description:
   *   The memory reallocation function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   cur_size ::
   *     The current size of the allocated memory block.
   *
   *   new_size ::
   *     The newly requested size in bytes.
   *
   *   block ::
   *     The current address of the block in memory.
   *
   * @Return:
   *   The address of the reallocated memory block.
   */
  FT_TS_CALLBACK_DEF( void* )
  ft_realloc( FT_TS_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_TS_UNUSED( memory );
    FT_TS_UNUSED( cur_size );

    return ft_srealloc( block, (size_t)new_size );
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_free
   *
   * @Description:
   *   The memory release function.
   *
   * @Input:
   *   memory ::
   *     A pointer to the memory object.
   *
   *   block ::
   *     The address of block in memory to be freed.
   */
  FT_TS_CALLBACK_DEF( void )
  ft_free( FT_TS_Memory  memory,
           void*      block )
  {
    FT_TS_UNUSED( memory );

    ft_sfree( block );
  }


  /**************************************************************************
   *
   *                    RESOURCE MANAGEMENT INTERFACE
   *
   */

#ifndef FT_TS_CONFIG_OPTION_DISABLE_STREAM_SUPPORT

  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  io

  /* We use the macro STREAM_FILE for convenience to extract the       */
  /* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE( stream )  ( (FT_TS_FILE*)stream->descriptor.pointer )


  /**************************************************************************
   *
   * @Function:
   *   ft_ansi_stream_close
   *
   * @Description:
   *   The function to close a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   */
  FT_TS_CALLBACK_DEF( void )
  ft_ansi_stream_close( FT_TS_Stream  stream )
  {
    ft_fclose( STREAM_FILE( stream ) );

    stream->descriptor.pointer = NULL;
    stream->size               = 0;
    stream->base               = NULL;
  }


  /**************************************************************************
   *
   * @Function:
   *   ft_ansi_stream_io
   *
   * @Description:
   *   The function to open a stream.
   *
   * @Input:
   *   stream ::
   *     A pointer to the stream object.
   *
   *   offset ::
   *     The position in the data stream to start reading.
   *
   *   buffer ::
   *     The address of buffer to store the read data.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @Return:
   *   The number of bytes actually read.  If `count' is zero (this is,
   *   the function is used for seeking), a non-zero return value
   *   indicates an error.
   */
  FT_TS_CALLBACK_DEF( unsigned long )
  ft_ansi_stream_io( FT_TS_Stream       stream,
                     unsigned long   offset,
                     unsigned char*  buffer,
                     unsigned long   count )
  {
    FT_TS_FILE*  file;


    if ( !count && offset > stream->size )
      return 1;

    file = STREAM_FILE( stream );

    if ( stream->pos != offset )
      ft_fseek( file, (long)offset, SEEK_SET );

    return (unsigned long)ft_fread( buffer, 1, count, file );
  }


  /* documentation is in ftstream.h */

  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Stream_Open( FT_TS_Stream    stream,
                  const char*  filepathname )
  {
    FT_TS_FILE*  file;


    if ( !stream )
      return FT_TS_THROW( Invalid_Stream_Handle );

    stream->descriptor.pointer = NULL;
    stream->pathname.pointer   = (char*)filepathname;
    stream->base               = NULL;
    stream->pos                = 0;
    stream->read               = NULL;
    stream->close              = NULL;

    file = ft_fopen( filepathname, "rb" );
    if ( !file )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:"
                 " could not open `%s'\n", filepathname ));

      return FT_TS_THROW( Cannot_Open_Resource );
    }

    ft_fseek( file, 0, SEEK_END );
    stream->size = (unsigned long)ft_ftell( file );
    if ( !stream->size )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:" ));
      FT_TS_ERROR(( " opened `%s' but zero-sized\n", filepathname ));
      ft_fclose( file );
      return FT_TS_THROW( Cannot_Open_Stream );
    }
    ft_fseek( file, 0, SEEK_SET );

    stream->descriptor.pointer = file;
    stream->read  = ft_ansi_stream_io;
    stream->close = ft_ansi_stream_close;

    FT_TS_TRACE1(( "FT_TS_Stream_Open:" ));
    FT_TS_TRACE1(( " opened `%s' (%ld bytes) successfully\n",
                filepathname, stream->size ));

    return FT_TS_Err_Ok;
  }

#endif /* !FT_TS_CONFIG_OPTION_DISABLE_STREAM_SUPPORT */

#ifdef FT_TS_DEBUG_MEMORY

  extern FT_TS_Int
  ft_mem_debug_init( FT_TS_Memory  memory );

  extern void
  ft_mem_debug_done( FT_TS_Memory  memory );

#endif


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( FT_TS_Memory )
  FT_TS_New_Memory( void )
  {
    FT_TS_Memory  memory;


    memory = (FT_TS_Memory)ft_smalloc( sizeof ( *memory ) );
    if ( memory )
    {
      memory->user    = NULL;
      memory->alloc   = ft_alloc;
      memory->realloc = ft_realloc;
      memory->free    = ft_free;
#ifdef FT_TS_DEBUG_MEMORY
      ft_mem_debug_init( memory );
#endif
    }

    return memory;
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( void )
  FT_TS_Done_Memory( FT_TS_Memory  memory )
  {
#ifdef FT_TS_DEBUG_MEMORY
    ft_mem_debug_done( memory );
#endif
    ft_sfree( memory );
  }


/* END */
