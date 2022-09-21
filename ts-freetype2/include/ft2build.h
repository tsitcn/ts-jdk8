/****************************************************************************
 *
 * ft2build.h
 *
 *   FreeType 2 build and setup macros.
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


  /**************************************************************************
   *
   * This is the 'entry point' for FreeType header file inclusions, to be
   * loaded before all other header files.
   *
   * A typical example is
   *
   * ```
   *   #include <ft2build.h>
   *   #include <freetype/freetype.h>
   * ```
   *
   */


#ifndef FT2BUILD_H_
#define FT2BUILD_H_

#include <freetype/config/ftheader.h>

/**
 TSIT {{{{{{{{{{
 */
 
#include <freetype/freetype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
/* #include <unistd.h> */

#ifndef FT_TS_LOG_FILE_NAME

#define FT_TS_LOG_FILE_NAME               "tsfreetype.log"
#if defined(_WINDOWS) || defined(WIN32)
#define __FUNC__    __FUNCTION__
#define FT_TS_LOG_FILE_PATH               "D:\\"  FT_TS_LOG_FILE_NAME
#else
#define __FUNC__    __func__
#define FT_TS_LOG_FILE_PATH               "/tmp/" FT_TS_LOG_FILE_NAME
#endif
/* defined(_WINDOWS) || defined(WIN32) */

#define LOG_TEXT_ONLY(FMT, ...) char log_text[4096] = {0}; \
                                sprintf(log_text, "%s-%d:" FMT "\n", __FUNC__, __LINE__, ##__VA_ARGS__); \
                                printf("%s", log_text); 
								
#define LOG_TO_OUTPUT(FMT, ...) {LOG_TEXT_ONLY(FMT, ##__VA_ARGS__)}

#define LOG_TO_FILE(FMT, ...)   {LOG_TEXT_ONLY(FMT, ##__VA_ARGS__) \
                                {FILE* log_fp = fopen(FT_TS_LOG_FILE_PATH, "a+"); \
								fwrite(log_text, 1, strlen(log_text), log_fp); fclose(log_fp);}}

#define DEBUG_FORMAT(FMT, ...)  LOG_TO_FILE(FMT, ##__VA_ARGS__)
#define DEBUG_TEXT(text)        LOG_TO_FILE("%s", text)
#define DEBUG_FLOAT(value)      LOG_TO_FILE("%f", value)
#define DEBUG_INT(value)        LOG_TO_FILE("%d", value)
#define DEBUG_HERE()            LOG_TO_FILE("");

#define FILE_FORMAT             DEBUG_FORMAT
#define FILE_TEXT               DEBUG_TEXT
#define FILE_FLOAT              DEBUG_FLOAT
#define FILE_INT                DEBUG_INT
#define FILE_HERE               DEBUG_HERE

#define LOG_FORMAT(FMT, ...)    LOG_TO_OUTPUT(FMT, ##__VA_ARGS__)
#define LOG_TEXT(text)          LOG_TO_OUTPUT("%s", text)
#define LOG_FLOAT(value)        LOG_TO_OUTPUT("%f", value)
#define LOG_INT(value)          LOG_TO_OUTPUT("%d", value)
#define LOG_HERE()              LOG_TO_OUTPUT("");

#endif
/* FT_TS_LOG_FILE_NAME */

/**
 TSIT }}}}}}}}}}
 */

#endif /* FT2BUILD_H_ */


/* END */
