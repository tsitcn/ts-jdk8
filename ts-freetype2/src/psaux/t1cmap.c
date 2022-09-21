/****************************************************************************
 *
 * t1cmap.c
 *
 *   Type 1 character map support (body).
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


#include "t1cmap.h"

#include <freetype/internal/ftdebug.h>

#include "psauxerr.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****          TYPE1 STANDARD (AND EXPERT) ENCODING CMAPS           *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  static void
  t1_cmap_std_init( T1_CMapStd  cmap,
                    FT_TS_Int      is_expert )
  {
    T1_Face             face    = (T1_Face)FT_TS_CMAP_FACE( cmap );
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)face->psnames;


    cmap->num_glyphs    = (FT_TS_UInt)face->type1.num_glyphs;
    cmap->glyph_names   = (const char* const*)face->type1.glyph_names;
    cmap->sid_to_string = psnames->adobe_std_strings;
    cmap->code_to_sid   = is_expert ? psnames->adobe_expert_encoding
                                    : psnames->adobe_std_encoding;

    FT_TS_ASSERT( cmap->code_to_sid );
  }


  FT_TS_CALLBACK_DEF( void )
  t1_cmap_std_done( T1_CMapStd  cmap )
  {
    cmap->num_glyphs    = 0;
    cmap->glyph_names   = NULL;
    cmap->sid_to_string = NULL;
    cmap->code_to_sid   = NULL;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  t1_cmap_std_char_index( T1_CMapStd  cmap,
                          FT_TS_UInt32   char_code )
  {
    FT_TS_UInt  result = 0;


    if ( char_code < 256 )
    {
      FT_TS_UInt      code, n;
      const char*  glyph_name;


      /* convert character code to Adobe SID string */
      code       = cmap->code_to_sid[char_code];
      glyph_name = cmap->sid_to_string( code );

      /* look for the corresponding glyph name */
      for ( n = 0; n < cmap->num_glyphs; n++ )
      {
        const char* gname = cmap->glyph_names[n];


        if ( gname && gname[0] == glyph_name[0]  &&
             ft_strcmp( gname, glyph_name ) == 0 )
        {
          result = n;
          break;
        }
      }
    }

    return result;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  t1_cmap_std_char_next( T1_CMapStd   cmap,
                         FT_TS_UInt32   *pchar_code )
  {
    FT_TS_UInt    result    = 0;
    FT_TS_UInt32  char_code = *pchar_code + 1;


    while ( char_code < 256 )
    {
      result = t1_cmap_std_char_index( cmap, char_code );
      if ( result != 0 )
        goto Exit;

      char_code++;
    }
    char_code = 0;

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  t1_cmap_standard_init( T1_CMapStd  cmap,
                         FT_TS_Pointer  pointer )
  {
    FT_TS_UNUSED( pointer );


    t1_cmap_std_init( cmap, 0 );
    return 0;
  }


  FT_TS_CALLBACK_TABLE_DEF const FT_TS_CMap_ClassRec
  t1_cmap_standard_class_rec =
  {
    sizeof ( T1_CMapStdRec ),

    (FT_TS_CMap_InitFunc)     t1_cmap_standard_init,   /* init       */
    (FT_TS_CMap_DoneFunc)     t1_cmap_std_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)t1_cmap_std_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) t1_cmap_std_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  };


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  t1_cmap_expert_init( T1_CMapStd  cmap,
                       FT_TS_Pointer  pointer )
  {
    FT_TS_UNUSED( pointer );


    t1_cmap_std_init( cmap, 1 );
    return 0;
  }

  FT_TS_CALLBACK_TABLE_DEF const FT_TS_CMap_ClassRec
  t1_cmap_expert_class_rec =
  {
    sizeof ( T1_CMapStdRec ),

    (FT_TS_CMap_InitFunc)     t1_cmap_expert_init,     /* init       */
    (FT_TS_CMap_DoneFunc)     t1_cmap_std_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)t1_cmap_std_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) t1_cmap_std_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    TYPE1 CUSTOM ENCODING CMAP                 *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  t1_cmap_custom_init( T1_CMapCustom  cmap,
                       FT_TS_Pointer     pointer )
  {
    T1_Face      face     = (T1_Face)FT_TS_CMAP_FACE( cmap );
    T1_Encoding  encoding = &face->type1.encoding;

    FT_TS_UNUSED( pointer );


    cmap->first   = (FT_TS_UInt)encoding->code_first;
    cmap->count   = (FT_TS_UInt)encoding->code_last - cmap->first;
    cmap->indices = encoding->char_index;

    FT_TS_ASSERT( cmap->indices );
    FT_TS_ASSERT( encoding->code_first <= encoding->code_last );

    return 0;
  }


  FT_TS_CALLBACK_DEF( void )
  t1_cmap_custom_done( T1_CMapCustom  cmap )
  {
    cmap->indices = NULL;
    cmap->first   = 0;
    cmap->count   = 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  t1_cmap_custom_char_index( T1_CMapCustom  cmap,
                             FT_TS_UInt32      char_code )
  {
    FT_TS_UInt    result = 0;


    if ( ( char_code >= cmap->first )                  &&
         ( char_code < ( cmap->first + cmap->count ) ) )
      result = cmap->indices[char_code];

    return result;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  t1_cmap_custom_char_next( T1_CMapCustom  cmap,
                            FT_TS_UInt32     *pchar_code )
  {
    FT_TS_UInt    result = 0;
    FT_TS_UInt32  char_code = *pchar_code;


    char_code++;

    if ( char_code < cmap->first )
      char_code = cmap->first;

    for ( ; char_code < ( cmap->first + cmap->count ); char_code++ )
    {
      result = cmap->indices[char_code];
      if ( result != 0 )
        goto Exit;
    }

    char_code = 0;

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_TS_CALLBACK_TABLE_DEF const FT_TS_CMap_ClassRec
  t1_cmap_custom_class_rec =
  {
    sizeof ( T1_CMapCustomRec ),

    (FT_TS_CMap_InitFunc)     t1_cmap_custom_init,        /* init       */
    (FT_TS_CMap_DoneFunc)     t1_cmap_custom_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)t1_cmap_custom_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) t1_cmap_custom_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  };


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****            TYPE1 SYNTHETIC UNICODE ENCODING CMAP              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_CALLBACK_DEF( const char * )
  psaux_get_glyph_name( T1_Face  face,
                        FT_TS_UInt  idx )
  {
    return face->type1.glyph_names[idx];
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  t1_cmap_unicode_init( PS_Unicodes  unicodes,
                        FT_TS_Pointer   pointer )
  {
    T1_Face             face    = (T1_Face)FT_TS_CMAP_FACE( unicodes );
    FT_TS_Memory           memory  = FT_TS_FACE_MEMORY( face );
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)face->psnames;

    FT_TS_UNUSED( pointer );


    if ( !psnames->unicodes_init )
      return FT_TS_THROW( Unimplemented_Feature );

    return psnames->unicodes_init( memory,
                                   unicodes,
                                   (FT_TS_UInt)face->type1.num_glyphs,
                                   (PS_GetGlyphNameFunc)&psaux_get_glyph_name,
                                   (PS_FreeGlyphNameFunc)NULL,
                                   (FT_TS_Pointer)face );
  }


  FT_TS_CALLBACK_DEF( void )
  t1_cmap_unicode_done( PS_Unicodes  unicodes )
  {
    FT_TS_Face    face   = FT_TS_CMAP_FACE( unicodes );
    FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


    FT_TS_FREE( unicodes->maps );
    unicodes->num_maps = 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  t1_cmap_unicode_char_index( PS_Unicodes  unicodes,
                              FT_TS_UInt32    char_code )
  {
    T1_Face             face    = (T1_Face)FT_TS_CMAP_FACE( unicodes );
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)face->psnames;


    return psnames->unicodes_char_index( unicodes, char_code );
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  t1_cmap_unicode_char_next( PS_Unicodes  unicodes,
                             FT_TS_UInt32   *pchar_code )
  {
    T1_Face             face    = (T1_Face)FT_TS_CMAP_FACE( unicodes );
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)face->psnames;


    return psnames->unicodes_char_next( unicodes, pchar_code );
  }


  FT_TS_CALLBACK_TABLE_DEF const FT_TS_CMap_ClassRec
  t1_cmap_unicode_class_rec =
  {
    sizeof ( PS_UnicodesRec ),

    (FT_TS_CMap_InitFunc)     t1_cmap_unicode_init,        /* init       */
    (FT_TS_CMap_DoneFunc)     t1_cmap_unicode_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)t1_cmap_unicode_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) t1_cmap_unicode_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  };


/* END */
