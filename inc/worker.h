#ifndef WORKER_H_
#define WORKER_H_

/**
 * @file emscripten.h
 * @brief Emscripten BS IDGAF.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Organized by Matt Farstad
 */

#ifndef __WORKER
#define __WORKER 1

typedef struct
{
	/* Image which worker should respond with */
	int want;
	/* Image which worker should compute after response */
	int next;
	/* Flags affecting both images */
	int flags;
	/* Dimensions for both images */
	unsigned int width, height;
} worker_param;

#endif /* !__WORKER */

#endif