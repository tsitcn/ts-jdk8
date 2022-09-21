/****************************************************************************
 *
 * fttrace.h
 *
 *   Tracing handling (specification only).
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


  /* definitions of trace levels for FreeType 2 */

  /* the maximum string length (if the argument to `FT_TS_TRACE_DEF` */
  /* gets used as a string) plus one charachter for ':' plus      */
  /* another one for the trace level                              */
#define FT_TS_MAX_TRACE_LEVEL_LENGTH  (9 + 1 + 1)

  /* the first level must always be `trace_any' */
FT_TS_TRACE_DEF( any )

  /* base components */
FT_TS_TRACE_DEF( calc )      /* calculations            (ftcalc.c)   */
FT_TS_TRACE_DEF( gloader )   /* glyph loader            (ftgloadr.c) */
FT_TS_TRACE_DEF( glyph )     /* glyph management        (ftglyph.c)  */
FT_TS_TRACE_DEF( memory )    /* memory manager          (ftobjs.c)   */
FT_TS_TRACE_DEF( init )      /* initialization          (ftinit.c)   */
FT_TS_TRACE_DEF( io )        /* i/o interface           (ftsystem.c) */
FT_TS_TRACE_DEF( list )      /* list management         (ftlist.c)   */
FT_TS_TRACE_DEF( objs )      /* base objects            (ftobjs.c)   */
FT_TS_TRACE_DEF( outline )   /* outline management      (ftoutln.c)  */
FT_TS_TRACE_DEF( stream )    /* stream manager          (ftstream.c) */

FT_TS_TRACE_DEF( bitmap )    /* bitmap manipulation     (ftbitmap.c) */
FT_TS_TRACE_DEF( checksum )  /* bitmap checksum         (ftobjs.c)   */
FT_TS_TRACE_DEF( mm )        /* MM interface            (ftmm.c)     */
FT_TS_TRACE_DEF( psprops )   /* PS driver properties    (ftpsprop.c) */
FT_TS_TRACE_DEF( raccess )   /* resource fork accessor  (ftrfork.c)  */
FT_TS_TRACE_DEF( synth )     /* bold/slant synthesizer  (ftsynth.c)  */

  /* rasterizers */
FT_TS_TRACE_DEF( raster )    /* monochrome rasterizer   (ftraster.c) */
FT_TS_TRACE_DEF( smooth )    /* anti-aliasing raster    (ftgrays.c)  */

  /* ot-svg module */
FT_TS_TRACE_DEF( otsvg )     /* OT-SVG renderer         (ftsvg.c)    */

  /* cache sub-system */
FT_TS_TRACE_DEF( cache )     /* cache sub-system   (ftcache.c, etc.) */

  /* SFNT driver components */
FT_TS_TRACE_DEF( sfdriver )  /* SFNT font driver        (sfdriver.c) */
FT_TS_TRACE_DEF( sfobjs )    /* SFNT object handler     (sfobjs.c)   */
FT_TS_TRACE_DEF( sfwoff )    /* WOFF format handler     (sfwoff.c)   */
FT_TS_TRACE_DEF( sfwoff2 )   /* WOFF2 format handler    (sfwoff2.c)  */
FT_TS_TRACE_DEF( ttbdf )     /* TrueType embedded BDF   (ttbdf.c)    */
FT_TS_TRACE_DEF( ttcmap )    /* charmap handler         (ttcmap.c)   */
FT_TS_TRACE_DEF( ttcolr )    /* glyph layer table       (ttcolr.c)   */
FT_TS_TRACE_DEF( ttcpal )    /* color palette table     (ttcpal.c)   */
FT_TS_TRACE_DEF( ttsvg )     /* OpenType SVG table      (ttsvg.c)    */
FT_TS_TRACE_DEF( ttkern )    /* kerning handler         (ttkern.c)   */
FT_TS_TRACE_DEF( ttload )    /* basic TrueType tables   (ttload.c)   */
FT_TS_TRACE_DEF( ttmtx )     /* metrics-related tables  (ttmtx.c)    */
FT_TS_TRACE_DEF( ttpost )    /* PS table processing     (ttpost.c)   */
FT_TS_TRACE_DEF( ttsbit )    /* TrueType sbit handling  (ttsbit.c)   */

  /* TrueType driver components */
FT_TS_TRACE_DEF( ttdriver )  /* TT font driver          (ttdriver.c) */
FT_TS_TRACE_DEF( ttgload )   /* TT glyph loader         (ttgload.c)  */
FT_TS_TRACE_DEF( ttgxvar )   /* TrueType GX var handler (ttgxvar.c)  */
FT_TS_TRACE_DEF( ttinterp )  /* bytecode interpreter    (ttinterp.c) */
FT_TS_TRACE_DEF( ttobjs )    /* TT objects manager      (ttobjs.c)   */
FT_TS_TRACE_DEF( ttpload )   /* TT data/program loader  (ttpload.c)  */

  /* Type 1 driver components */
FT_TS_TRACE_DEF( t1afm )
FT_TS_TRACE_DEF( t1driver )
FT_TS_TRACE_DEF( t1gload )
FT_TS_TRACE_DEF( t1load )
FT_TS_TRACE_DEF( t1objs )
FT_TS_TRACE_DEF( t1parse )

  /* PostScript helper module `psaux' */
FT_TS_TRACE_DEF( afmparse )
FT_TS_TRACE_DEF( cffdecode )
FT_TS_TRACE_DEF( psconv )
FT_TS_TRACE_DEF( psobjs )
FT_TS_TRACE_DEF( t1decode )

  /* PostScript hinting module `pshinter' */
FT_TS_TRACE_DEF( pshalgo )
FT_TS_TRACE_DEF( pshrec )

  /* Type 2 driver components */
FT_TS_TRACE_DEF( cffdriver )
FT_TS_TRACE_DEF( cffgload )
FT_TS_TRACE_DEF( cffload )
FT_TS_TRACE_DEF( cffobjs )
FT_TS_TRACE_DEF( cffparse )

FT_TS_TRACE_DEF( cf2blues )
FT_TS_TRACE_DEF( cf2hints )
FT_TS_TRACE_DEF( cf2interp )

  /* Type 42 driver component */
FT_TS_TRACE_DEF( t42 )

  /* CID driver components */
FT_TS_TRACE_DEF( ciddriver )
FT_TS_TRACE_DEF( cidgload )
FT_TS_TRACE_DEF( cidload )
FT_TS_TRACE_DEF( cidobjs )
FT_TS_TRACE_DEF( cidparse )

  /* Windows font component */
FT_TS_TRACE_DEF( winfnt )

  /* PCF font components */
FT_TS_TRACE_DEF( pcfdriver )
FT_TS_TRACE_DEF( pcfread )

  /* BDF font components */
FT_TS_TRACE_DEF( bdfdriver )
FT_TS_TRACE_DEF( bdflib )

  /* PFR font component */
FT_TS_TRACE_DEF( pfr )

  /* OpenType validation components */
FT_TS_TRACE_DEF( otvcommon )
FT_TS_TRACE_DEF( otvbase )
FT_TS_TRACE_DEF( otvgdef )
FT_TS_TRACE_DEF( otvgpos )
FT_TS_TRACE_DEF( otvgsub )
FT_TS_TRACE_DEF( otvjstf )
FT_TS_TRACE_DEF( otvmath )
FT_TS_TRACE_DEF( otvmodule )

  /* TrueTypeGX/AAT validation components */
FT_TS_TRACE_DEF( gxvbsln )
FT_TS_TRACE_DEF( gxvcommon )
FT_TS_TRACE_DEF( gxvfeat )
FT_TS_TRACE_DEF( gxvjust )
FT_TS_TRACE_DEF( gxvkern )
FT_TS_TRACE_DEF( gxvmodule )
FT_TS_TRACE_DEF( gxvmort )
FT_TS_TRACE_DEF( gxvmorx )
FT_TS_TRACE_DEF( gxvlcar )
FT_TS_TRACE_DEF( gxvopbd )
FT_TS_TRACE_DEF( gxvprop )
FT_TS_TRACE_DEF( gxvtrak )

  /* autofit components */
FT_TS_TRACE_DEF( afcjk )
FT_TS_TRACE_DEF( afglobal )
FT_TS_TRACE_DEF( afhints )
FT_TS_TRACE_DEF( afmodule )
FT_TS_TRACE_DEF( aflatin )
FT_TS_TRACE_DEF( afshaper )

  /* SDF components */
FT_TS_TRACE_DEF( sdf )       /* signed distance raster for outlines (ftsdf.c) */
FT_TS_TRACE_DEF( bsdf )      /* signed distance raster for bitmaps (ftbsdf.c) */

/* END */
