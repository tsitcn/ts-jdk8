/****************************************************************************
 *
 * fttypes.h
 *
 *   FreeType simple types definitions (specification only).
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


#ifndef FTTYPES_H_
#define FTTYPES_H_


#include <ft2build.h>
#include FT_TS_CONFIG_CONFIG_H
#include <freetype/ftsystem.h>
#include <freetype/ftimage.h>

#include <stddef.h>


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   basic_types
   *
   * @title:
   *   Basic Data Types
   *
   * @abstract:
   *   The basic data types defined by the library.
   *
   * @description:
   *   This section contains the basic data types defined by FreeType~2,
   *   ranging from simple scalar types to bitmap descriptors.  More
   *   font-specific structures are defined in a different section.
   *
   * @order:
   *   FT_TS_Byte
   *   FT_TS_Bytes
   *   FT_TS_Char
   *   FT_TS_Int
   *   FT_TS_UInt
   *   FT_TS_Int16
   *   FT_TS_UInt16
   *   FT_TS_Int32
   *   FT_TS_UInt32
   *   FT_TS_Int64
   *   FT_TS_UInt64
   *   FT_TS_Short
   *   FT_TS_UShort
   *   FT_TS_Long
   *   FT_TS_ULong
   *   FT_TS_Bool
   *   FT_TS_Offset
   *   FT_TS_PtrDist
   *   FT_TS_String
   *   FT_TS_Tag
   *   FT_TS_Error
   *   FT_TS_Fixed
   *   FT_TS_Pointer
   *   FT_TS_Pos
   *   FT_TS_Vector
   *   FT_TS_BBox
   *   FT_TS_Matrix
   *   FT_TS_FWord
   *   FT_TS_UFWord
   *   FT_TS_F2Dot14
   *   FT_TS_UnitVector
   *   FT_TS_F26Dot6
   *   FT_TS_Data
   *
   *   FT_TS_MAKE_TAG
   *
   *   FT_TS_Generic
   *   FT_TS_Generic_Finalizer
   *
   *   FT_TS_Bitmap
   *   FT_TS_Pixel_Mode
   *   FT_TS_Palette_Mode
   *   FT_TS_Glyph_Format
   *   FT_TS_IMAGE_TAG
   *
   */


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Bool
   *
   * @description:
   *   A typedef of unsigned char, used for simple booleans.  As usual,
   *   values 1 and~0 represent true and false, respectively.
   */
  typedef unsigned char  FT_TS_Bool;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_FWord
   *
   * @description:
   *   A signed 16-bit integer used to store a distance in original font
   *   units.
   */
  typedef signed short  FT_TS_FWord;   /* distance in FUnits */


  /**************************************************************************
   *
   * @type:
   *   FT_TS_UFWord
   *
   * @description:
   *   An unsigned 16-bit integer used to store a distance in original font
   *   units.
   */
  typedef unsigned short  FT_TS_UFWord;  /* unsigned distance */


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Char
   *
   * @description:
   *   A simple typedef for the _signed_ char type.
   */
  typedef signed char  FT_TS_Char;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Byte
   *
   * @description:
   *   A simple typedef for the _unsigned_ char type.
   */
  typedef unsigned char  FT_TS_Byte;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Bytes
   *
   * @description:
   *   A typedef for constant memory areas.
   */
  typedef const FT_TS_Byte*  FT_TS_Bytes;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Tag
   *
   * @description:
   *   A typedef for 32-bit tags (as used in the SFNT format).
   */
  typedef FT_TS_UInt32  FT_TS_Tag;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_String
   *
   * @description:
   *   A simple typedef for the char type, usually used for strings.
   */
  typedef char  FT_TS_String;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Short
   *
   * @description:
   *   A typedef for signed short.
   */
  typedef signed short  FT_TS_Short;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_UShort
   *
   * @description:
   *   A typedef for unsigned short.
   */
  typedef unsigned short  FT_TS_UShort;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Int
   *
   * @description:
   *   A typedef for the int type.
   */
  typedef signed int  FT_TS_Int;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_UInt
   *
   * @description:
   *   A typedef for the unsigned int type.
   */
  typedef unsigned int  FT_TS_UInt;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Long
   *
   * @description:
   *   A typedef for signed long.
   */
  typedef signed long  FT_TS_Long;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_ULong
   *
   * @description:
   *   A typedef for unsigned long.
   */
  typedef unsigned long  FT_TS_ULong;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_F2Dot14
   *
   * @description:
   *   A signed 2.14 fixed-point type used for unit vectors.
   */
  typedef signed short  FT_TS_F2Dot14;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_F26Dot6
   *
   * @description:
   *   A signed 26.6 fixed-point type used for vectorial pixel coordinates.
   */
  typedef signed long  FT_TS_F26Dot6;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Fixed
   *
   * @description:
   *   This type is used to store 16.16 fixed-point values, like scaling
   *   values or matrix coefficients.
   */
  typedef signed long  FT_TS_Fixed;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Error
   *
   * @description:
   *   The FreeType error code type.  A value of~0 is always interpreted as a
   *   successful operation.
   */
  typedef int  FT_TS_Error;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Pointer
   *
   * @description:
   *   A simple typedef for a typeless pointer.
   */
  typedef void*  FT_TS_Pointer;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Offset
   *
   * @description:
   *   This is equivalent to the ANSI~C `size_t` type, i.e., the largest
   *   _unsigned_ integer type used to express a file size or position, or a
   *   memory block size.
   */
  typedef size_t  FT_TS_Offset;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_PtrDist
   *
   * @description:
   *   This is equivalent to the ANSI~C `ptrdiff_t` type, i.e., the largest
   *   _signed_ integer type used to express the distance between two
   *   pointers.
   */
  typedef ft_ptrdiff_t  FT_TS_PtrDist;


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_UnitVector
   *
   * @description:
   *   A simple structure used to store a 2D vector unit vector.  Uses
   *   FT_TS_F2Dot14 types.
   *
   * @fields:
   *   x ::
   *     Horizontal coordinate.
   *
   *   y ::
   *     Vertical coordinate.
   */
  typedef struct  FT_TS_UnitVector_
  {
    FT_TS_F2Dot14  x;
    FT_TS_F2Dot14  y;

  } FT_TS_UnitVector;


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_Matrix
   *
   * @description:
   *   A simple structure used to store a 2x2 matrix.  Coefficients are in
   *   16.16 fixed-point format.  The computation performed is:
   *
   *   ```
   *     x' = x*xx + y*xy
   *     y' = x*yx + y*yy
   *   ```
   *
   * @fields:
   *   xx ::
   *     Matrix coefficient.
   *
   *   xy ::
   *     Matrix coefficient.
   *
   *   yx ::
   *     Matrix coefficient.
   *
   *   yy ::
   *     Matrix coefficient.
   */
  typedef struct  FT_TS_Matrix_
  {
    FT_TS_Fixed  xx, xy;
    FT_TS_Fixed  yx, yy;

  } FT_TS_Matrix;


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_Data
   *
   * @description:
   *   Read-only binary data represented as a pointer and a length.
   *
   * @fields:
   *   pointer ::
   *     The data.
   *
   *   length ::
   *     The length of the data in bytes.
   */
  typedef struct  FT_TS_Data_
  {
    const FT_TS_Byte*  pointer;
    FT_TS_UInt         length;

  } FT_TS_Data;


  /**************************************************************************
   *
   * @functype:
   *   FT_TS_Generic_Finalizer
   *
   * @description:
   *   Describe a function used to destroy the 'client' data of any FreeType
   *   object.  See the description of the @FT_TS_Generic type for details of
   *   usage.
   *
   * @input:
   *   The address of the FreeType object that is under finalization.  Its
   *   client data is accessed through its `generic` field.
   */
  typedef void  (*FT_TS_Generic_Finalizer)( void*  object );


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_Generic
   *
   * @description:
   *   Client applications often need to associate their own data to a
   *   variety of FreeType core objects.  For example, a text layout API
   *   might want to associate a glyph cache to a given size object.
   *
   *   Some FreeType object contains a `generic` field, of type `FT_TS_Generic`,
   *   which usage is left to client applications and font servers.
   *
   *   It can be used to store a pointer to client-specific data, as well as
   *   the address of a 'finalizer' function, which will be called by
   *   FreeType when the object is destroyed (for example, the previous
   *   client example would put the address of the glyph cache destructor in
   *   the `finalizer` field).
   *
   * @fields:
   *   data ::
   *     A typeless pointer to any client-specified data. This field is
   *     completely ignored by the FreeType library.
   *
   *   finalizer ::
   *     A pointer to a 'generic finalizer' function, which will be called
   *     when the object is destroyed.  If this field is set to `NULL`, no
   *     code will be called.
   */
  typedef struct  FT_TS_Generic_
  {
    void*                 data;
    FT_TS_Generic_Finalizer  finalizer;

  } FT_TS_Generic;


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_MAKE_TAG
   *
   * @description:
   *   This macro converts four-letter tags that are used to label TrueType
   *   tables into an `FT_TS_Tag` type, to be used within FreeType.
   *
   * @note:
   *   The produced values **must** be 32-bit integers.  Don't redefine this
   *   macro.
   */
#define FT_TS_MAKE_TAG( _x1, _x2, _x3, _x4 )                  \
          ( ( FT_TS_STATIC_BYTE_CAST( FT_TS_Tag, _x1 ) << 24 ) | \
            ( FT_TS_STATIC_BYTE_CAST( FT_TS_Tag, _x2 ) << 16 ) | \
            ( FT_TS_STATIC_BYTE_CAST( FT_TS_Tag, _x3 ) <<  8 ) | \
              FT_TS_STATIC_BYTE_CAST( FT_TS_Tag, _x4 )         )


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                    L I S T   M A N A G E M E N T                      */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/


  /**************************************************************************
   *
   * @section:
   *   list_processing
   *
   */


  /**************************************************************************
   *
   * @type:
   *   FT_TS_ListNode
   *
   * @description:
   *    Many elements and objects in FreeType are listed through an @FT_TS_List
   *    record (see @FT_TS_ListRec).  As its name suggests, an FT_TS_ListNode is a
   *    handle to a single list element.
   */
  typedef struct FT_TS_ListNodeRec_*  FT_TS_ListNode;


  /**************************************************************************
   *
   * @type:
   *   FT_TS_List
   *
   * @description:
   *   A handle to a list record (see @FT_TS_ListRec).
   */
  typedef struct FT_TS_ListRec_*  FT_TS_List;


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_ListNodeRec
   *
   * @description:
   *   A structure used to hold a single list element.
   *
   * @fields:
   *   prev ::
   *     The previous element in the list.  `NULL` if first.
   *
   *   next ::
   *     The next element in the list.  `NULL` if last.
   *
   *   data ::
   *     A typeless pointer to the listed object.
   */
  typedef struct  FT_TS_ListNodeRec_
  {
    FT_TS_ListNode  prev;
    FT_TS_ListNode  next;
    void*        data;

  } FT_TS_ListNodeRec;


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_ListRec
   *
   * @description:
   *   A structure used to hold a simple doubly-linked list.  These are used
   *   in many parts of FreeType.
   *
   * @fields:
   *   head ::
   *     The head (first element) of doubly-linked list.
   *
   *   tail ::
   *     The tail (last element) of doubly-linked list.
   */
  typedef struct  FT_TS_ListRec_
  {
    FT_TS_ListNode  head;
    FT_TS_ListNode  tail;

  } FT_TS_ListRec;

  /* */


#define FT_TS_IS_EMPTY( list )  ( (list).head == 0 )
#define FT_TS_BOOL( x )         FT_TS_STATIC_CAST( FT_TS_Bool, (x) != 0 )

  /* concatenate C tokens */
#define FT_TS_ERR_XCAT( x, y )  x ## y
#define FT_TS_ERR_CAT( x, y )   FT_TS_ERR_XCAT( x, y )

  /* see `ftmoderr.h` for descriptions of the following macros */

#define FT_TS_ERR( e )  FT_TS_ERR_CAT( FT_TS_ERR_PREFIX, e )

#define FT_TS_ERROR_BASE( x )    ( (x) & 0xFF )
#define FT_TS_ERROR_MODULE( x )  ( (x) & 0xFF00U )

#define FT_TS_ERR_EQ( x, e )                                        \
          ( FT_TS_ERROR_BASE( x ) == FT_TS_ERROR_BASE( FT_TS_ERR( e ) ) )
#define FT_TS_ERR_NEQ( x, e )                                       \
          ( FT_TS_ERROR_BASE( x ) != FT_TS_ERROR_BASE( FT_TS_ERR( e ) ) )


FT_TS_END_HEADER

#endif /* FTTYPES_H_ */


/* END */
