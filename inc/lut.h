#ifndef LUT_H_
#define LUT_H_

/**
 * @file lut.h
 * @brief Lookup tables and some helpful math macros.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

#ifndef __LUT
#define __LUT 1

#define TRIG_UNIT 511
#define ANGLE_UNIT 256
/* The same idea as 2*PI, PI/2, PI/4, etc. */
#define ANGLE_UNIT_2 (ANGLE_UNIT * 2)
#define ANGLE_UNIT_HALF (ANGLE_UNIT / 2)
#define ANGLE_UNIT_EIGHTH (ANGLE_UNIT / 8)
#define ANGLE_UNIT_QUART (ANGLE_UNIT / 4)
#define ANGLE_UNIT_THREE_QUARTERS (ANGLE_UNIT * 3 / 4)

long lut_sin(long a);
/* long lut_cos (long a);  As a macro */
#define lut_cos(angle) (lut_sin((angle) + ANGLE_UNIT_QUART))
long lut_angle(long dx, long dy);
long lut_dist(long x, long y);
#endif

/* You see, 360 degree, 2*PI radians are more or less arbitrary angle units.
 * Since I have created my own trig functions I can create my own
 * angle units. I created ones that powers of 2.
 */

#endif