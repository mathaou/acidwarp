#ifndef HANDY_H_
#define HANDY_H_

/**
 * @file handy.h
 * @brief Handy stuff.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

#ifndef __HANDY
#define __HANDY 1

 /* Random number stuff that SHOULD have been there */
#include <time.h>
#include <stdint.h>
#include <stdio.h> /* Needed for NULL * */
#include <stdlib.h>

/* Two new control structures for C. Yay! */
#define loop for (;;)
#define loopend(x) \
    {              \
        if (x)     \
            break; \
    }
/* Usage:  do { ... } until (boolean); */
#define until(a) while (!(a))

/* Some basic data types */
typedef enum
{
	FALSE,
	TRUE
} BOOL;

#define RANDOMIZE() (srand((uint8_t)time((time_t *)NULL)))
/* This is skewed due to integer division, but
 * good enough when a is much smaller than RAND_MAX.
 */
#define RANDOM(a) (rand() / (RAND_MAX / (a) + 1))

 /* Stuff that's already there, but is faster as a MACRO */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define ABS(a) (((a) < 0) ? -(a) : (a))
#define FABS(a) (((a) < F 0) ? -(a) : (a))
#define DABS(a) (((a) < 0.0) ? -(a) : (a))

#define SIGN(a) (((a) < 0) ? -1 : 1)
#define FSIGN(a) (((a) < F 0) ? F(-1) : F 1)
#define DSIGN(a) (((a) < 0.0) ? -1.0 : 1.0)

/* This seems nifty to me */
#define DONE 1
#define NOT_DONE 0

void makeShuffledList(int* list, int listSize);

#endif
#endif
