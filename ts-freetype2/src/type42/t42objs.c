/****************************************************************************
 *
 * t42objs.c
 *
 *   Type 42 objects manager (body).
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


#include "t42objs.h"
#include "t42parse.h"
#include "t42error.h"
#include <freetype/internal/ftdebug.h>
#include <freetype/ftlist.h>
#include <freetype/ttnameid.h>


#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  t42


  static FT_TS_Error
  T42_Open_Face( T42_Face  face )
  {
    T42_LoaderRec  loader;
    T42_Parser     parser;
    T1_Font        type1 = &face->type1;
    FT_TS_Memory      memory = face->root.memory;
    FT_TS_Error       error;

    PSAux_Service  psaux  = (PSAux_Service)face->psaux;


    t42_loader_init( &loader, face );

    parser = &loader.parser;

    face->ttf_data = NULL;
    face->ttf_size = 0;

    error = t42_parser_init( parser,
                             face->root.stream,
                             memory,
                             psaux);
    if ( error )
      goto Exit;

    error = t42_parse_dict( face, &loader,
                            parser->base_dict, parser->base_len );
    if ( error )
      goto Exit;

    if ( type1->font_type != 42 )
    {
      FT_TS_ERROR(( "T42_Open_Face: cannot handle FontType %d\n",
                 type1->font_type ));
      error = FT_TS_THROW( Unknown_File_Format );
      goto Exit;
    }

    /* now, propagate the charstrings and glyphnames tables */
    /* to the Type1 data                                    */
    type1->num_glyphs = loader.num_glyphs;

    if ( !loader.charstrings.init )
    {
      FT_TS_ERROR(( "T42_Open_Face: no charstrings array in face\n" ));
      error = FT_TS_THROW( Invalid_File_Format );
    }

    loader.charstrings.init  = 0;
    type1->charstrings_block = loader.charstrings.block;
    type1->charstrings       = loader.charstrings.elements;
    type1->charstrings_len   = loader.charstrings.lengths;

    /* we copy the glyph names `block' and `elements' fields; */
    /* the `lengths' field must be released later             */
    type1->glyph_names_block    = loader.glyph_names.block;
    type1->glyph_names          = (FT_TS_String**)loader.glyph_names.elements;
    loader.glyph_names.block    = NULL;
    loader.glyph_names.elements = NULL;

    /* we must now build type1.encoding when we have a custom array */
    if ( type1->encoding_type == T1_ENCODING_TYPE_ARRAY )
    {
      FT_TS_Int  charcode, idx, min_char, max_char;


      /* OK, we do the following: for each element in the encoding   */
      /* table, look up the index of the glyph having the same name  */
      /* as defined in the CharStrings array.                        */
      /* The index is then stored in type1.encoding.char_index, and  */
      /* the name in type1.encoding.char_name                        */

      min_char = 0;
      max_char = 0;

      charcode = 0;
      for ( ; charcode < loader.encoding_table.max_elems; charcode++ )
      {
        const FT_TS_String*  char_name =
              (const FT_TS_String*)loader.encoding_table.elements[charcode];


        type1->encoding.char_index[charcode] = 0;
        type1->encoding.char_name [charcode] = ".notdef";

        if ( char_name )
          for ( idx = 0; idx < type1->num_glyphs; idx++ )
          {
            const FT_TS_String*  glyph_name = type1->glyph_names[idx];


            if ( ft_strcmp( char_name, glyph_name ) == 0 )
            {
              type1->encoding.char_index[charcode] = (FT_TS_UShort)idx;
              type1->encoding.char_name [charcode] = glyph_name;

              /* Change min/max encoded char only if glyph name is */
              /* not /.notdef                                      */
              if ( ft_strcmp( ".notdef", glyph_name ) != 0 )
              {
                if ( charcode < min_char )
                  min_char = charcode;
                if ( charcode >= max_char )
                  max_char = charcode + 1;
              }
              break;
            }
          }
      }

      type1->encoding.code_first = min_char;
      type1->encoding.code_last  = max_char;
      type1->encoding.num_chars  = loader.num_chars;
    }

  Exit:
    t42_loader_done( &loader );
    if ( error )
    {
      FT_TS_FREE( face->ttf_data );
      face->ttf_size = 0;
    }
    return error;
  }


  /***************** Driver Functions *************/


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_Face_Init( FT_TS_Stream      stream,
                 FT_TS_Face        t42face,       /* T42_Face */
                 FT_TS_Int         face_index,
                 FT_TS_Int         num_params,
                 FT_TS_Parameter*  params )
  {
    T42_Face            face  = (T42_Face)t42face;
    FT_TS_Error            error;
    FT_TS_Service_PsCMaps  psnames;
    PSAux_Service       psaux;
    FT_TS_Face             root  = (FT_TS_Face)&face->root;
    T1_Font             type1 = &face->type1;
    PS_FontInfo         info  = &type1->font_info;

    FT_TS_UNUSED( num_params );
    FT_TS_UNUSED( params );
    FT_TS_UNUSED( stream );


    face->ttf_face       = NULL;
    face->root.num_faces = 1;

    FT_TS_FACE_FIND_GLOBAL_SERVICE( face, psnames, POSTSCRIPT_CMAPS );
    face->psnames = psnames;

    face->psaux = FT_TS_Get_Module_Interface( FT_TS_FACE_LIBRARY( face ),
                                           "psaux" );
    psaux = (PSAux_Service)face->psaux;
    if ( !psaux )
    {
      FT_TS_ERROR(( "T42_Face_Init: cannot access `psaux' module\n" ));
      error = FT_TS_THROW( Missing_Module );
      goto Exit;
    }

    FT_TS_TRACE2(( "Type 42 driver\n" ));

    /* open the tokenizer, this will also check the font format */
    error = T42_Open_Face( face );
    if ( error )
      goto Exit;

    /* if we just wanted to check the format, leave successfully now */
    if ( face_index < 0 )
      goto Exit;

    /* check the face index */
    if ( ( face_index & 0xFFFF ) > 0 )
    {
      FT_TS_ERROR(( "T42_Face_Init: invalid face index\n" ));
      error = FT_TS_THROW( Invalid_Argument );
      goto Exit;
    }

    /* Now load the font program into the face object */

    /* Init the face object fields */
    /* Now set up root face fields */

    root->num_glyphs   = type1->num_glyphs;
    root->num_charmaps = 0;
    root->face_index   = 0;

    root->face_flags |= FT_TS_FACE_FLAG_SCALABLE    |
                        FT_TS_FACE_FLAG_HORIZONTAL  |
                        FT_TS_FACE_FLAG_GLYPH_NAMES;

    if ( info->is_fixed_pitch )
      root->face_flags |= FT_TS_FACE_FLAG_FIXED_WIDTH;

#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
    root->face_flags |= FT_TS_FACE_FLAG_HINTER;
#endif

    /* XXX: TODO -- add kerning with .afm support */

    /* get style name -- be careful, some broken fonts only */
    /* have a `/FontName' dictionary entry!                 */
    root->family_name = info->family_name;
    /* assume "Regular" style if we don't know better */
    root->style_name = (char *)"Regular";
    if ( root->family_name )
    {
      char*  full   = info->full_name;
      char*  family = root->family_name;


      if ( full )
      {
        while ( *full )
        {
          if ( *full == *family )
          {
            family++;
            full++;
          }
          else
          {
            if ( *full == ' ' || *full == '-' )
              full++;
            else if ( *family == ' ' || *family == '-' )
              family++;
            else
            {
              if ( !*family )
                root->style_name = full;
              break;
            }
          }
        }
      }
    }
    else
    {
      /* do we have a `/FontName'? */
      if ( type1->font_name )
        root->family_name = type1->font_name;
    }

    /* no embedded bitmap support */
    root->num_fixed_sizes = 0;
    root->available_sizes = NULL;

    /* Load the TTF font embedded in the T42 font */
    {
      FT_TS_Open_Args  args;


      args.flags       = FT_TS_OPEN_MEMORY | FT_TS_OPEN_DRIVER;
      args.driver      = FT_TS_Get_Module( FT_TS_FACE_LIBRARY( face ),
                                        "truetype" );
      args.memory_base = face->ttf_data;
      args.memory_size = face->ttf_size;

      if ( num_params )
      {
        args.flags     |= FT_TS_OPEN_PARAMS;
        args.num_params = num_params;
        args.params     = params;
      }

      error = FT_TS_Open_Face( FT_TS_FACE_LIBRARY( face ),
                            &args, 0, &face->ttf_face );
    }

    if ( error )
      goto Exit;

    FT_TS_Done_Size( face->ttf_face->size );

    /* Ignore info in FontInfo dictionary and use the info from the  */
    /* loaded TTF font.  The PostScript interpreter also ignores it. */
    root->bbox         = face->ttf_face->bbox;
    root->units_per_EM = face->ttf_face->units_per_EM;

    root->ascender  = face->ttf_face->ascender;
    root->descender = face->ttf_face->descender;
    root->height    = face->ttf_face->height;

    root->max_advance_width  = face->ttf_face->max_advance_width;
    root->max_advance_height = face->ttf_face->max_advance_height;

    root->underline_position  = (FT_TS_Short)info->underline_position;
    root->underline_thickness = (FT_TS_Short)info->underline_thickness;

    /* compute style flags */
    root->style_flags = 0;
    if ( info->italic_angle )
      root->style_flags |= FT_TS_STYLE_FLAG_ITALIC;

    if ( face->ttf_face->style_flags & FT_TS_STYLE_FLAG_BOLD )
      root->style_flags |= FT_TS_STYLE_FLAG_BOLD;

    if ( face->ttf_face->face_flags & FT_TS_FACE_FLAG_VERTICAL )
      root->face_flags |= FT_TS_FACE_FLAG_VERTICAL;

    {
      if ( psnames )
      {
        FT_TS_CharMapRec    charmap;
        T1_CMap_Classes  cmap_classes = psaux->t1_cmap_classes;
        FT_TS_CMap_Class    clazz;


        charmap.face = root;

        /* first of all, try to synthesize a Unicode charmap */
        charmap.platform_id = TT_PLATFORM_MICROSOFT;
        charmap.encoding_id = TT_MS_ID_UNICODE_CS;
        charmap.encoding    = FT_TS_ENCODING_UNICODE;

        error = FT_TS_CMap_New( cmap_classes->unicode, NULL, &charmap, NULL );
        if ( error                                      &&
             FT_TS_ERR_NEQ( error, No_Unicode_Glyph_Name ) &&
             FT_TS_ERR_NEQ( error, Unimplemented_Feature ) )
          goto Exit;
        error = FT_TS_Err_Ok;

        /* now, generate an Adobe Standard encoding when appropriate */
        charmap.platform_id = TT_PLATFORM_ADOBE;
        clazz               = NULL;

        switch ( type1->encoding_type )
        {
        case T1_ENCODING_TYPE_STANDARD:
          charmap.encoding    = FT_TS_ENCODING_ADOBE_STANDARD;
          charmap.encoding_id = TT_ADOBE_ID_STANDARD;
          clazz               = cmap_classes->standard;
          break;

        case T1_ENCODING_TYPE_EXPERT:
          charmap.encoding    = FT_TS_ENCODING_ADOBE_EXPERT;
          charmap.encoding_id = TT_ADOBE_ID_EXPERT;
          clazz               = cmap_classes->expert;
          break;

        case T1_ENCODING_TYPE_ARRAY:
          charmap.encoding    = FT_TS_ENCODING_ADOBE_CUSTOM;
          charmap.encoding_id = TT_ADOBE_ID_CUSTOM;
          clazz               = cmap_classes->custom;
          break;

        case T1_ENCODING_TYPE_ISOLATIN1:
          charmap.encoding    = FT_TS_ENCODING_ADOBE_LATIN_1;
          charmap.encoding_id = TT_ADOBE_ID_LATIN_1;
          clazz               = cmap_classes->unicode;
          break;

        default:
          ;
        }

        if ( clazz )
          error = FT_TS_CMap_New( clazz, NULL, &charmap, NULL );
      }
    }
  Exit:
    return error;
  }


  FT_TS_LOCAL_DEF( void )
  T42_Face_Done( FT_TS_Face  t42face )
  {
    T42_Face     face = (T42_Face)t42face;
    T1_Font      type1;
    PS_FontInfo  info;
    FT_TS_Memory    memory;


    if ( !face )
      return;

    type1  = &face->type1;
    info   = &type1->font_info;
    memory = face->root.memory;

    /* delete internal ttf face prior to freeing face->ttf_data */
    if ( face->ttf_face )
      FT_TS_Done_Face( face->ttf_face );

    /* release font info strings */
    FT_TS_FREE( info->version );
    FT_TS_FREE( info->notice );
    FT_TS_FREE( info->full_name );
    FT_TS_FREE( info->family_name );
    FT_TS_FREE( info->weight );

    /* release top dictionary */
    FT_TS_FREE( type1->charstrings_len );
    FT_TS_FREE( type1->charstrings );
    FT_TS_FREE( type1->glyph_names );

    FT_TS_FREE( type1->charstrings_block );
    FT_TS_FREE( type1->glyph_names_block );

    FT_TS_FREE( type1->encoding.char_index );
    FT_TS_FREE( type1->encoding.char_name );
    FT_TS_FREE( type1->font_name );

    FT_TS_FREE( face->ttf_data );

#if 0
    /* release afm data if present */
    if ( face->afm_data )
      T1_Done_AFM( memory, (T1_AFM*)face->afm_data );
#endif

    /* release unicode map, if any */
    FT_TS_FREE( face->unicode_map.maps );
    face->unicode_map.num_maps = 0;

    face->root.family_name = NULL;
    face->root.style_name  = NULL;
  }


  /**************************************************************************
   *
   * @Function:
   *   T42_Driver_Init
   *
   * @Description:
   *   Initializes a given Type 42 driver object.
   *
   * @Input:
   *   driver ::
   *     A handle to the target driver object.
   *
   * @Return:
   *   FreeType error code.  0 means success.
   */
  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_Driver_Init( FT_TS_Module  module )        /* T42_Driver */
  {
    T42_Driver  driver = (T42_Driver)module;
    FT_TS_Module   ttmodule;


    ttmodule = FT_TS_Get_Module( module->library, "truetype" );
    if ( !ttmodule )
    {
      FT_TS_ERROR(( "T42_Driver_Init: cannot access `truetype' module\n" ));
      return FT_TS_THROW( Missing_Module );
    }

    driver->ttclazz = (FT_TS_Driver_Class)ttmodule->clazz;

    return FT_TS_Err_Ok;
  }


  FT_TS_LOCAL_DEF( void )
  T42_Driver_Done( FT_TS_Module  module )
  {
    FT_TS_UNUSED( module );
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_Size_Init( FT_TS_Size  size )         /* T42_Size */
  {
    T42_Size  t42size = (T42_Size)size;
    FT_TS_Face   face    = size->face;
    T42_Face  t42face = (T42_Face)face;
    FT_TS_Size   ttsize;
    FT_TS_Error  error;


    error = FT_TS_New_Size( t42face->ttf_face, &ttsize );
    if ( !error )
      t42size->ttsize = ttsize;

    FT_TS_Activate_Size( ttsize );

    return error;
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_Size_Request( FT_TS_Size          t42size,      /* T42_Size */
                    FT_TS_Size_Request  req )
  {
    T42_Size  size = (T42_Size)t42size;
    T42_Face  face = (T42_Face)t42size->face;
    FT_TS_Error  error;


    FT_TS_Activate_Size( size->ttsize );

    error = FT_TS_Request_Size( face->ttf_face, req );
    if ( !error )
      t42size->metrics = face->ttf_face->size->metrics;

    return error;
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_Size_Select( FT_TS_Size   t42size,         /* T42_Size */
                   FT_TS_ULong  strike_index )
  {
    T42_Size  size = (T42_Size)t42size;
    T42_Face  face = (T42_Face)t42size->face;
    FT_TS_Error  error;


    FT_TS_Activate_Size( size->ttsize );

    error = FT_TS_Select_Size( face->ttf_face, (FT_TS_Int)strike_index );
    if ( !error )
      t42size->metrics = face->ttf_face->size->metrics;

    return error;

  }


  FT_TS_LOCAL_DEF( void )
  T42_Size_Done( FT_TS_Size  t42size )             /* T42_Size */
  {
    T42_Size     size    = (T42_Size)t42size;
    FT_TS_Face      face    = t42size->face;
    T42_Face     t42face = (T42_Face)face;
    FT_TS_ListNode  node;


    node = FT_TS_List_Find( &t42face->ttf_face->sizes_list, size->ttsize );
    if ( node )
    {
      FT_TS_Done_Size( size->ttsize );
      size->ttsize = NULL;
    }
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_GlyphSlot_Init( FT_TS_GlyphSlot  t42slot )        /* T42_GlyphSlot */
  {
    T42_GlyphSlot  slot    = (T42_GlyphSlot)t42slot;
    FT_TS_Face        face    = t42slot->face;
    T42_Face       t42face = (T42_Face)face;
    FT_TS_GlyphSlot   ttslot;
    FT_TS_Memory      memory  = face->memory;
    FT_TS_Error       error   = FT_TS_Err_Ok;


    if ( !face->glyph )
    {
      /* First glyph slot for this face */
      slot->ttslot = t42face->ttf_face->glyph;
    }
    else
    {
      error = FT_TS_New_GlyphSlot( t42face->ttf_face, &ttslot );
      if ( !error )
        slot->ttslot = ttslot;
    }

    /* share the loader so that the autohinter can see it */
    FT_TS_GlyphLoader_Done( slot->ttslot->internal->loader );
    FT_TS_FREE( slot->ttslot->internal );
    slot->ttslot->internal = t42slot->internal;

    return error;
  }


  FT_TS_LOCAL_DEF( void )
  T42_GlyphSlot_Done( FT_TS_GlyphSlot  t42slot )       /* T42_GlyphSlot */
  {
    T42_GlyphSlot  slot = (T42_GlyphSlot)t42slot;


    /* do not destroy the inherited internal structure just yet */
    slot->ttslot->internal = NULL;
    FT_TS_Done_GlyphSlot( slot->ttslot );
  }


  static void
  t42_glyphslot_clear( FT_TS_GlyphSlot  slot )
  {
    /* free bitmap if needed */
    ft_glyphslot_free_bitmap( slot );

    /* clear all public fields in the glyph slot */
    FT_TS_ZERO( &slot->metrics );
    FT_TS_ZERO( &slot->outline );
    FT_TS_ZERO( &slot->bitmap );

    slot->bitmap_left   = 0;
    slot->bitmap_top    = 0;
    slot->num_subglyphs = 0;
    slot->subglyphs     = NULL;
    slot->control_data  = NULL;
    slot->control_len   = 0;
    slot->other         = NULL;
    slot->format        = FT_TS_GLYPH_FORMAT_NONE;

    slot->linearHoriAdvance = 0;
    slot->linearVertAdvance = 0;
  }


  FT_TS_LOCAL_DEF( FT_TS_Error )
  T42_GlyphSlot_Load( FT_TS_GlyphSlot  glyph,
                      FT_TS_Size       size,
                      FT_TS_UInt       glyph_index,
                      FT_TS_Int32      load_flags )
  {
    FT_TS_Error         error;
    T42_GlyphSlot    t42slot = (T42_GlyphSlot)glyph;
    T42_Size         t42size = (T42_Size)size;
    T42_Face         t42face = (T42_Face)size->face;
    FT_TS_Driver_Class  ttclazz = ((T42_Driver)glyph->face->driver)->ttclazz;


    FT_TS_TRACE1(( "T42_GlyphSlot_Load: glyph index %d\n", glyph_index ));

    /* map T42 glyph index to embedded TTF's glyph index */
    glyph_index = (FT_TS_UInt)ft_strtol(
                    (const char *)t42face->type1.charstrings[glyph_index],
                    NULL, 10 );

    t42_glyphslot_clear( t42slot->ttslot );
    error = ttclazz->load_glyph( t42slot->ttslot,
                                 t42size->ttsize,
                                 glyph_index,
                                 load_flags | FT_TS_LOAD_NO_BITMAP );

    if ( !error )
    {
      glyph->metrics = t42slot->ttslot->metrics;

      glyph->linearHoriAdvance = t42slot->ttslot->linearHoriAdvance;
      glyph->linearVertAdvance = t42slot->ttslot->linearVertAdvance;

      glyph->format  = t42slot->ttslot->format;
      glyph->outline = t42slot->ttslot->outline;

      glyph->bitmap      = t42slot->ttslot->bitmap;
      glyph->bitmap_left = t42slot->ttslot->bitmap_left;
      glyph->bitmap_top  = t42slot->ttslot->bitmap_top;

      glyph->num_subglyphs = t42slot->ttslot->num_subglyphs;
      glyph->subglyphs     = t42slot->ttslot->subglyphs;

      glyph->control_data  = t42slot->ttslot->control_data;
      glyph->control_len   = t42slot->ttslot->control_len;
    }

    return error;
  }


/* END */
