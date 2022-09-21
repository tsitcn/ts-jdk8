/****************************************************************************
 *
 * ftlist.h
 *
 *   Generic list support for FreeType (specification).
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
   * This file implements functions relative to list processing.  Its data
   * structures are defined in `freetype.h`.
   *
   */


#ifndef FTLIST_H_
#define FTLIST_H_


#include <freetype/freetype.h>

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   list_processing
   *
   * @title:
   *   List Processing
   *
   * @abstract:
   *   Simple management of lists.
   *
   * @description:
   *   This section contains various definitions related to list processing
   *   using doubly-linked nodes.
   *
   * @order:
   *   FT_TS_List
   *   FT_TS_ListNode
   *   FT_TS_ListRec
   *   FT_TS_ListNodeRec
   *
   *   FT_TS_List_Add
   *   FT_TS_List_Insert
   *   FT_TS_List_Find
   *   FT_TS_List_Remove
   *   FT_TS_List_Up
   *   FT_TS_List_Iterate
   *   FT_TS_List_Iterator
   *   FT_TS_List_Finalize
   *   FT_TS_List_Destructor
   *
   */


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Find
   *
   * @description:
   *   Find the list node for a given listed object.
   *
   * @input:
   *   list ::
   *     A pointer to the parent list.
   *   data ::
   *     The address of the listed object.
   *
   * @return:
   *   List node.  `NULL` if it wasn't found.
   */
  FT_TS_EXPORT( FT_TS_ListNode )
  FT_TS_List_Find( FT_TS_List  list,
                void*    data );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Add
   *
   * @description:
   *   Append an element to the end of a list.
   *
   * @inout:
   *   list ::
   *     A pointer to the parent list.
   *   node ::
   *     The node to append.
   */
  FT_TS_EXPORT( void )
  FT_TS_List_Add( FT_TS_List      list,
               FT_TS_ListNode  node );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Insert
   *
   * @description:
   *   Insert an element at the head of a list.
   *
   * @inout:
   *   list ::
   *     A pointer to parent list.
   *   node ::
   *     The node to insert.
   */
  FT_TS_EXPORT( void )
  FT_TS_List_Insert( FT_TS_List      list,
                  FT_TS_ListNode  node );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Remove
   *
   * @description:
   *   Remove a node from a list.  This function doesn't check whether the
   *   node is in the list!
   *
   * @input:
   *   node ::
   *     The node to remove.
   *
   * @inout:
   *   list ::
   *     A pointer to the parent list.
   */
  FT_TS_EXPORT( void )
  FT_TS_List_Remove( FT_TS_List      list,
                  FT_TS_ListNode  node );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Up
   *
   * @description:
   *   Move a node to the head/top of a list.  Used to maintain LRU lists.
   *
   * @inout:
   *   list ::
   *     A pointer to the parent list.
   *   node ::
   *     The node to move.
   */
  FT_TS_EXPORT( void )
  FT_TS_List_Up( FT_TS_List      list,
              FT_TS_ListNode  node );


  /**************************************************************************
   *
   * @functype:
   *   FT_TS_List_Iterator
   *
   * @description:
   *   An FT_TS_List iterator function that is called during a list parse by
   *   @FT_TS_List_Iterate.
   *
   * @input:
   *   node ::
   *     The current iteration list node.
   *
   *   user ::
   *     A typeless pointer passed to @FT_TS_List_Iterate.  Can be used to point
   *     to the iteration's state.
   */
  typedef FT_TS_Error
  (*FT_TS_List_Iterator)( FT_TS_ListNode  node,
                       void*        user );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Iterate
   *
   * @description:
   *   Parse a list and calls a given iterator function on each element.
   *   Note that parsing is stopped as soon as one of the iterator calls
   *   returns a non-zero value.
   *
   * @input:
   *   list ::
   *     A handle to the list.
   *   iterator ::
   *     An iterator function, called on each node of the list.
   *   user ::
   *     A user-supplied field that is passed as the second argument to the
   *     iterator.
   *
   * @return:
   *   The result (a FreeType error code) of the last iterator call.
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_List_Iterate( FT_TS_List           list,
                   FT_TS_List_Iterator  iterator,
                   void*             user );


  /**************************************************************************
   *
   * @functype:
   *   FT_TS_List_Destructor
   *
   * @description:
   *   An @FT_TS_List iterator function that is called during a list
   *   finalization by @FT_TS_List_Finalize to destroy all elements in a given
   *   list.
   *
   * @input:
   *   system ::
   *     The current system object.
   *
   *   data ::
   *     The current object to destroy.
   *
   *   user ::
   *     A typeless pointer passed to @FT_TS_List_Iterate.  It can be used to
   *     point to the iteration's state.
   */
  typedef void
  (*FT_TS_List_Destructor)( FT_TS_Memory  memory,
                         void*      data,
                         void*      user );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_List_Finalize
   *
   * @description:
   *   Destroy all elements in the list as well as the list itself.
   *
   * @input:
   *   list ::
   *     A handle to the list.
   *
   *   destroy ::
   *     A list destructor that will be applied to each element of the list.
   *     Set this to `NULL` if not needed.
   *
   *   memory ::
   *     The current memory object that handles deallocation.
   *
   *   user ::
   *     A user-supplied field that is passed as the last argument to the
   *     destructor.
   *
   * @note:
   *   This function expects that all nodes added by @FT_TS_List_Add or
   *   @FT_TS_List_Insert have been dynamically allocated.
   */
  FT_TS_EXPORT( void )
  FT_TS_List_Finalize( FT_TS_List             list,
                    FT_TS_List_Destructor  destroy,
                    FT_TS_Memory           memory,
                    void*               user );

  /* */


FT_TS_END_HEADER

#endif /* FTLIST_H_ */


/* END */
