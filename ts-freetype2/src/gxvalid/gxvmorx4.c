/****************************************************************************
 *
 * gxvmorx4.c
 *
 *   TrueTypeGX/AAT morx table validation
 *   body for "morx" type4 (Non-Contextual Glyph Substitution) subtable.
 *
 * Copyright (C) 2005-2022 by
 * suzuki toshiya, Masatake YAMATO, Red Hat K.K.,
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */

/****************************************************************************
 *
 * gxvalid is derived from both gxlayout module and otvalid module.
 * Development of gxlayout is supported by the Information-technology
 * Promotion Agency(IPA), Japan.
 *
 */


#include "gxvmorx.h"


  /**************************************************************************
   *
   * The macro FT_TS_COMPONENT is used in trace mode.  It is an implicit
   * parameter of the FT_TS_TRACE() and FT_TS_ERROR() macros, used to print/log
   * messages during execution.
   */
#undef  FT_TS_COMPONENT
#define FT_TS_COMPONENT  gxvmorx


  FT_TS_LOCAL_DEF( void )
  gxv_morx_subtable_type4_validate( FT_TS_Bytes       table,
                                    FT_TS_Bytes       limit,
                                    GXV_Validator  gxvalid )
  {
    GXV_NAME_ENTER( "morx chain subtable type4 "
                    "(Non-Contextual Glyph Substitution)" );

    gxv_mort_subtable_type4_validate( table, limit, gxvalid );

    GXV_EXIT;
  }


/* END */
