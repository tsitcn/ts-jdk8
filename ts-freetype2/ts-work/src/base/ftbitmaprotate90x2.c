/****************************************************************************
 *
 * ftbitmap90x2.c
 *
 *   FreeType utility functions for bitmaps (body).
 *
 * Copyright (C) 2004-2021 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/**
 TSIT {{{{{{{{{{
 */

#include <freetype/internal/ftdebug.h>

#include <freetype/ftbitmap.h>
#include <freetype/ftbitmapext.h>
#include <freetype/ftbitmaprotate.h>
#include <freetype/ftbitmappixel.h>
#include <freetype/ftimage.h>
#include <freetype/internal/ftobjs.h>
#include <freetype/ftsynth.h>


DECLARE_FT_TS_BITMAP_ROTATE_INIT_90(2);

DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_90(2)
{
    pDst->pixel_mode = pSrc->pixel_mode;
    pPixelFuncs->set_width(pDst, pPixelFuncs->get_width(pSrc));
    pPixelFuncs->set_rows (pDst, pPixelFuncs->get_rows (pSrc));
    pPixelFuncs->set_pitch(pDst, pPixelFuncs->get_pitch(pSrc));
    return 0;
}

DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_ITALIC_90(2)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_ROTATE_INIT_4_BOLD_90(2)
{
    return 0;
}

DECLARE_FT_TS_BITMAP_ROTATE_DST_COORDS_90(2)
{
    pDstCoords[0] = src_width-1-xSrc;
    pDstCoords[1] = src_rows -1-ySrc;
    return 0;
}

DECLARE_FT_TS_BITMAP_ROTATE_SLOT_POSITION_90(2)
{
    //slot->bitmap_top  = -slot->bitmap_top;
    slot->bitmap_top  =  pPixelFuncs->get_rows(pSrc) - slot->bitmap_top;
    if (FT_TS_CHECK_BITMAP_SLOT_TUNING(flags))
    {
        switch (slot->face->size->metrics.height)
        {
            case FONT_SIZE_12:
            case FONT_SIZE_13:
            case FONT_SIZE_14:
                slot->bitmap_top --;
                break;

            case FONT_SIZE_15:
            case FONT_SIZE_17:
                slot->bitmap_left --;
                break;

            default:
                break;
        }
    }

    slot->bitmap_left = 0 - (pPixelFuncs->get_width(pSrc) + 1 + slot->bitmap_left);
    return 0;
}

DECLARE_FT_TS_BITMAP_ROTATE_ITALIC_ACTION_90(2)
{
    if (to_bottom)
    {
        FT_TS_Bitmap_Italic_Ver(pDst, degree, to_bottom, oblique, pSrc, slot, pPixelFuncs);
    }
    else
    {
        FT_TS_Bitmap_Italic_Hor(pDst, degree, to_bottom, oblique, pSrc, slot, pPixelFuncs);
    }
    return 0;
}

/**
 TSIT }}}}}}}}}}
 */

/* END */
