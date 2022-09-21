/****************************************************************************
 *
 * ftrender.h
 *
 *   FreeType renderer modules public interface (specification).
 *
 * Copyright (C) 1996-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTRENDER_H_
#define FTRENDER_H_


#include <freetype/ftmodapi.h>
#include <freetype/ftglyph.h>


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *   module_management
   *
   */


  /* create a new glyph object */
  typedef FT_TS_Error
  (*FT_TS_Glyph_InitFunc)( FT_TS_Glyph      glyph,
                        FT_TS_GlyphSlot  slot );

  /* destroys a given glyph object */
  typedef void
  (*FT_TS_Glyph_DoneFunc)( FT_TS_Glyph  glyph );

  typedef void
  (*FT_TS_Glyph_TransformFunc)( FT_TS_Glyph          glyph,
                             const FT_TS_Matrix*  matrix,
                             const FT_TS_Vector*  delta );

  typedef void
  (*FT_TS_Glyph_GetBBoxFunc)( FT_TS_Glyph  glyph,
                           FT_TS_BBox*  abbox );

  typedef FT_TS_Error
  (*FT_TS_Glyph_CopyFunc)( FT_TS_Glyph   source,
                        FT_TS_Glyph   target );

  typedef FT_TS_Error
  (*FT_TS_Glyph_PrepareFunc)( FT_TS_Glyph      glyph,
                           FT_TS_GlyphSlot  slot );

/* deprecated */
#define FT_TS_Glyph_Init_Func       FT_TS_Glyph_InitFunc
#define FT_TS_Glyph_Done_Func       FT_TS_Glyph_DoneFunc
#define FT_TS_Glyph_Transform_Func  FT_TS_Glyph_TransformFunc
#define FT_TS_Glyph_BBox_Func       FT_TS_Glyph_GetBBoxFunc
#define FT_TS_Glyph_Copy_Func       FT_TS_Glyph_CopyFunc
#define FT_TS_Glyph_Prepare_Func    FT_TS_Glyph_PrepareFunc


  struct  FT_TS_Glyph_Class_
  {
    FT_TS_Long                 glyph_size;
    FT_TS_Glyph_Format         glyph_format;

    FT_TS_Glyph_InitFunc       glyph_init;
    FT_TS_Glyph_DoneFunc       glyph_done;
    FT_TS_Glyph_CopyFunc       glyph_copy;
    FT_TS_Glyph_TransformFunc  glyph_transform;
    FT_TS_Glyph_GetBBoxFunc    glyph_bbox;
    FT_TS_Glyph_PrepareFunc    glyph_prepare;
  };


  typedef FT_TS_Error
  (*FT_TS_Renderer_RenderFunc)( FT_TS_Renderer       renderer,
                             FT_TS_GlyphSlot      slot,
                             FT_TS_Render_Mode    mode,
                             const FT_TS_Vector*  origin );

  typedef FT_TS_Error
  (*FT_TS_Renderer_TransformFunc)( FT_TS_Renderer       renderer,
                                FT_TS_GlyphSlot      slot,
                                const FT_TS_Matrix*  matrix,
                                const FT_TS_Vector*  delta );


  typedef void
  (*FT_TS_Renderer_GetCBoxFunc)( FT_TS_Renderer   renderer,
                              FT_TS_GlyphSlot  slot,
                              FT_TS_BBox*      cbox );


  typedef FT_TS_Error
  (*FT_TS_Renderer_SetModeFunc)( FT_TS_Renderer  renderer,
                              FT_TS_ULong     mode_tag,
                              FT_TS_Pointer   mode_ptr );

/* deprecated identifiers */
#define FTRenderer_render  FT_TS_Renderer_RenderFunc
#define FTRenderer_transform  FT_TS_Renderer_TransformFunc
#define FTRenderer_getCBox  FT_TS_Renderer_GetCBoxFunc
#define FTRenderer_setMode  FT_TS_Renderer_SetModeFunc


  /**************************************************************************
   *
   * @struct:
   *   FT_TS_Renderer_Class
   *
   * @description:
   *   The renderer module class descriptor.
   *
   * @fields:
   *   root ::
   *     The root @FT_TS_Module_Class fields.
   *
   *   glyph_format ::
   *     The glyph image format this renderer handles.
   *
   *   render_glyph ::
   *     A method used to render the image that is in a given glyph slot into
   *     a bitmap.
   *
   *   transform_glyph ::
   *     A method used to transform the image that is in a given glyph slot.
   *
   *   get_glyph_cbox ::
   *     A method used to access the glyph's cbox.
   *
   *   set_mode ::
   *     A method used to pass additional parameters.
   *
   *   raster_class ::
   *     For @FT_TS_GLYPH_FORMAT_OUTLINE renderers only.  This is a pointer to
   *     its raster's class.
   */
  typedef struct  FT_TS_Renderer_Class_
  {
    FT_TS_Module_Class            root;

    FT_TS_Glyph_Format            glyph_format;

    FT_TS_Renderer_RenderFunc     render_glyph;
    FT_TS_Renderer_TransformFunc  transform_glyph;
    FT_TS_Renderer_GetCBoxFunc    get_glyph_cbox;
    FT_TS_Renderer_SetModeFunc    set_mode;

    FT_TS_Raster_Funcs*           raster_class;

  } FT_TS_Renderer_Class;


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Get_Renderer
   *
   * @description:
   *   Retrieve the current renderer for a given glyph format.
   *
   * @input:
   *   library ::
   *     A handle to the library object.
   *
   *   format ::
   *     The glyph format.
   *
   * @return:
   *   A renderer handle.  0~if none found.
   *
   * @note:
   *   An error will be returned if a module already exists by that name, or
   *   if the module requires a version of FreeType that is too great.
   *
   *   To add a new renderer, simply use @FT_TS_Add_Module.  To retrieve a
   *   renderer by its name, use @FT_TS_Get_Module.
   */
  FT_TS_EXPORT( FT_TS_Renderer )
  FT_TS_Get_Renderer( FT_TS_Library       library,
                   FT_TS_Glyph_Format  format );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Set_Renderer
   *
   * @description:
   *   Set the current renderer to use, and set additional mode.
   *
   * @inout:
   *   library ::
   *     A handle to the library object.
   *
   * @input:
   *   renderer ::
   *     A handle to the renderer object.
   *
   *   num_params ::
   *     The number of additional parameters.
   *
   *   parameters ::
   *     Additional parameters.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   In case of success, the renderer will be used to convert glyph images
   *   in the renderer's known format into bitmaps.
   *
   *   This doesn't change the current renderer for other formats.
   *
   *   Currently, no FreeType renderer module uses `parameters`; you should
   *   thus always pass `NULL` as the value.
   */
  FT_TS_EXPORT( FT_TS_Error )
  FT_TS_Set_Renderer( FT_TS_Library     library,
                   FT_TS_Renderer    renderer,
                   FT_TS_UInt        num_params,
                   FT_TS_Parameter*  parameters );

  /* */


FT_TS_END_HEADER

#endif /* FTRENDER_H_ */


/* END */
