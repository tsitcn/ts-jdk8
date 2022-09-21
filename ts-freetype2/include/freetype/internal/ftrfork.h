/****************************************************************************
 *
 * ftrfork.h
 *
 *   Embedded resource forks accessor (specification).
 *
 * Copyright (C) 2004-2022 by
 * Masatake YAMATO and Redhat K.K.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/****************************************************************************
 * Development of the code in this file is support of
 * Information-technology Promotion Agency, Japan.
 */


#ifndef FTRFORK_H_
#define FTRFORK_H_


#include <freetype/internal/ftobjs.h>


FT_TS_BEGIN_HEADER


  /* Number of guessing rules supported in `FT_TS_Raccess_Guess'.            */
  /* Don't forget to increment the number if you add a new guessing rule. */
#define FT_TS_RACCESS_N_RULES  9


  /* A structure to describe a reference in a resource by its resource ID */
  /* and internal offset.  The `POST' resource expects to be concatenated */
  /* by the order of resource IDs instead of its appearance in the file.  */

  typedef struct  FT_TS_RFork_Ref_
  {
    FT_TS_Short  res_id;
    FT_TS_Long   offset;

  } FT_TS_RFork_Ref;


#ifdef FT_TS_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK
  typedef FT_TS_Error
  (*ft_raccess_guess_func)( FT_TS_Library  library,
                            FT_TS_Stream   stream,
                            char       *base_file_name,
                            char      **result_file_name,
                            FT_TS_Long    *result_offset );

  typedef enum  FT_TS_RFork_Rule_ {
    FT_TS_RFork_Rule_invalid = -2,
    FT_TS_RFork_Rule_uknown, /* -1 */
    FT_TS_RFork_Rule_apple_double,
    FT_TS_RFork_Rule_apple_single,
    FT_TS_RFork_Rule_darwin_ufs_export,
    FT_TS_RFork_Rule_darwin_newvfs,
    FT_TS_RFork_Rule_darwin_hfsplus,
    FT_TS_RFork_Rule_vfat,
    FT_TS_RFork_Rule_linux_cap,
    FT_TS_RFork_Rule_linux_double,
    FT_TS_RFork_Rule_linux_netatalk
  } FT_TS_RFork_Rule;

  /* For fast translation between rule index and rule type,
   * the macros FT_TS_RFORK_xxx should be kept consistent with the
   * raccess_guess_funcs table
   */
  typedef struct ft_raccess_guess_rec_ {
    ft_raccess_guess_func  func;
    FT_TS_RFork_Rule          type;
  } ft_raccess_guess_rec;


#define CONST_FT_TS_RFORK_RULE_ARRAY_BEGIN( name, type )  \
          static const type name[] = {
#define CONST_FT_TS_RFORK_RULE_ARRAY_ENTRY( func_suffix, type_suffix )  \
          { raccess_guess_ ## func_suffix,                           \
            FT_TS_RFork_Rule_ ## type_suffix },
  /* this array is a storage, thus a final `;' is needed */
#define CONST_FT_TS_RFORK_RULE_ARRAY_END  };

#endif /* FT_TS_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK */


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Raccess_Guess
   *
   * @description:
   *   Guess a file name and offset where the actual resource fork is stored.
   *   The macro FT_TS_RACCESS_N_RULES holds the number of guessing rules; the
   *   guessed result for the Nth rule is represented as a triplet: a new
   *   file name (new_names[N]), a file offset (offsets[N]), and an error
   *   code (errors[N]).
   *
   * @input:
   *   library ::
   *     A FreeType library instance.
   *
   *   stream ::
   *     A file stream containing the resource fork.
   *
   *   base_name ::
   *     The (base) file name of the resource fork used for some guessing
   *     rules.
   *
   * @output:
   *   new_names ::
   *     An array of guessed file names in which the resource forks may
   *     exist.  If 'new_names[N]' is `NULL`, the guessed file name is equal
   *     to `base_name`.
   *
   *   offsets ::
   *     An array of guessed file offsets.  'offsets[N]' holds the file
   *     offset of the possible start of the resource fork in file
   *     'new_names[N]'.
   *
   *   errors ::
   *     An array of FreeType error codes.  'errors[N]' is the error code of
   *     Nth guessing rule function.  If 'errors[N]' is not FT_TS_Err_Ok,
   *     'new_names[N]' and 'offsets[N]' are meaningless.
   */
  FT_TS_BASE( void )
  FT_TS_Raccess_Guess( FT_TS_Library  library,
                    FT_TS_Stream   stream,
                    char*       base_name,
                    char**      new_names,
                    FT_TS_Long*    offsets,
                    FT_TS_Error*   errors );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Raccess_Get_HeaderInfo
   *
   * @description:
   *   Get the information from the header of resource fork.  The information
   *   includes the file offset where the resource map starts, and the file
   *   offset where the resource data starts.  `FT_TS_Raccess_Get_DataOffsets`
   *   requires these two data.
   *
   * @input:
   *   library ::
   *     A FreeType library instance.
   *
   *   stream ::
   *     A file stream containing the resource fork.
   *
   *   rfork_offset ::
   *     The file offset where the resource fork starts.
   *
   * @output:
   *   map_offset ::
   *     The file offset where the resource map starts.
   *
   *   rdata_pos ::
   *     The file offset where the resource data starts.
   *
   * @return:
   *   FreeType error code.  FT_TS_Err_Ok means success.
   */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Raccess_Get_HeaderInfo( FT_TS_Library  library,
                             FT_TS_Stream   stream,
                             FT_TS_Long     rfork_offset,
                             FT_TS_Long    *map_offset,
                             FT_TS_Long    *rdata_pos );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Raccess_Get_DataOffsets
   *
   * @description:
   *   Get the data offsets for a tag in a resource fork.  Offsets are stored
   *   in an array because, in some cases, resources in a resource fork have
   *   the same tag.
   *
   * @input:
   *   library ::
   *     A FreeType library instance.
   *
   *   stream ::
   *     A file stream containing the resource fork.
   *
   *   map_offset ::
   *     The file offset where the resource map starts.
   *
   *   rdata_pos ::
   *     The file offset where the resource data starts.
   *
   *   tag ::
   *     The resource tag.
   *
   *   sort_by_res_id ::
   *     A Boolean to sort the fragmented resource by their ids.  The
   *     fragmented resources for 'POST' resource should be sorted to restore
   *     Type1 font properly.  For 'sfnt' resources, sorting may induce a
   *     different order of the faces in comparison to that by QuickDraw API.
   *
   * @output:
   *   offsets ::
   *     The stream offsets for the resource data specified by 'tag'.  This
   *     array is allocated by the function, so you have to call @ft_mem_free
   *     after use.
   *
   *   count ::
   *     The length of offsets array.
   *
   * @return:
   *   FreeType error code.  FT_TS_Err_Ok means success.
   *
   * @note:
   *   Normally you should use `FT_TS_Raccess_Get_HeaderInfo` to get the value
   *   for `map_offset` and `rdata_pos`.
   */
  FT_TS_BASE( FT_TS_Error )
  FT_TS_Raccess_Get_DataOffsets( FT_TS_Library  library,
                              FT_TS_Stream   stream,
                              FT_TS_Long     map_offset,
                              FT_TS_Long     rdata_pos,
                              FT_TS_Long     tag,
                              FT_TS_Bool     sort_by_res_id,
                              FT_TS_Long   **offsets,
                              FT_TS_Long    *count );


FT_TS_END_HEADER

#endif /* FTRFORK_H_ */


/* END */
