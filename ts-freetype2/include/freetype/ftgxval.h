/****************************************************************************
 *
 * ftgxval.h
 *
 *   FreeType API for validating TrueTypeGX/AAT tables (specification).
 *
 * Copyright (C) 2004-2022 by
 * Masatake YAMATO, Redhat K.K,
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/****************************************************************************
 *
 * gxvalid is derived from both gxlayout module and otvalid module.
 * Development of gxlayout is supported by the Information-technology
 * Promotion Agency(IPA), Japan.
 *
 */


#ifndef FTGXVAL_H_
#define FTGXVAL_H_

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
   *   gx_validation
   *
   * @title:
   *   TrueTypeGX/AAT Validation
   *
   * @abstract:
   *   An API to validate TrueTypeGX/AAT tables.
   *
   * @description:
   *   This section contains the declaration of functions to validate some
   *   TrueTypeGX tables (feat, mort, morx, bsln, just, kern, opbd, trak,
   *   prop, lcar).
   *
   * @order:
   *   FT_TS_TrueTypeGX_Validate
   *   FT_TS_TrueTypeGX_Free
   *
   *   FT_TS_ClassicKern_Validate
   *   FT_TS_ClassicKern_Free
   *
   *   FT_TS_VALIDATE_GX_LENGTH
   *   FT_TS_VALIDATE_GXXXX
   *   FT_TS_VALIDATE_CKERNXXX
   *
   */

  /**************************************************************************
   *
   *
   * Warning: Use `FT_TS_VALIDATE_XXX` to validate a table.
   *          Following definitions are for gxvalid developers.
   *
   *
   */

#define FT_TS_VALIDATE_feat_INDEX     0
#define FT_TS_VALIDATE_mort_INDEX     1
#define FT_TS_VALIDATE_morx_INDEX     2
#define FT_TS_VALIDATE_bsln_INDEX     3
#define FT_TS_VALIDATE_just_INDEX     4
#define FT_TS_VALIDATE_kern_INDEX     5
#define FT_TS_VALIDATE_opbd_INDEX     6
#define FT_TS_VALIDATE_trak_INDEX     7
#define FT_TS_VALIDATE_prop_INDEX     8
#define FT_TS_VALIDATE_lcar_INDEX     9
#define FT_TS_VALIDATE_GX_LAST_INDEX  FT_TS_VALIDATE_lcar_INDEX


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_VALIDATE_GX_LENGTH
   *
   * @description:
   *   The number of tables checked in this module.  Use it as a parameter
   *   for the `table-length` argument of function @FT_TS_TrueTypeGX_Validate.
   */
#define FT_TS_VALIDATE_GX_LENGTH  ( FT_TS_VALIDATE_GX_LAST_INDEX + 1 )

  /* */

  /* Up to 0x1000 is used by otvalid.
     Ox2xxx is reserved for feature OT extension. */
#define FT_TS_VALIDATE_GX_START  0x4000
#define FT_TS_VALIDATE_GX_BITFIELD( tag ) \
          ( FT_TS_VALIDATE_GX_START << FT_TS_VALIDATE_##tag##_INDEX )


  /**************************************************************************
   *
   * @enum:
   *    FT_TS_VALIDATE_GXXXX
   *
   * @description:
   *    A list of bit-field constants used with @FT_TS_TrueTypeGX_Validate to
   *    indicate which TrueTypeGX/AAT Type tables should be validated.
   *
   * @values:
   *    FT_TS_VALIDATE_feat ::
   *      Validate 'feat' table.
   *
   *    FT_TS_VALIDATE_mort ::
   *      Validate 'mort' table.
   *
   *    FT_TS_VALIDATE_morx ::
   *      Validate 'morx' table.
   *
   *    FT_TS_VALIDATE_bsln ::
   *      Validate 'bsln' table.
   *
   *    FT_TS_VALIDATE_just ::
   *      Validate 'just' table.
   *
   *    FT_TS_VALIDATE_kern ::
   *      Validate 'kern' table.
   *
   *    FT_TS_VALIDATE_opbd ::
   *      Validate 'opbd' table.
   *
   *    FT_TS_VALIDATE_trak ::
   *      Validate 'trak' table.
   *
   *    FT_TS_VALIDATE_prop ::
   *      Validate 'prop' table.
   *
   *    FT_TS_VALIDATE_lcar ::
   *      Validate 'lcar' table.
   *
   *    FT_TS_VALIDATE_GX ::
   *      Validate all TrueTypeGX tables (feat, mort, morx, bsln, just, kern,
   *      opbd, trak, prop and lcar).
   *
   */

#define FT_TS_VALIDATE_feat  FT_TS_VALIDATE_GX_BITFIELD( feat )
#define FT_TS_VALIDATE_mort  FT_TS_VALIDATE_GX_BITFIELD( mort )
#define FT_TS_VALIDATE_morx  FT_TS_VALIDATE_GX_BITFIELD( morx )
#define FT_TS_VALIDATE_bsln  FT_TS_VALIDATE_GX_BITFIELD( bsln )
#define FT_TS_VALIDATE_just  FT_TS_VALIDATE_GX_BITFIELD( just )
#define FT_TS_VALIDATE_kern  FT_TS_VALIDATE_GX_BITFIELD( kern )
#define FT_TS_VALIDATE_opbd  FT_TS_VALIDATE_GX_BITFIELD( opbd )
#define FT_TS_VALIDATE_trak  FT_TS_VALIDATE_GX_BITFIELD( trak )
#define FT_TS_VALIDATE_prop  FT_TS_VALIDATE_GX_BITFIELD( prop )
#define FT_TS_VALIDATE_lcar  FT_TS_VALIDATE_GX_BITFIELD( lcar )

#define FT_TS_VALIDATE_GX  ( FT_TS_VALIDATE_feat | \
                          FT_TS_VALIDATE_mort | \
                          FT_TS_VALIDATE_morx | \
                          FT_TS_VALIDATE_bsln | \
                          FT_TS_VALIDATE_just | \
                          FT_TS_VALIDATE_kern | \
                          FT_TS_VALIDATE_opbd | \
                          FT_TS_VALIDATE_trak | \
                          FT_TS_VALIDATE_prop | \
                          FT_TS_VALIDATE_lcar )


  /**************************************************************************
   *
   * @function:
   *    FT_TS_TrueTypeGX_Validate
   *
   * @description:
   *    Validate various TrueTypeGX tables to assure that all offsets and
   *    indices are valid.  The idea is that a higher-level library that
   *    actually does the text layout can access those tables without error
   *    checking (which can be quite time consuming).
   *
   * @input:
   *    face ::
   *      A handle to the input face.
   *
   *    validation_flags ::
   *      A bit field that specifies the tables to be validated.  See
   *      @FT_TS_VALIDATE_GXXXX for possible values.
   *
   *    table_length ::
   *      The size of the `tables` array.  Normally, @FT_TS_VALIDATE_GX_LENGTH
   *      should be passed.
   *
   * @output:
   *    tables ::
   *      The array where all validated sfnt tables are stored.  The array
   *      itself must be allocated by a client.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   This function only works with TrueTypeGX fonts, returning an error
   *   otherwise.
   *
   *   After use, the application should deallocate the buffers pointed to by
   *   each `tables` element, by calling @FT_TS_TrueTypeGX_Free.  A `NULL` value
   *   indicates that the table either doesn't exist in the font, the
   *   application hasn't asked for validation, or the validator doesn't have
   *   the ability to validate the sfnt table.
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_TrueTypeGX_Validate( FT_TS_Face   face,
                          FT_TS_UInt   validation_flags,
                          FT_TS_Bytes  tables[FT_TS_VALIDATE_GX_LENGTH],
                          FT_TS_UInt   table_length );


  /**************************************************************************
   *
   * @function:
   *    FT_TS_TrueTypeGX_Free
   *
   * @description:
   *    Free the buffer allocated by TrueTypeGX validator.
   *
   * @input:
   *    face ::
   *      A handle to the input face.
   *
   *    table ::
   *      The pointer to the buffer allocated by @FT_TS_TrueTypeGX_Validate.
   *
   * @note:
   *   This function must be used to free the buffer allocated by
   *   @FT_TS_TrueTypeGX_Validate only.
   */
  FT_TS_EXPORT( void )
  FT_TS_TrueTypeGX_Free( FT_TS_Face   face,
                      FT_TS_Bytes  table );


  /**************************************************************************
   *
   * @enum:
   *    FT_TS_VALIDATE_CKERNXXX
   *
   * @description:
   *    A list of bit-field constants used with @FT_TS_ClassicKern_Validate to
   *    indicate the classic kern dialect or dialects.  If the selected type
   *    doesn't fit, @FT_TS_ClassicKern_Validate regards the table as invalid.
   *
   * @values:
   *    FT_TS_VALIDATE_MS ::
   *      Handle the 'kern' table as a classic Microsoft kern table.
   *
   *    FT_TS_VALIDATE_APPLE ::
   *      Handle the 'kern' table as a classic Apple kern table.
   *
   *    FT_TS_VALIDATE_CKERN ::
   *      Handle the 'kern' as either classic Apple or Microsoft kern table.
   */
#define FT_TS_VALIDATE_MS     ( FT_TS_VALIDATE_GX_START << 0 )
#define FT_TS_VALIDATE_APPLE  ( FT_TS_VALIDATE_GX_START << 1 )

#define FT_TS_VALIDATE_CKERN  ( FT_TS_VALIDATE_MS | FT_TS_VALIDATE_APPLE )


  /**************************************************************************
   *
   * @function:
   *    FT_TS_ClassicKern_Validate
   *
   * @description:
   *    Validate classic (16-bit format) kern table to assure that the
   *    offsets and indices are valid.  The idea is that a higher-level
   *    library that actually does the text layout can access those tables
   *    without error checking (which can be quite time consuming).
   *
   *    The 'kern' table validator in @FT_TS_TrueTypeGX_Validate deals with both
   *    the new 32-bit format and the classic 16-bit format, while
   *    FT_TS_ClassicKern_Validate only supports the classic 16-bit format.
   *
   * @input:
   *    face ::
   *      A handle to the input face.
   *
   *    validation_flags ::
   *      A bit field that specifies the dialect to be validated.  See
   *      @FT_TS_VALIDATE_CKERNXXX for possible values.
   *
   * @output:
   *    ckern_table ::
   *      A pointer to the kern table.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   After use, the application should deallocate the buffers pointed to by
   *   `ckern_table`, by calling @FT_TS_ClassicKern_Free.  A `NULL` value
   *   indicates that the table doesn't exist in the font.
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_ClassicKern_Validate( FT_TS_Face    face,
                           FT_TS_UInt    validation_flags,
                           FT_TS_Bytes  *ckern_table );


  /**************************************************************************
   *
   * @function:
   *    FT_TS_ClassicKern_Free
   *
   * @description:
   *    Free the buffer allocated by classic Kern validator.
   *
   * @input:
   *    face ::
   *      A handle to the input face.
   *
   *    table ::
   *      The pointer to the buffer that is allocated by
   *      @FT_TS_ClassicKern_Validate.
   *
   * @note:
   *   This function must be used to free the buffer allocated by
   *   @FT_TS_ClassicKern_Validate only.
   */
  FT_TS_EXPORT( void )
  FT_TS_ClassicKern_Free( FT_TS_Face   face,
                       FT_TS_Bytes  table );

  /* */


FT_TS_END_HEADER

#endif /* FTGXVAL_H_ */


/* END */
