/***************************************************************************/
/*                                                                         */
/*  ftsystem.c                                                             */
/*                                                                         */
/*    VMS-specific FreeType low-level system interface (body).             */
/*                                                                         */
/*  Copyright (C) 1996-2022 by                                             */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#include <ft2build.h>
  /* we use our special ftconfig.h file, not the standard one */
#include <ftconfig.h>
#include <freetype/internal/ftdebug.h>
#include <freetype/ftsystem.h>
#include <freetype/fterrors.h>
#include <freetype/fttypes.h>
#include <freetype/internal/ftobjs.h>

  /* memory-mapping includes and definitions */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE  0x00
#endif

#ifdef MUNMAP_USES_VOIDP
#define MUNMAP_ARG_CAST  void *
#else
#define MUNMAP_ARG_CAST  char *
#endif

#ifdef NEED_MUNMAP_DECL

#ifdef __cplusplus
  extern "C"
#else
  extern
#endif
  int
  munmap( char*  addr,
          int    len );

#define MUNMAP_ARG_CAST  char *

#endif /* NEED_DECLARATION_MUNMAP */


#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


  /*************************************************************************/
  /*                                                                       */
  /*                       MEMORY MANAGEMENT INTERFACE                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_alloc                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory allocation function.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory :: A pointer to the memory object.                          */
  /*                                                                       */
  /*    size   :: The requested size in bytes.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of newly allocated block.                              */
  /*                                                                       */
  FT_TS_CALLBACK_DEF( void* )
  ft_alloc( FT_TS_Memory  memory,
            long       size )
  {
    FT_TS_UNUSED( memory );

    return malloc( size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_realloc                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory reallocation function.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory   :: A pointer to the memory object.                        */
  /*                                                                       */
  /*    cur_size :: The current size of the allocated memory block.        */
  /*                                                                       */
  /*    new_size :: The newly requested size in bytes.                     */
  /*                                                                       */
  /*    block    :: The current address of the block in memory.            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The address of the reallocated memory block.                       */
  /*                                                                       */
  FT_TS_CALLBACK_DEF( void* )
  ft_realloc( FT_TS_Memory  memory,
              long       cur_size,
              long       new_size,
              void*      block )
  {
    FT_TS_UNUSED( memory );
    FT_TS_UNUSED( cur_size );

    return realloc( block, new_size );
  }


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_free                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The memory release function.                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory :: A pointer to the memory object.                          */
  /*                                                                       */
  /*    block  :: The address of block in memory to be freed.              */
  /*                                                                       */
  FT_TS_CALLBACK_DEF( void )
  ft_free( FT_TS_Memory  memory,
           void*      block )
  {
    FT_TS_UNUSED( memory );

    free( block );
  }


  /*************************************************************************/
  /*                                                                       */
  /*                     RESOURCE MANAGEMENT INTERFACE                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit      */
  /* parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log  */
  /* messages during execution.                                            */
  /*                                                                       */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  io

  /* We use the macro STREAM_FILE for convenience to extract the       */
  /* system-specific stream handle from a given FreeType stream object */
#define STREAM_FILE( stream )  ( (FILE*)stream->descriptor.pointer )


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    ft_close_stream                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The function to close a stream.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    stream :: A pointer to the stream object.                          */
  /*                                                                       */
  FT_TS_CALLBACK_DEF( void )
  ft_close_stream( FT_TS_Stream  stream )
  {
    munmap( (MUNMAP_ARG_CAST)stream->descriptor.pointer, stream->size );

    stream->descriptor.pointer = NULL;
    stream->size               = 0;
    stream->base               = NULL;
  }


  /* documentation is in ftobjs.h */

  FT_TS_BASE_DEF( FT_TS_Error )
  FT_TS_Stream_Open( FT_TS_Stream    stream,
                  const char*  filepathname )
  {
    int          file;
    struct stat  stat_buf;


    if ( !stream )
      return FT_TS_THROW( Invalid_Stream_Handle );

    /* open the file */
    file = open( filepathname, O_RDONLY );
    if ( file < 0 )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:" ));
      FT_TS_ERROR(( " could not open `%s'\n", filepathname ));
      return FT_TS_THROW( Cannot_Open_Resource );
    }

    if ( fstat( file, &stat_buf ) < 0 )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:" ));
      FT_TS_ERROR(( " could not `fstat' file `%s'\n", filepathname ));
      goto Fail_Map;
    }

    stream->size = stat_buf.st_size;
    if ( !stream->size )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:" ));
      FT_TS_ERROR(( " opened `%s' but zero-sized\n", filepathname ));
      goto Fail_Map;
    }

    stream->pos  = 0;
    stream->base = (unsigned char *)mmap( NULL,
                                          stream->size,
                                          PROT_READ,
                                          MAP_FILE | MAP_PRIVATE,
                                          file,
                                          0 );

    if ( stream->base == MAP_FAILED )
    {
      FT_TS_ERROR(( "FT_TS_Stream_Open:" ));
      FT_TS_ERROR(( " could not `mmap' file `%s'\n", filepathname ));
      goto Fail_Map;
    }

    close( file );

    stream->descriptor.pointer = stream->base;
    stream->pathname.pointer   = (char*)filepathname;

    stream->close = ft_close_stream;
    stream->read  = NULL;

    FT_TS_TRACE1(( "FT_TS_Stream_Open:" ));
    FT_TS_TRACE1(( " opened `%s' (%d bytes) successfully\n",
                filepathname, stream->size ));

    return FT_TS_Err_Ok;

  Fail_Map:
    close( file );

    stream->base = NULL;
    stream->size = 0;
    stream->pos  = 0;

    return FT_TS_THROW( Cannot_Open_Stream );
  }


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


    memory = (FT_TS_Memory)malloc( sizeof ( *memory ) );
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
    memory->free( memory, memory );
  }


/* END */
