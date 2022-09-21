/****************************************************************************
 *
 * pfrcmap.c
 *
 *   FreeType PFR cmap handling (body).
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
#include "pfrcmap.h"
#include "pfrobjs.h"

#include "pfrerror.h"


  FT_TS_CALLBACK_DEF( FT_TS_Error )
  pfr_cmap_init( PFR_CMap    cmap,
                 FT_TS_Pointer  pointer )
  {
    FT_TS_Error  error = FT_TS_Err_Ok;
    PFR_Face  face  = (PFR_Face)FT_TS_CMAP_FACE( cmap );

    FT_TS_UNUSED( pointer );


    cmap->num_chars = face->phy_font.num_chars;
    cmap->chars     = face->phy_font.chars;

    /* just for safety, check that the character entries are correctly */
    /* sorted in increasing character code order                       */
    {
      FT_TS_UInt  n;


      for ( n = 1; n < cmap->num_chars; n++ )
      {
        if ( cmap->chars[n - 1].char_code >= cmap->chars[n].char_code )
        {
          error = FT_TS_THROW( Invalid_Table );
          goto Exit;
        }
      }
    }

  Exit:
    return error;
  }


  FT_TS_CALLBACK_DEF( void )
  pfr_cmap_done( PFR_CMap  cmap )
  {
    cmap->chars     = NULL;
    cmap->num_chars = 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt )
  pfr_cmap_char_index( PFR_CMap   cmap,
                       FT_TS_UInt32  char_code )
  {
    FT_TS_UInt  min = 0;
    FT_TS_UInt  max = cmap->num_chars;


    while ( min < max )
    {
      PFR_Char  gchar;
      FT_TS_UInt   mid;


      mid   = min + ( max - min ) / 2;
      gchar = cmap->chars + mid;

      if ( gchar->char_code == char_code )
        return mid + 1;

      if ( gchar->char_code < char_code )
        min = mid + 1;
      else
        max = mid;
    }
    return 0;
  }


  FT_TS_CALLBACK_DEF( FT_TS_UInt32 )
  pfr_cmap_char_next( PFR_CMap    cmap,
                      FT_TS_UInt32  *pchar_code )
  {
    FT_TS_UInt    result    = 0;
    FT_TS_UInt32  char_code = *pchar_code + 1;


  Restart:
    {
      FT_TS_UInt   min = 0;
      FT_TS_UInt   max = cmap->num_chars;
      FT_TS_UInt   mid;
      PFR_Char  gchar;


      while ( min < max )
      {
        mid   = min + ( ( max - min ) >> 1 );
        gchar = cmap->chars + mid;

        if ( gchar->char_code == char_code )
        {
          result = mid;
          if ( result != 0 )
          {
            result++;
            goto Exit;
          }

          char_code++;
          goto Restart;
        }

        if ( gchar->char_code < char_code )
          min = mid + 1;
        else
          max = mid;
      }

      /* we didn't find it, but we have a pair just above it */
      char_code = 0;

      if ( min < cmap->num_chars )
      {
        gchar  = cmap->chars + min;
        result = min;
        if ( result != 0 )
        {
          result++;
          char_code = gchar->char_code;
        }
      }
    }

  Exit:
    *pchar_code = char_code;
    return result;
  }


  FT_TS_CALLBACK_TABLE_DEF const FT_TS_CMap_ClassRec
  pfr_cmap_class_rec =
  {
    sizeof ( PFR_CMapRec ),

    (FT_TS_CMap_InitFunc)     pfr_cmap_init,        /* init       */
    (FT_TS_CMap_DoneFunc)     pfr_cmap_done,        /* done       */
    (FT_TS_CMap_CharIndexFunc)pfr_cmap_char_index,  /* char_index */
    (FT_TS_CMap_CharNextFunc) pfr_cmap_char_next,   /* char_next  */

    (FT_TS_CMap_CharVarIndexFunc)    NULL,  /* char_var_index   */
    (FT_TS_CMap_CharVarIsDefaultFunc)NULL,  /* char_var_default */
    (FT_TS_CMap_VariantListFunc)     NULL,  /* variant_list     */
    (FT_TS_CMap_CharVariantListFunc) NULL,  /* charvariant_list */
    (FT_TS_CMap_VariantCharListFunc) NULL   /* variantchar_list */
  };


/* END */
