/****************************************************************************
 *
 * cffcmap.c
 *
 *   CFF character mapping table (cmap) support (body).
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
#include "cffcmap.h"
#include "cffload.h"

#include "cfferrs.h"


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****           CFF STANDARD (AND EXPERT) ENCODING CMAPS            *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_CALLBACK_DEF( FT_TS_Error )
  cff_cmap_encoding_init( CFF_CMapStd  cmap,
                          FT_TS_Pointer   pointer )
  {
    TT_Face       face     = (TT_Face)FT_TS_CMAP_FACE( cmap );
    CFF_Font      cff      = (CFF_Font)face->extra.data;
    CFF_Encoding  encoding = &cff->encoding;

    FT_TS_UNUSED( pointer );


    cmap->gids  = encoding->codes;

    return 0;
  }


  FT_TS_CALLBACK_DEF( void )
  cff_cmap_encoding_done( CFF_CMapStd  cmap )
  {
    cmap->gids  = NULL;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  cff_cmap_encoding_char_index( CFF_CMapStd  cmap,
                                FT_TS_UInt32    char_code )
  {
    FT_TS_UInt  result = 0;


    if ( char_code < 256 )
      result = cmap->gids[char_code];

    return result;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  cff_cmap_encoding_char_next( CFF_CMapStd   cmap,
                               FT_TS_UInt32    *pchar_code )
  {
    FT_TS_UInt    result    = 0;
    FT_TS_UInt32  char_code = *pchar_code;


    *pchar_code = 0;

    if ( char_code < 255 )
    {
      FT_TS_UInt  code = (FT_TS_UInt)( char_code + 1 );


      for (;;)
      {
        if ( code >= 256 )
          break;

        result = cmap->gids[code];
        if ( result != 0 )
        {
          *pchar_code = code;
          break;
        }

        code++;
      }
    }
    return result;
  }


  FT_TS_DEFINE_CMAP_CLASS(
    cff_cmap_encoding_class_rec,

    sizeof ( CFF_CMapStdRec ),

    (FT_TS_CMap_InitFunc)     cff_cmap_encoding_init,        /* init       */
    (FT_TS_CMap_DoneFunc)     cff_cmap_encoding_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)cff_cmap_encoding_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) cff_cmap_encoding_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  )


  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****              CFF SYNTHETIC UNICODE ENCODING CMAP              *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/

  FT_TS_CALLBACK_DEF( const char* )
  cff_sid_to_glyph_name( TT_Face  face,
                         FT_TS_UInt  idx )
  {
    CFF_Font     cff     = (CFF_Font)face->extra.data;
    CFF_Charset  charset = &cff->charset;
    FT_TS_UInt      sid     = charset->sids[idx];


    return cff_index_get_sid_string( cff, sid );
  }


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  cff_cmap_unicode_init( PS_Unicodes  unicodes,
                         FT_TS_Pointer   pointer )
  {
    TT_Face             face    = (TT_Face)FT_TS_CMAP_FACE( unicodes );
    FT_TS_Memory           memory  = FT_TS_FACE_MEMORY( face );
    CFF_Font            cff     = (CFF_Font)face->extra.data;
    CFF_Charset         charset = &cff->charset;
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)cff->psnames;

    FT_TS_UNUSED( pointer );


    /* can't build Unicode map for CID-keyed font */
    /* because we don't know glyph names.         */
    if ( !charset->sids )
      return FT_TS_THROW( No_Unicode_Glyph_Name );

    if ( !psnames->unicodes_init )
      return FT_TS_THROW( Unimplemented_Feature );

    return psnames->unicodes_init( memory,
                                   unicodes,
                                   cff->num_glyphs,
                                   (PS_GetGlyphNameFunc)&cff_sid_to_glyph_name,
                                   (PS_FreeGlyphNameFunc)NULL,
                                   (FT_TS_Pointer)face );
  }


  FT_TS_CALLBACK_DEF( void )
  cff_cmap_unicode_done( PS_Unicodes  unicodes )
  {
    FT_TS_Face    face   = FT_TS_CMAP_FACE( unicodes );
    FT_TS_Memory  memory = FT_TS_FACE_MEMORY( face );


    FT_TS_FREE( unicodes->maps );
    unicodes->num_maps = 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  cff_cmap_unicode_char_index( PS_Unicodes  unicodes,
                               FT_TS_UInt32    char_code )
  {
    TT_Face             face    = (TT_Face)FT_TS_CMAP_FACE( unicodes );
    CFF_Font            cff     = (CFF_Font)face->extra.data;
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)cff->psnames;


    return psnames->unicodes_char_index( unicodes, char_code );
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  cff_cmap_unicode_char_next( PS_Unicodes  unicodes,
                              FT_TS_UInt32   *pchar_code )
  {
    TT_Face             face    = (TT_Face)FT_TS_CMAP_FACE( unicodes );
    CFF_Font            cff     = (CFF_Font)face->extra.data;
    FT_TS_Service_PsCMaps  psnames = (FT_TS_Service_PsCMaps)cff->psnames;


    return psnames->unicodes_char_next( unicodes, pchar_code );
  }


  FT_TS_DEFINE_CMAP_CLASS(
    cff_cmap_unicode_class_rec,

    sizeof ( PS_UnicodesRec ),

    (FT_TS_CMap_InitFunc)     cff_cmap_unicode_init,        /* init       */
    (FT_TS_CMap_DoneFunc)     cff_cmap_unicode_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)cff_cmap_unicode_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) cff_cmap_unicode_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  )


/* END */
