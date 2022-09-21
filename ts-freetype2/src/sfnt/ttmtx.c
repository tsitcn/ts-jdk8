/****************************************************************************
 *
 * ttmtx.c
 *
 *   Load the metrics tables common to TTF and OTF fonts (body).
 *
 * Copyright (C) 2006-2022 by
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
#include <freetype/internal/ftstream.h>
#include <freetype/tttags.h>

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
#include <freetype/internal/services/svmetric.h>
#endif

#include "ttmtx.h"

#include "sferrors.h"


  /* IMPORTANT: The TT_HoriHeader and TT_VertHeader structures should   */
  /*            be identical except for the names of their fields,      */
  /*            which are different.                                    */
  /*                                                                    */
  /*            This ensures that `tt_face_load_hmtx' is able to read   */
  /*            both the horizontal and vertical headers.               */


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  ttmtx


  /**************************************************************************
   *
   * @Function:
   *   tt_face_load_hmtx
   *
   * @Description:
   *   Load the `hmtx' or `vmtx' table into a face object.
   *
   * @Input:
   *   face ::
   *     A handle to the target face object.
   *
   *   stream ::
   *     The input stream.
   *
   *   vertical ::
   *     A boolean flag.  If set, load `vmtx'.
   *
   * @Return:
   *   FreeType error code.  0 means success.
   */
  FT_TS_LOCAL_DEF( FT_TS_Error )
  tt_face_load_hmtx( TT_Face    face,
                     FT_TS_Stream  stream,
                     FT_TS_Bool    vertical )
  {
    FT_TS_Error   error;
    FT_TS_ULong   tag, table_size;
    FT_TS_ULong*  ptable_offset;
    FT_TS_ULong*  ptable_size;


    if ( vertical )
    {
      tag           = TTAG_vmtx;
      ptable_offset = &face->vert_metrics_offset;
      ptable_size   = &face->vert_metrics_size;
    }
    else
    {
      tag           = TTAG_hmtx;
      ptable_offset = &face->horz_metrics_offset;
      ptable_size   = &face->horz_metrics_size;
    }

    error = face->goto_table( face, tag, stream, &table_size );
    if ( error )
      goto Fail;

    *ptable_size   = table_size;
    *ptable_offset = FT_TS_STREAM_POS();

  Fail:
    return error;
  }


  /**************************************************************************
   *
   * @Function:
   *   tt_face_load_hhea
   *
   * @Description:
   *   Load the `hhea' or 'vhea' table into a face object.
   *
   * @Input:
   *   face ::
   *     A handle to the target face object.
   *
   *   stream ::
   *     The input stream.
   *
   *   vertical ::
   *     A boolean flag.  If set, load `vhea'.
   *
   * @Return:
   *   FreeType error code.  0 means success.
   */
  FT_TS_LOCAL_DEF( FT_TS_Error )
  tt_face_load_hhea( TT_Face    face,
                     FT_TS_Stream  stream,
                     FT_TS_Bool    vertical )
  {
    FT_TS_Error        error;
    TT_HoriHeader*  header;

    static const FT_TS_Frame_Field  metrics_header_fields[] =
    {
#undef  FT_TS_STRUCTURE
#define FT_TS_STRUCTURE  TT_HoriHeader

      FT_TS_FRAME_START( 36 ),
        FT_TS_FRAME_ULONG ( Version ),
        FT_TS_FRAME_SHORT ( Ascender ),
        FT_TS_FRAME_SHORT ( Descender ),
        FT_TS_FRAME_SHORT ( Line_Gap ),
        FT_TS_FRAME_USHORT( advance_Width_Max ),
        FT_TS_FRAME_SHORT ( min_Left_Side_Bearing ),
        FT_TS_FRAME_SHORT ( min_Right_Side_Bearing ),
        FT_TS_FRAME_SHORT ( xMax_Extent ),
        FT_TS_FRAME_SHORT ( caret_Slope_Rise ),
        FT_TS_FRAME_SHORT ( caret_Slope_Run ),
        FT_TS_FRAME_SHORT ( caret_Offset ),
        FT_TS_FRAME_SHORT ( Reserved[0] ),
        FT_TS_FRAME_SHORT ( Reserved[1] ),
        FT_TS_FRAME_SHORT ( Reserved[2] ),
        FT_TS_FRAME_SHORT ( Reserved[3] ),
        FT_TS_FRAME_SHORT ( metric_Data_Format ),
        FT_TS_FRAME_USHORT( number_Of_HMetrics ),
      FT_TS_FRAME_END
    };


    if ( vertical )
    {
      void  *v = &face->vertical;


      error = face->goto_table( face, TTAG_vhea, stream, 0 );
      if ( error )
        goto Fail;

      header = (TT_HoriHeader*)v;
    }
    else
    {
      error = face->goto_table( face, TTAG_hhea, stream, 0 );
      if ( error )
        goto Fail;

      header = &face->horizontal;
    }

    if ( FT_TS_STREAM_READ_FIELDS( metrics_header_fields, header ) )
      goto Fail;

    FT_TS_TRACE3(( "Ascender:          %5d\n", header->Ascender ));
    FT_TS_TRACE3(( "Descender:         %5d\n", header->Descender ));
    FT_TS_TRACE3(( "number_Of_Metrics: %5u\n", header->number_Of_HMetrics ));

    header->long_metrics  = NULL;
    header->short_metrics = NULL;

  Fail:
    return error;
  }


  /**************************************************************************
   *
   * @Function:
   *   tt_face_get_metrics
   *
   * @Description:
   *   Return the horizontal or vertical metrics in font units for a
   *   given glyph.  The values are the left side bearing (top side
   *   bearing for vertical metrics) and advance width (advance height
   *   for vertical metrics).
   *
   * @Input:
   *   face ::
   *     A pointer to the TrueType face structure.
   *
   *   vertical ::
   *     If set to TRUE, get vertical metrics.
   *
   *   gindex ::
   *     The glyph index.
   *
   * @Output:
   *   abearing ::
   *     The bearing, either left side or top side.
   *
   *   aadvance ::
   *     The advance width or advance height, depending on
   *     the `vertical' flag.
   */
  FT_TS_LOCAL_DEF( void )
  tt_face_get_metrics( TT_Face     face,
                       FT_TS_Bool     vertical,
                       FT_TS_UInt     gindex,
                       FT_TS_Short   *abearing,
                       FT_TS_UShort  *aadvance )
  {
    FT_TS_Error        error;
    FT_TS_Stream       stream = face->root.stream;
    TT_HoriHeader*  header;
    FT_TS_ULong        table_pos, table_size, table_end;
    FT_TS_UShort       k;

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
    FT_TS_Service_MetricsVariations  var =
      (FT_TS_Service_MetricsVariations)face->var;
#endif


    if ( vertical )
    {
      void*  v = &face->vertical;


      header     = (TT_HoriHeader*)v;
      table_pos  = face->vert_metrics_offset;
      table_size = face->vert_metrics_size;
    }
    else
    {
      header     = &face->horizontal;
      table_pos  = face->horz_metrics_offset;
      table_size = face->horz_metrics_size;
    }

    table_end = table_pos + table_size;

    k = header->number_Of_HMetrics;

    if ( k > 0 )
    {
      if ( gindex < (FT_TS_UInt)k )
      {
        table_pos += 4 * gindex;
        if ( table_pos + 4 > table_end )
          goto NoData;

        if ( FT_TS_STREAM_SEEK( table_pos ) ||
             FT_TS_READ_USHORT( *aadvance ) ||
             FT_TS_READ_SHORT( *abearing )  )
          goto NoData;
      }
      else
      {
        table_pos += 4 * ( k - 1 );
        if ( table_pos + 2 > table_end )
          goto NoData;

        if ( FT_TS_STREAM_SEEK( table_pos ) ||
             FT_TS_READ_USHORT( *aadvance ) )
          goto NoData;

        table_pos += 4 + 2 * ( gindex - k );
        if ( table_pos + 2 > table_end )
          *abearing = 0;
        else
        {
          if ( FT_TS_STREAM_SEEK( table_pos ) )
            *abearing = 0;
          else
            (void)FT_TS_READ_SHORT( *abearing );
        }
      }
    }
    else
    {
    NoData:
      *abearing = 0;
      *aadvance = 0;
    }

#ifdef TT_CONFIG_OPTION_GX_VAR_SUPPORT
    if ( var )
    {
      FT_TS_Face  f = FT_TS_FACE( face );
      FT_TS_Int   a = (FT_TS_Int)*aadvance;
      FT_TS_Int   b = (FT_TS_Int)*abearing;


      if ( vertical )
      {
        if ( var->vadvance_adjust )
          var->vadvance_adjust( f, gindex, &a );
        if ( var->tsb_adjust )
          var->tsb_adjust( f, gindex, &b );
      }
      else
      {
        if ( var->hadvance_adjust )
          var->hadvance_adjust( f, gindex, &a );
        if ( var->lsb_adjust )
          var->lsb_adjust( f, gindex, &b );
      }

      *aadvance = (FT_TS_UShort)a;
      *abearing = (FT_TS_Short)b;
    }
#endif
  }


/* END */
