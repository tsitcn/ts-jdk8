/****************************************************************************
 *
 * t42parse.h
 *
 *   Type 42 font parser (specification).
 *
 * Copyright (C) 2002-2022 by
 * Roberto Alameda.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef T42PARSE_H_
#define T42PARSE_H_


#include "t42objs.h"
#include <freetype/internal/psaux.h>


FT_TS_BEGIN_HEADER

  typedef struct  T42_ParserRec_
  {
    PS_ParserRec  root;
    FT_TS_Stream     stream;

    FT_TS_Byte*      base_dict;
    FT_TS_Long       base_len;

    FT_TS_Bool       in_memory;

  } T42_ParserRec, *T42_Parser;


  typedef struct  T42_Loader_
  {
    T42_ParserRec  parser;          /* parser used to read the stream */

    FT_TS_Int         num_chars;       /* number of characters in encoding */
    PS_TableRec    encoding_table;  /* PS_Table used to store the       */
                                    /* encoding character names         */

    FT_TS_Int         num_glyphs;
    PS_TableRec    glyph_names;
    PS_TableRec    charstrings;
    PS_TableRec    swap_table;      /* For moving .notdef glyph to index 0. */

  } T42_LoaderRec, *T42_Loader;


  FT_TS_LOCAL( FT_TS_Error )
  t42_parser_init( T42_Parser     parser,
                   FT_TS_Stream      stream,
                   FT_TS_Memory      memory,
                   PSAux_Service  psaux );

  FT_TS_LOCAL( void )
  t42_parser_done( T42_Parser  parser );


  FT_TS_LOCAL( FT_TS_Error )
  t42_parse_dict( T42_Face    face,
                  T42_Loader  loader,
                  FT_TS_Byte*    base,
                  FT_TS_Long     size );


  FT_TS_LOCAL( void )
  t42_loader_init( T42_Loader  loader,
                   T42_Face    face );

  FT_TS_LOCAL( void )
  t42_loader_done( T42_Loader  loader );


 /* */

FT_TS_END_HEADER


#endif /* T42PARSE_H_ */


/* END */
