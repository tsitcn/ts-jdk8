/****************************************************************************
 *
 * ftutil.c
 *
 *   FreeType utility file for memory and list management (body).
 *
 * Copyright (C) 2002-2022 by
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
#include <freetype/internal/ftmemory.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftlist.h>


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  memory


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****               M E M O R Y   M A N A G E M E N T               *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_alloc( FT_TS_Memory  memory,
                FT_TS_Long    size,
                FT_TS_Error  *p_error )
  {
    FT_TS_Error    error;
    FT_TS_Pointer  block = ft_mem_qalloc( memory, size, &error );

    if ( !error && block && size > 0 )
      FT_TS_MEM_ZERO( block, size );

    *p_error = error;
    return block;
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_qalloc( FT_TS_Memory  memory,
                 FT_TS_Long    size,
                 FT_TS_Error  *p_error )
  {
    FT_TS_Error    error = FT_TS_Err_Ok;
    FT_TS_Pointer  block = NULL;


    if ( size > 0 )
    {
      block = memory->alloc( memory, size );
      if ( !block )
        error = FT_TS_THROW( Out_Of_Memory );
    }
    else if ( size < 0 )
    {
      /* may help catch/prevent security issues */
      error = FT_TS_THROW( Invalid_Argument );
    }

    *p_error = error;
    return block;
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_realloc( FT_TS_Memory  memory,
                  FT_TS_Long    item_size,
                  FT_TS_Long    cur_count,
                  FT_TS_Long    new_count,
                  void*      block,
                  FT_TS_Error  *p_error )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    block = ft_mem_qrealloc( memory, item_size,
                             cur_count, new_count, block, &error );
    if ( !error && block && new_count > cur_count )
      FT_TS_MEM_ZERO( (char*)block + cur_count * item_size,
                   ( new_count - cur_count ) * item_size );

    *p_error = error;
    return block;
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_qrealloc( FT_TS_Memory  memory,
                   FT_TS_Long    item_size,
                   FT_TS_Long    cur_count,
                   FT_TS_Long    new_count,
                   void*      block,
                   FT_TS_Error  *p_error )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;


    /* Note that we now accept `item_size == 0' as a valid parameter, in
     * order to cover very weird cases where an ALLOC_MULT macro would be
     * called.
     */
    if ( cur_count < 0 || new_count < 0 || item_size < 0 )
    {
      /* may help catch/prevent nasty security issues */
      error = FT_TS_THROW( Invalid_Argument );
    }
    else if ( new_count == 0 || item_size == 0 )
    {
      ft_mem_free( memory, block );
      block = NULL;
    }
    else if ( new_count > FT_TS_INT_MAX / item_size )
    {
      error = FT_TS_THROW( Array_Too_Large );
    }
    else if ( cur_count == 0 )
    {
      FT_TS_ASSERT( !block );

      block = memory->alloc( memory, new_count * item_size );
      if ( block == NULL )
        error = FT_TS_THROW( Out_Of_Memory );
    }
    else
    {
      FT_TS_Pointer  block2;
      FT_TS_Long     cur_size = cur_count * item_size;
      FT_TS_Long     new_size = new_count * item_size;


      block2 = memory->realloc( memory, cur_size, new_size, block );
      if ( !block2 )
        error = FT_TS_THROW( Out_Of_Memory );
      else
        block = block2;
    }

    *p_error = error;
    return block;
  }


  FT_TS_BASE_DEF( void )
  ft_mem_free( FT_TS_Memory   memory,
               const void *P )
  {
    if ( P )
      memory->free( memory, (void*)P );
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_dup( FT_TS_Memory    memory,
              const void*  address,
              FT_TS_ULong     size,
              FT_TS_Error    *p_error )
  {
    FT_TS_Error    error;
    FT_TS_Pointer  p = ft_mem_qalloc( memory, (FT_TS_Long)size, &error );


    if ( !error && address && size > 0 )
      ft_memcpy( p, address, size );

    *p_error = error;
    return p;
  }


  FT_TS_BASE_DEF( FT_TS_Pointer )
  ft_mem_strdup( FT_TS_Memory    memory,
                 const char*  str,
                 FT_TS_Error    *p_error )
  {
    FT_TS_ULong  len = str ? (FT_TS_ULong)ft_strlen( str ) + 1
                        : 0;


    return ft_mem_dup( memory, str, len, p_error );
  }


  FT_TS_BASE_DEF( FT_TS_Int )
  ft_mem_strcpyn( char*        dst,
                  const char*  src,
                  FT_TS_ULong     size )
  {
    while ( size > 1 && *src != 0 )
    {
      *dst++ = *src++;
      size--;
    }

    *dst = 0;  /* always zero-terminate */

    return *src != 0;
  }


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                                                               *****/
  /*****            D O U B L Y   L I N K E D   L I S T S              *****/
  /*****                                                               *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  list

  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( FT_TS_ListNode )
  FT_TS_List_Find( FT_TS_List  list,
                void*    data )
  {
    FT_TS_ListNode  cur;


    if ( !list )
      return NULL;

    cur = list->head;
    while ( cur )
    {
      if ( cur->data == data )
        return cur;

      cur = cur->next;
    }

    return NULL;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_List_Add( FT_TS_List      list,
               FT_TS_ListNode  node )
  {
    FT_TS_ListNode  before;


    if ( !list || !node )
      return;

    before = list->tail;

    node->next = NULL;
    node->prev = before;

    if ( before )
      before->next = node;
    else
      list->head = node;

    list->tail = node;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_List_Insert( FT_TS_List      list,
                  FT_TS_ListNode  node )
  {
    FT_TS_ListNode  after;


    if ( !list || !node )
      return;

    after = list->head;

    node->next = after;
    node->prev = NULL;

    if ( !after )
      list->tail = node;
    else
      after->prev = node;

    list->head = node;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_List_Remove( FT_TS_List      list,
                  FT_TS_ListNode  node )
  {
    FT_TS_ListNode  before, after;


    if ( !list || !node )
      return;

    before = node->prev;
    after  = node->next;

    if ( before )
      before->next = after;
    else
      list->head = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_List_Up( FT_TS_List      list,
              FT_TS_ListNode  node )
  {
    FT_TS_ListNode  before, after;


    if ( !list || !node )
      return;

    before = node->prev;
    after  = node->next;

    /* check whether we are already on top of the list */
    if ( !before )
      return;

    before->next = after;

    if ( after )
      after->prev = before;
    else
      list->tail = before;

    node->prev       = NULL;
    node->next       = list->head;
    list->head->prev = node;
    list->head       = node;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( FT_TS_Error )
  FT_TS_List_Iterate( FT_TS_List           list,
                   FT_TS_List_Iterator  iterator,
                   void*             user )
  {
    FT_TS_ListNode  cur;
    FT_TS_Error     error = FT_TS_Err_Ok;


    if ( !list || !iterator )
      return FT_TS_THROW( Invalid_Argument );

    cur = list->head;

    while ( cur )
    {
      FT_TS_ListNode  next = cur->next;


      error = iterator( cur, user );
      if ( error )
        break;

      cur = next;
    }

    return error;
  }


  /* documentation is in ftlist.h */

  FT_TS_EXPORT_DEF( void )
  FT_TS_List_Finalize( FT_TS_List             list,
                    FT_TS_List_Destructor  destroy,
                    FT_TS_Memory           memory,
                    void*               user )
  {
    FT_TS_ListNode  cur;


    if ( !list || !memory )
      return;

    cur = list->head;
    while ( cur )
    {
      FT_TS_ListNode  next = cur->next;
      void*        data = cur->data;


      if ( destroy )
        destroy( memory, data, user );

      FT_TS_FREE( cur );
      cur = next;
    }

    list->head = NULL;
    list->tail = NULL;
  }


/* END */
