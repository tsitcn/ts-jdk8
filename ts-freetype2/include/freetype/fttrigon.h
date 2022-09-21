/****************************************************************************
 *
 * fttrigon.h
 *
 *   FreeType trigonometric functions (specification).
 *
 * Copyright (C) 2001-2022 by
 * David Turner, Robert Wilhelm, and Werner Lemberg.
 *
 * This file is part of the FreeType project, and may only be used,
 * modified, and distributed under the terms of the FreeType project
 * license, LICENSE.TXT.  By continuing to use, modify, or distribute
 * this file you indicate that you have read the license and
 * understand and accept it fully.
 *
 */


#ifndef FTTRIGON_H_
#define FTTRIGON_H_

#include <freetype/freetype.h>

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_TS_BEGIN_HEADER


  /**************************************************************************
   *
   * @section:
   *  computations
   *
   */


  /**************************************************************************
   *
   * @type:
   *   FT_TS_Angle
   *
   * @description:
   *   This type is used to model angle values in FreeType.  Note that the
   *   angle is a 16.16 fixed-point value expressed in degrees.
   *
   */
  typedef FT_TS_Fixed  FT_TS_Angle;


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_ANGLE_PI
   *
   * @description:
   *   The angle pi expressed in @FT_TS_Angle units.
   *
   */
#define FT_TS_ANGLE_PI  ( 180L << 16 )


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_ANGLE_2PI
   *
   * @description:
   *   The angle 2*pi expressed in @FT_TS_Angle units.
   *
   */
#define FT_TS_ANGLE_2PI  ( FT_TS_ANGLE_PI * 2 )


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_ANGLE_PI2
   *
   * @description:
   *   The angle pi/2 expressed in @FT_TS_Angle units.
   *
   */
#define FT_TS_ANGLE_PI2  ( FT_TS_ANGLE_PI / 2 )


  /**************************************************************************
   *
   * @macro:
   *   FT_TS_ANGLE_PI4
   *
   * @description:
   *   The angle pi/4 expressed in @FT_TS_Angle units.
   *
   */
#define FT_TS_ANGLE_PI4  ( FT_TS_ANGLE_PI / 4 )


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Sin
   *
   * @description:
   *   Return the sinus of a given angle in fixed-point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The sinus value.
   *
   * @note:
   *   If you need both the sinus and cosinus for a given angle, use the
   *   function @FT_TS_Vector_Unit.
   *
   */
  FT_TS_EXPORT( FT_TS_Fixed )
  FT_TS_Sin( FT_TS_Angle  angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Cos
   *
   * @description:
   *   Return the cosinus of a given angle in fixed-point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The cosinus value.
   *
   * @note:
   *   If you need both the sinus and cosinus for a given angle, use the
   *   function @FT_TS_Vector_Unit.
   *
   */
  FT_TS_EXPORT( FT_TS_Fixed )
  FT_TS_Cos( FT_TS_Angle  angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Tan
   *
   * @description:
   *   Return the tangent of a given angle in fixed-point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The tangent value.
   *
   */
  FT_TS_EXPORT( FT_TS_Fixed )
  FT_TS_Tan( FT_TS_Angle  angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Atan2
   *
   * @description:
   *   Return the arc-tangent corresponding to a given vector (x,y) in the 2d
   *   plane.
   *
   * @input:
   *   x ::
   *     The horizontal vector coordinate.
   *
   *   y ::
   *     The vertical vector coordinate.
   *
   * @return:
   *   The arc-tangent value (i.e. angle).
   *
   */
  FT_TS_EXPORT( FT_TS_Angle )
  FT_TS_Atan2( FT_TS_Fixed  x,
            FT_TS_Fixed  y );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Angle_Diff
   *
   * @description:
   *   Return the difference between two angles.  The result is always
   *   constrained to the ]-PI..PI] interval.
   *
   * @input:
   *   angle1 ::
   *     First angle.
   *
   *   angle2 ::
   *     Second angle.
   *
   * @return:
   *   Constrained value of `angle2-angle1`.
   *
   */
  FT_TS_EXPORT( FT_TS_Angle )
  FT_TS_Angle_Diff( FT_TS_Angle  angle1,
                 FT_TS_Angle  angle2 );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Vector_Unit
   *
   * @description:
   *   Return the unit vector corresponding to a given angle.  After the
   *   call, the value of `vec.x` will be `cos(angle)`, and the value of
   *   `vec.y` will be `sin(angle)`.
   *
   *   This function is useful to retrieve both the sinus and cosinus of a
   *   given angle quickly.
   *
   * @output:
   *   vec ::
   *     The address of target vector.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   */
  FT_TS_EXPORT( void )
  FT_TS_Vector_Unit( FT_TS_Vector*  vec,
                  FT_TS_Angle    angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Vector_Rotate
   *
   * @description:
   *   Rotate a vector by a given angle.
   *
   * @inout:
   *   vec ::
   *     The address of target vector.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   */
  FT_TS_EXPORT( void )
  FT_TS_Vector_Rotate( FT_TS_Vector*  vec,
                    FT_TS_Angle    angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Vector_Length
   *
   * @description:
   *   Return the length of a given vector.
   *
   * @input:
   *   vec ::
   *     The address of target vector.
   *
   * @return:
   *   The vector length, expressed in the same units that the original
   *   vector coordinates.
   *
   */
  FT_TS_EXPORT( FT_TS_Fixed )
  FT_TS_Vector_Length( FT_TS_Vector*  vec );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Vector_Polarize
   *
   * @description:
   *   Compute both the length and angle of a given vector.
   *
   * @input:
   *   vec ::
   *     The address of source vector.
   *
   * @output:
   *   length ::
   *     The vector length.
   *
   *   angle ::
   *     The vector angle.
   *
   */
  FT_TS_EXPORT( void )
  FT_TS_Vector_Polarize( FT_TS_Vector*  vec,
                      FT_TS_Fixed   *length,
                      FT_TS_Angle   *angle );


  /**************************************************************************
   *
   * @function:
   *   FT_TS_Vector_From_Polar
   *
   * @description:
   *   Compute vector coordinates from a length and angle.
   *
   * @output:
   *   vec ::
   *     The address of source vector.
   *
   * @input:
   *   length ::
   *     The vector length.
   *
   *   angle ::
   *     The vector angle.
   *
   */
  FT_TS_EXPORT( void )
  FT_TS_Vector_From_Polar( FT_TS_Vector*  vec,
                        FT_TS_Fixed    length,
                        FT_TS_Angle    angle );

  /* */


FT_TS_END_HEADER

#endif /* FTTRIGON_H_ */


/* END */
