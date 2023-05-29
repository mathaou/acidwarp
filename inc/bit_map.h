#ifndef BIT_MAP_H_
#define BIT_MAP_H_

/**
 * @file btmap.h
 * @brief Various macros and bitmap-specific functionality.
 *
 * @attention ACID WARP &copy; (c) Copyright 1992, 1993 by Noah Spurrier
 *
 * All Rights reserved. Private Proprietary Source Code by Noah Spurrier <br/>
 * Ported to Linux by Steven Wills <br/>
 * Ported to SDL by Boris Gjenero <br/>
 * Fixed to prioritize SDL2 by Matt Farstad
 */

#include <stdint.h>

#define X_TITLE 80
#define Y_TITLE 98

#define NOAHS_FACE 0

void writeBitmapImageToArray(uint8_t* buf_graf, int image_number,
							 int xmax, int ymax, int stride);
void bit_map_uncompress(uint8_t* buf_graf, uint8_t* bit_data,
						int x_map, int y_map,
						int xmax, int ymax, int xsize);
#endif
