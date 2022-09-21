/****************************************************************************
 *
 * svsfnt.h
 *
 *   The FreeType SFNT table loading service (specification).
 *
 * Copyright (C) 2003-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef SVSFNT_H_
#define SVSFNT_H_

#include <freetype/internal/ftserv.h>
#include <freetype/tttables.h>


FT_TS_BEGIN_HEADER


  /*
   * SFNT table loading service.
   */

#define FT_TS_SERVICE_ID_SFNT_TABLE  "sfnt-table"


  /*
   * Used to implement FT_TS_Load_Sfnt_Table().
   */
  typedef FT_TS_Error
  (*FT_TS_SFNT_TableLoadFunc)( FT_TS_Face    face,
                            FT_TS_ULong   tag,
                            FT_TS_Long    offset,
                            FT_TS_Byte*   buffer,
                            FT_TS_ULong*  length );

  /*
   * Used to implement FT_TS_Get_Sfnt_Table().
   */
  typedef void*
  (*FT_TS_SFNT_TableGetFunc)( FT_TS_Face      face,
                           FT_TS_Sfnt_Tag  tag );


  /*
   * Used to implement FT_TS_Sfnt_Table_Info().
   */
  typedef FT_TS_Error
  (*FT_TS_SFNT_TableInfoFunc)( FT_TS_Face    face,
                            FT_TS_UInt    idx,
                            FT_TS_ULong  *tag,
                            FT_TS_ULong  *offset,
                            FT_TS_ULong  *length );


  FT_TS_DEFINE_SERVICE( SFNT_Table )
  {
    FT_TS_SFNT_TableLoadFunc  load_table;
    FT_TS_SFNT_TableGetFunc   get_table;
    FT_TS_SFNT_TableInfoFunc  table_info;
  };


#define FT_TS_DEFINE_SERVICE_SFNT_TABLEREC( class_, load_, get_, info_ )  \
  static const FT_TS_Service_SFNT_TableRec  class_ =                      \
  {                                                                    \
    load_, get_, info_                                                 \
  };

  /* */


FT_TS_END_HEADER


#endif /* SVSFNT_H_ */


/* END */
